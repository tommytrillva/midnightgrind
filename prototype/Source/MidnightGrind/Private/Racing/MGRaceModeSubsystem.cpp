// Copyright Midnight Grind. All Rights Reserved.

#include "Racing/MGRaceModeSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGRaceModeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentRaceState = EMGRaceState::None;
}

void UMGRaceModeSubsystem::Deinitialize()
{
	CancelRace();
	Super::Deinitialize();
}

// ==========================================
// RACE SETUP
// ==========================================

bool UMGRaceModeSubsystem::SetupRace(const FMGRaceConfig& Config)
{
	if (CurrentRaceState != EMGRaceState::None && CurrentRaceState != EMGRaceState::Finished)
	{
		return false;
	}

	CurrentRaceConfig = Config;
	Racers.Empty();
	Checkpoints.Empty();
	RaceTime = 0.0f;
	NextFinishPosition = 1;
	EliminationCount = 0;
	EliminationTimer = 0.0f;
	ReactionTimes.Empty();
	DragSplits.Empty();

	SetRaceState(EMGRaceState::Setup);

	return true;
}

bool UMGRaceModeSubsystem::AddRacer(const FMGRacerEntry& Racer)
{
	if (CurrentRaceState != EMGRaceState::Setup)
	{
		return false;
	}

	// Check for duplicate
	for (const FMGRacerEntry& Existing : Racers)
	{
		if (Existing.RacerID == Racer.RacerID)
		{
			return false;
		}
	}

	// Check performance class restriction
	if (CurrentRaceConfig.PerformanceClass != EMGPerformanceClass::Open)
	{
		EMGPerformanceClass RacerClass = GetClassForPI(Racer.PerformanceIndex);
		if (RacerClass != CurrentRaceConfig.PerformanceClass)
		{
			return false;
		}
	}

	FMGRacerEntry NewRacer = Racer;
	NewRacer.CurrentPosition = Racers.Num() + 1;
	NewRacer.CurrentLap = 0;
	NewRacer.CurrentCheckpoint = 0;

	Racers.Add(NewRacer);

	return true;
}

bool UMGRaceModeSubsystem::RemoveRacer(const FGuid& RacerID)
{
	if (CurrentRaceState != EMGRaceState::Setup)
	{
		return false;
	}

	for (int32 i = Racers.Num() - 1; i >= 0; --i)
	{
		if (Racers[i].RacerID == RacerID)
		{
			Racers.RemoveAt(i);
			return true;
		}
	}

	return false;
}

void UMGRaceModeSubsystem::ClearRacers()
{
	if (CurrentRaceState == EMGRaceState::Setup)
	{
		Racers.Empty();
	}
}

void UMGRaceModeSubsystem::RegisterCheckpoint(const FMGCheckpointData& Checkpoint)
{
	Checkpoints.Add(Checkpoint);

	// Sort by index
	Checkpoints.Sort([](const FMGCheckpointData& A, const FMGCheckpointData& B)
	{
		return A.CheckpointIndex < B.CheckpointIndex;
	});
}

void UMGRaceModeSubsystem::ClearCheckpoints()
{
	Checkpoints.Empty();
}

bool UMGRaceModeSubsystem::ValidateRaceSetup() const
{
	// Need at least 2 racers
	if (Racers.Num() < 2)
	{
		return false;
	}

	// Need checkpoints for circuit/sprint races
	if (CurrentRaceConfig.RaceType == EMGRaceType::Circuit ||
		CurrentRaceConfig.RaceType == EMGRaceType::Sprint)
	{
		if (Checkpoints.Num() < 2)
		{
			return false;
		}
	}

	// Need a player
	bool bHasPlayer = false;
	for (const FMGRacerEntry& Racer : Racers)
	{
		if (Racer.bIsPlayer)
		{
			bHasPlayer = true;
			break;
		}
	}

	return bHasPlayer;
}

// ==========================================
// RACE CONTROL
// ==========================================

bool UMGRaceModeSubsystem::StartCountdown()
{
	if (CurrentRaceState != EMGRaceState::Setup)
	{
		return false;
	}

	if (!ValidateRaceSetup())
	{
		return false;
	}

	SetRaceState(EMGRaceState::Countdown);
	CountdownSeconds = 3;

	OnCountdownTick.Broadcast(CountdownSeconds);

	// Start countdown timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGRaceModeSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			[WeakThis]()
			{
				if (!WeakThis.IsValid())
				{
					return;
				}
				WeakThis->CountdownSeconds--;

				if (WeakThis->CountdownSeconds > 0)
				{
					WeakThis->OnCountdownTick.Broadcast(WeakThis->CountdownSeconds);
				}
				else
				{
					if (UWorld* World = WeakThis->GetWorld())
					{
						World->GetTimerManager().ClearTimer(WeakThis->CountdownTimerHandle);
					}
					WeakThis->StartRace();
				}
			},
			1.0f,
			true
		);
	}

	return true;
}

void UMGRaceModeSubsystem::StartRace()
{
	SetRaceState(EMGRaceState::Racing);
	RaceTime = 0.0f;

	// Set all racers to lap 1
	for (FMGRacerEntry& Racer : Racers)
	{
		Racer.CurrentLap = 1;
		Racer.TotalTime = 0.0f;
	}

	OnRaceStarted.Broadcast();

	// Start race update timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			UpdateTimerHandle,
			[this]()
			{
				if (!bRacePaused && CurrentRaceState == EMGRaceState::Racing)
				{
					const float DeltaTime = 0.05f;
					RaceTime += DeltaTime;

					// Update all racer times
					for (FMGRacerEntry& Racer : Racers)
					{
						if (!Racer.bFinished && !Racer.bDNF && !Racer.bEliminated)
						{
							Racer.TotalTime += DeltaTime;
						}
					}

					UpdatePositions();

					// Handle elimination races
					if (CurrentRaceConfig.RaceType == EMGRaceType::Elimination)
					{
						UpdateEliminationTimer(DeltaTime);
					}

					CheckRaceCompletion();
				}
			},
			0.05f,
			true
		);

		// Start elimination timer if needed
		if (CurrentRaceConfig.RaceType == EMGRaceType::Elimination)
		{
			EliminationTimer = CurrentRaceConfig.EliminationInterval;
		}
	}
}

void UMGRaceModeSubsystem::PauseRace()
{
	if (CurrentRaceState == EMGRaceState::Racing)
	{
		bRacePaused = true;
	}
}

void UMGRaceModeSubsystem::ResumeRace()
{
	if (CurrentRaceState == EMGRaceState::Racing)
	{
		bRacePaused = false;
	}
}

void UMGRaceModeSubsystem::FinishRace()
{
	if (CurrentRaceState != EMGRaceState::Racing)
	{
		return;
	}

	// Stop timers
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		World->GetTimerManager().ClearTimer(EliminationTimerHandle);
	}

	// Mark unfinished racers as DNF
	for (FMGRacerEntry& Racer : Racers)
	{
		if (!Racer.bFinished && !Racer.bDNF && !Racer.bEliminated)
		{
			Racer.bDNF = true;
		}
	}

	SetRaceState(EMGRaceState::Finished);

	// Generate and broadcast result
	LastRaceResult = GenerateRaceResult();
	OnRaceFinished.Broadcast(LastRaceResult);
}

void UMGRaceModeSubsystem::CancelRace()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimerHandle);
		World->GetTimerManager().ClearTimer(CountdownTimerHandle);
		World->GetTimerManager().ClearTimer(EliminationTimerHandle);
	}

	SetRaceState(EMGRaceState::Cancelled);
}

void UMGRaceModeSubsystem::RestartRace()
{
	FMGRaceConfig Config = CurrentRaceConfig;
	TArray<FMGRacerEntry> SavedRacers = Racers;

	CancelRace();
	SetupRace(Config);

	for (FMGRacerEntry& Racer : SavedRacers)
	{
		Racer.CurrentLap = 0;
		Racer.CurrentCheckpoint = 0;
		Racer.TotalTime = 0.0f;
		Racer.bFinished = false;
		Racer.bDNF = false;
		Racer.bEliminated = false;
		Racer.LapTimes.Empty();
		Racer.DriftScore = 0.0f;

		AddRacer(Racer);
	}
}

// ==========================================
// RACER PROGRESS
// ==========================================

void UMGRaceModeSubsystem::RacerPassedCheckpoint(const FGuid& RacerID, int32 CheckpointIndex)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID)
		{
			// Validate checkpoint order
			int32 ExpectedCheckpoint = (Racer.CurrentCheckpoint + 1) % Checkpoints.Num();
			if (CheckpointIndex == ExpectedCheckpoint)
			{
				Racer.CurrentCheckpoint = CheckpointIndex;

				// Calculate progress
				if (Checkpoints.Num() > 0)
				{
					float LapProgress = static_cast<float>(CheckpointIndex + 1) / Checkpoints.Num();
					Racer.RaceProgress = (static_cast<float>(Racer.CurrentLap - 1) + LapProgress) / CurrentRaceConfig.NumLaps;
				}

				OnCheckpointPassed.Broadcast(RacerID, CheckpointIndex);

				// Check if completed lap
				if (CheckpointIndex == 0 && Racer.CurrentLap > 0)
				{
					RacerCompletedLap(RacerID);
				}
			}
			break;
		}
	}
}

void UMGRaceModeSubsystem::RacerCompletedLap(const FGuid& RacerID)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID)
		{
			// Calculate lap time
			float LapTime = Racer.TotalTime;
			if (Racer.LapTimes.Num() > 0)
			{
				float PreviousTotal = 0.0f;
				for (float Time : Racer.LapTimes)
				{
					PreviousTotal += Time;
				}
				LapTime = Racer.TotalTime - PreviousTotal;
			}

			Racer.LapTimes.Add(LapTime);

			// Update best lap
			if (Racer.BestLapTime <= 0.0f || LapTime < Racer.BestLapTime)
			{
				Racer.BestLapTime = LapTime;
			}

			OnLapCompleted.Broadcast(RacerID, Racer.CurrentLap);

			// Check if finished race
			if (Racer.CurrentLap >= CurrentRaceConfig.NumLaps)
			{
				RacerFinished(RacerID);
			}
			else
			{
				Racer.CurrentLap++;
			}
			break;
		}
	}
}

void UMGRaceModeSubsystem::RacerFinished(const FGuid& RacerID)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID && !Racer.bFinished)
		{
			Racer.bFinished = true;
			Racer.FinishTime = Racer.TotalTime;
			Racer.CurrentPosition = NextFinishPosition++;

			OnRacerFinished.Broadcast(Racer);

			// Check for track record
			FMGTrackRecord CurrentRecord = GetTrackRecord(CurrentRaceConfig.TrackID, CurrentRaceConfig.RaceType);
			if (CurrentRecord.BestLapTime <= 0.0f || Racer.BestLapTime < CurrentRecord.BestLapTime)
			{
				FMGTrackRecord NewRecord;
				NewRecord.TrackID = CurrentRaceConfig.TrackID;
				NewRecord.RaceType = CurrentRaceConfig.RaceType;
				NewRecord.BestLapTime = Racer.BestLapTime;
				NewRecord.BestTotalTime = Racer.TotalTime;
				NewRecord.RecordHolderName = Racer.RacerName;
				NewRecord.VehicleUsed = Racer.VehicleID;
				NewRecord.RecordDate = FDateTime::Now();

				SaveTrackRecord(NewRecord);
				OnNewTrackRecord.Broadcast(NewRecord);
			}

			break;
		}
	}

	CheckRaceCompletion();
}

void UMGRaceModeSubsystem::RacerDNF(const FGuid& RacerID)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID)
		{
			Racer.bDNF = true;
			break;
		}
	}

	CheckRaceCompletion();
}

void UMGRaceModeSubsystem::EliminateRacer(const FGuid& RacerID)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID)
		{
			Racer.bEliminated = true;
			EliminationCount++;

			OnRacerEliminated.Broadcast(RacerID);
			break;
		}
	}

	CheckRaceCompletion();
}

FMGRacerEntry UMGRaceModeSubsystem::GetRacerData(const FGuid& RacerID) const
{
	for (const FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID)
		{
			return Racer;
		}
	}

	return FMGRacerEntry();
}

TArray<FMGRacerEntry> UMGRaceModeSubsystem::GetCurrentStandings() const
{
	TArray<FMGRacerEntry> Standings = Racers;

	Standings.Sort([](const FMGRacerEntry& A, const FMGRacerEntry& B)
	{
		// Finished racers first, by position
		if (A.bFinished && !B.bFinished) return true;
		if (!A.bFinished && B.bFinished) return false;
		if (A.bFinished && B.bFinished) return A.CurrentPosition < B.CurrentPosition;

		// DNF/Eliminated at the end
		if (A.bDNF || A.bEliminated) return false;
		if (B.bDNF || B.bEliminated) return true;

		// By progress
		return A.RaceProgress > B.RaceProgress;
	});

	return Standings;
}

int32 UMGRaceModeSubsystem::GetRacerPosition(const FGuid& RacerID) const
{
	TArray<FMGRacerEntry> Standings = GetCurrentStandings();

	for (int32 i = 0; i < Standings.Num(); ++i)
	{
		if (Standings[i].RacerID == RacerID)
		{
			return i + 1;
		}
	}

	return -1;
}

// ==========================================
// PLAYER SPECIFIC
// ==========================================

void UMGRaceModeSubsystem::SetPlayerRacer(const FGuid& RacerID)
{
	PlayerRacerID = RacerID;
}

FMGRacerEntry UMGRaceModeSubsystem::GetPlayerRacerData() const
{
	return GetRacerData(PlayerRacerID);
}

int32 UMGRaceModeSubsystem::GetPlayerPosition() const
{
	return GetRacerPosition(PlayerRacerID);
}

int32 UMGRaceModeSubsystem::GetPlayerCurrentLap() const
{
	return GetRacerData(PlayerRacerID).CurrentLap;
}

float UMGRaceModeSubsystem::GetPlayerBestLap() const
{
	return GetRacerData(PlayerRacerID).BestLapTime;
}

float UMGRaceModeSubsystem::GetGapToLeader() const
{
	TArray<FMGRacerEntry> Standings = GetCurrentStandings();

	if (Standings.Num() < 2)
	{
		return 0.0f;
	}

	FMGRacerEntry PlayerData = GetPlayerRacerData();

	// If player is leader, return 0
	if (Standings[0].RacerID == PlayerRacerID)
	{
		return 0.0f;
	}

	return PlayerData.TotalTime - Standings[0].TotalTime;
}

float UMGRaceModeSubsystem::GetGapToRacerAhead() const
{
	int32 PlayerPos = GetPlayerPosition();

	if (PlayerPos <= 1)
	{
		return 0.0f;
	}

	TArray<FMGRacerEntry> Standings = GetCurrentStandings();
	FMGRacerEntry PlayerData = GetPlayerRacerData();

	if (PlayerPos <= Standings.Num())
	{
		return PlayerData.TotalTime - Standings[PlayerPos - 2].TotalTime;
	}

	return 0.0f;
}

float UMGRaceModeSubsystem::GetGapToRacerBehind() const
{
	int32 PlayerPos = GetPlayerPosition();
	TArray<FMGRacerEntry> Standings = GetCurrentStandings();

	if (PlayerPos >= Standings.Num())
	{
		return 0.0f;
	}

	FMGRacerEntry PlayerData = GetPlayerRacerData();

	return Standings[PlayerPos].TotalTime - PlayerData.TotalTime;
}

// ==========================================
// TIMING
// ==========================================

void UMGRaceModeSubsystem::UpdateRacerTime(const FGuid& RacerID, float DeltaTime)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID && !Racer.bFinished && !Racer.bDNF)
		{
			Racer.TotalTime += DeltaTime;
			break;
		}
	}
}

float UMGRaceModeSubsystem::GetRacerLapTime(const FGuid& RacerID) const
{
	FMGRacerEntry Racer = GetRacerData(RacerID);

	if (Racer.LapTimes.Num() == 0)
	{
		return Racer.TotalTime;
	}

	float PreviousLapsTotal = 0.0f;
	for (float Time : Racer.LapTimes)
	{
		PreviousLapsTotal += Time;
	}

	return Racer.TotalTime - PreviousLapsTotal;
}

float UMGRaceModeSubsystem::GetRacerBestLap(const FGuid& RacerID) const
{
	return GetRacerData(RacerID).BestLapTime;
}

float UMGRaceModeSubsystem::GetTrackRecordLap(FName TrackID, EMGRaceType RaceType) const
{
	FMGTrackRecord Record = GetTrackRecord(TrackID, RaceType);
	return Record.BestLapTime;
}

// ==========================================
// DRIFT SCORING
// ==========================================

void UMGRaceModeSubsystem::AddDriftScore(const FGuid& RacerID, float Score)
{
	for (FMGRacerEntry& Racer : Racers)
	{
		if (Racer.RacerID == RacerID)
		{
			Racer.DriftScore += Score;
			break;
		}
	}
}

float UMGRaceModeSubsystem::GetRacerDriftScore(const FGuid& RacerID) const
{
	return GetRacerData(RacerID).DriftScore;
}

TArray<FMGRacerEntry> UMGRaceModeSubsystem::GetDriftStandings() const
{
	TArray<FMGRacerEntry> Standings = Racers;

	Standings.Sort([](const FMGRacerEntry& A, const FMGRacerEntry& B)
	{
		return A.DriftScore > B.DriftScore;
	});

	return Standings;
}

// ==========================================
// DRAG RACING
// ==========================================

void UMGRaceModeSubsystem::RecordReactionTime(const FGuid& RacerID, float ReactionTime)
{
	ReactionTimes.Add(RacerID, ReactionTime);
}

void UMGRaceModeSubsystem::RecordDragSplit(const FGuid& RacerID, float Distance, float Time)
{
	if (!DragSplits.Contains(RacerID))
	{
		DragSplits.Add(RacerID, TArray<float>());
	}

	DragSplits[RacerID].Add(Time);
}

float UMGRaceModeSubsystem::GetRacerReactionTime(const FGuid& RacerID) const
{
	const float* Time = ReactionTimes.Find(RacerID);
	return Time ? *Time : 0.0f;
}

// ==========================================
// UTILITIES
// ==========================================

EMGPerformanceClass UMGRaceModeSubsystem::GetClassForPI(int32 PerformanceIndex) const
{
	if (PerformanceIndex >= 900) return EMGPerformanceClass::X;
	if (PerformanceIndex >= 800) return EMGPerformanceClass::S2;
	if (PerformanceIndex >= 700) return EMGPerformanceClass::S1;
	if (PerformanceIndex >= 600) return EMGPerformanceClass::S;
	if (PerformanceIndex >= 500) return EMGPerformanceClass::A;
	if (PerformanceIndex >= 400) return EMGPerformanceClass::B;
	if (PerformanceIndex >= 300) return EMGPerformanceClass::C;
	return EMGPerformanceClass::D;
}

FString UMGRaceModeSubsystem::GetClassDisplayName(EMGPerformanceClass Class) const
{
	switch (Class)
	{
		case EMGPerformanceClass::D: return TEXT("D");
		case EMGPerformanceClass::C: return TEXT("C");
		case EMGPerformanceClass::B: return TEXT("B");
		case EMGPerformanceClass::A: return TEXT("A");
		case EMGPerformanceClass::S: return TEXT("S");
		case EMGPerformanceClass::S1: return TEXT("S1");
		case EMGPerformanceClass::S2: return TEXT("S2");
		case EMGPerformanceClass::X: return TEXT("X");
		default: return TEXT("Open");
	}
}

int64 UMGRaceModeSubsystem::CalculateRewards(int32 Position, const FMGRaceConfig& Config) const
{
	switch (Position)
	{
		case 1: return Config.PrizeMoney1st;
		case 2: return Config.PrizeMoney2nd;
		case 3: return Config.PrizeMoney3rd;
		default:
			// Participation bonus
			return FMath::Max(static_cast<int64>(100), Config.PrizeMoney3rd / 10);
	}
}

int32 UMGRaceModeSubsystem::CalculateREPReward(int32 Position, const FMGRaceConfig& Config) const
{
	float Multiplier = 1.0f;

	switch (Position)
	{
		case 1: Multiplier = 1.0f; break;
		case 2: Multiplier = 0.75f; break;
		case 3: Multiplier = 0.5f; break;
		default: Multiplier = 0.25f; break;
	}

	// Difficulty bonus
	switch (Config.Difficulty)
	{
		case EMGRaceDifficulty::Easy: break;
		case EMGRaceDifficulty::Medium: Multiplier *= 1.25f; break;
		case EMGRaceDifficulty::Hard: Multiplier *= 1.5f; break;
		case EMGRaceDifficulty::Expert: Multiplier *= 2.0f; break;
		case EMGRaceDifficulty::Legendary: Multiplier *= 3.0f; break;
	}

	return static_cast<int32>(Config.REPReward * Multiplier);
}

// ==========================================
// RECORDS
// ==========================================

void UMGRaceModeSubsystem::SaveTrackRecord(const FMGTrackRecord& Record)
{
	// Find and update existing, or add new
	for (FMGTrackRecord& Existing : TrackRecords)
	{
		if (Existing.TrackID == Record.TrackID && Existing.RaceType == Record.RaceType)
		{
			Existing = Record;
			return;
		}
	}

	TrackRecords.Add(Record);
}

FMGTrackRecord UMGRaceModeSubsystem::GetTrackRecord(FName TrackID, EMGRaceType RaceType) const
{
	for (const FMGTrackRecord& Record : TrackRecords)
	{
		if (Record.TrackID == TrackID && Record.RaceType == RaceType)
		{
			return Record;
		}
	}

	return FMGTrackRecord();
}

TArray<FMGTrackRecord> UMGRaceModeSubsystem::GetAllRecordsForTrack(FName TrackID) const
{
	TArray<FMGTrackRecord> Result;

	for (const FMGTrackRecord& Record : TrackRecords)
	{
		if (Record.TrackID == TrackID)
		{
			Result.Add(Record);
		}
	}

	return Result;
}

// ==========================================
// INTERNAL FUNCTIONS
// ==========================================

void UMGRaceModeSubsystem::UpdatePositions()
{
	TArray<FMGRacerEntry> Standings = GetCurrentStandings();

	for (int32 i = 0; i < Standings.Num(); ++i)
	{
		FGuid RacerID = Standings[i].RacerID;
		int32 NewPosition = i + 1;

		for (FMGRacerEntry& Racer : Racers)
		{
			if (Racer.RacerID == RacerID)
			{
				if (Racer.CurrentPosition != NewPosition && !Racer.bFinished)
				{
					Racer.CurrentPosition = NewPosition;
					OnPositionChanged.Broadcast(RacerID, NewPosition);
				}
				break;
			}
		}
	}
}

void UMGRaceModeSubsystem::UpdateEliminationTimer(float DeltaTime)
{
	EliminationTimer -= DeltaTime;

	if (EliminationTimer <= 0.0f)
	{
		// Eliminate last place racer
		TArray<FMGRacerEntry> Standings = GetCurrentStandings();

		// Find last non-eliminated, non-finished racer
		for (int32 i = Standings.Num() - 1; i >= 0; --i)
		{
			if (!Standings[i].bFinished && !Standings[i].bEliminated && !Standings[i].bDNF)
			{
				EliminateRacer(Standings[i].RacerID);
				break;
			}
		}

		// Reset timer
		EliminationTimer = CurrentRaceConfig.EliminationInterval;
	}
}

void UMGRaceModeSubsystem::CheckRaceCompletion()
{
	int32 ActiveRacers = 0;
	int32 FinishedRacers = 0;

	for (const FMGRacerEntry& Racer : Racers)
	{
		if (!Racer.bDNF && !Racer.bEliminated)
		{
			ActiveRacers++;
		}
		if (Racer.bFinished)
		{
			FinishedRacers++;
		}
	}

	// Race ends when all active racers finish
	bool bAllFinished = (ActiveRacers > 0 && FinishedRacers >= ActiveRacers);

	// Or when only 1 racer left in elimination
	bool bEliminationComplete = (CurrentRaceConfig.RaceType == EMGRaceType::Elimination && ActiveRacers <= 1);

	// Or when player is busted/DNF
	FMGRacerEntry PlayerData = GetPlayerRacerData();
	bool bPlayerOut = PlayerData.bDNF || PlayerData.bEliminated;

	if (bAllFinished || bEliminationComplete || bPlayerOut)
	{
		FinishRace();
	}
}

FMGRaceResult UMGRaceModeSubsystem::GenerateRaceResult()
{
	FMGRaceResult Result;
	Result.RaceConfig = CurrentRaceConfig;
	Result.FinalStandings = GetCurrentStandings();

	// Find player position
	FMGRacerEntry PlayerData = GetPlayerRacerData();
	Result.PlayerFinishPosition = GetRacerPosition(PlayerRacerID);
	Result.bPlayerWon = (Result.PlayerFinishPosition == 1);
	Result.bPlayerDNF = PlayerData.bDNF || PlayerData.bEliminated;

	// Calculate rewards
	if (!Result.bPlayerDNF)
	{
		Result.CashEarned = CalculateRewards(Result.PlayerFinishPosition, CurrentRaceConfig);
		Result.REPEarned = CalculateREPReward(Result.PlayerFinishPosition, CurrentRaceConfig);
	}

	// Pink slip
	if (CurrentRaceConfig.bIsPinkSlipRace && Result.bPlayerWon)
	{
		Result.bWonPinkSlip = true;
		Result.PinkSlipVehicleWon = CurrentRaceConfig.PinkSlipVehicleID;
	}

	// Timing
	Result.PlayerBestLap = PlayerData.BestLapTime;
	Result.PlayerTotalTime = PlayerData.TotalTime;

	// Track record
	FMGTrackRecord CurrentRecord = GetTrackRecord(CurrentRaceConfig.TrackID, CurrentRaceConfig.RaceType);
	Result.TrackRecordTime = CurrentRecord.BestLapTime;
	Result.bNewTrackRecord = (PlayerData.BestLapTime < CurrentRecord.BestLapTime && PlayerData.BestLapTime > 0.0f);

	// Stats
	Result.PlayerTopSpeed = PlayerData.TopSpeed;
	Result.TotalDriftScore = PlayerData.DriftScore;
	Result.TotalNearMisses = PlayerData.NearMisses;
	Result.TotalPerfectShifts = PlayerData.PerfectShifts;

	return Result;
}

void UMGRaceModeSubsystem::SetRaceState(EMGRaceState NewState)
{
	if (CurrentRaceState != NewState)
	{
		CurrentRaceState = NewState;
		OnRaceStateChanged.Broadcast(NewState);
	}
}

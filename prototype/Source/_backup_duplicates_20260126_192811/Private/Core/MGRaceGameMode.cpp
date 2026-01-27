// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGRaceGameMode.h"
#include "Track/MGTrackSubsystem.h"
#include "Replay/MGReplaySubsystem.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

AMGRaceGameMode::AMGRaceGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AMGRaceGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Get subsystems
	if (UWorld* World = GetWorld())
	{
		TrackSubsystem = World->GetSubsystem<UMGTrackSubsystem>();
		ReplaySubsystem = World->GetSubsystem<UMGReplaySubsystem>();
	}
}

void AMGRaceGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentPhase)
	{
		case EMGRacePhase::Countdown:
			UpdateCountdown(DeltaTime);
			break;

		case EMGRacePhase::Racing:
			UpdateRace(DeltaTime);
			break;

		default:
			break;
	}
}

// ==========================================
// RACE CONTROL
// ==========================================

void AMGRaceGameMode::InitializeRace(const FMGRaceConfig& Config)
{
	RaceConfig = Config;
	CurrentPhase = EMGRacePhase::WaitingForPlayers;
	RaceTime = 0.0f;
	NextFinishPosition = 1;
	Racers.Empty();
	FinalResults.Empty();

	// Initialize track if we have the subsystem
	if (TrackSubsystem)
	{
		TrackSubsystem->LoadTrack(Config.TrackID);
	}

	SetPhase(EMGRacePhase::WaitingForPlayers);
}

void AMGRaceGameMode::StartCountdown()
{
	if (CurrentPhase != EMGRacePhase::WaitingForPlayers)
	{
		return;
	}

	CountdownRemaining = RaceConfig.CountdownDuration;
	LastCountdownTick = FMath::CeilToInt(CountdownRemaining);

	SetPhase(EMGRacePhase::Countdown);

	// Initial countdown broadcast
	OnCountdownTick.Broadcast(LastCountdownTick);
}

void AMGRaceGameMode::ForceStartRace()
{
	if (CurrentPhase == EMGRacePhase::Racing)
	{
		return;
	}

	StartRace();
}

void AMGRaceGameMode::AbortRace()
{
	// Mark all racers as DNF
	for (FMGRacerState& Racer : Racers)
	{
		if (!Racer.bHasFinished)
		{
			Racer.bDNF = true;
		}
	}

	EndRace();
}

// ==========================================
// RACER MANAGEMENT
// ==========================================

void AMGRaceGameMode::RegisterRacer(AController* Controller, APawn* Vehicle, bool bIsPlayer, const FString& PlayerName, FName VehicleID)
{
	if (!Controller || !Vehicle)
	{
		return;
	}

	// Check if already registered
	if (FindRacerIndex(Controller) != INDEX_NONE)
	{
		return;
	}

	FMGRacerState NewRacer;
	NewRacer.Controller = Controller;
	NewRacer.Vehicle = Vehicle;
	NewRacer.bIsPlayer = bIsPlayer;
	NewRacer.PlayerName = PlayerName;
	NewRacer.VehicleID = VehicleID;
	NewRacer.Position = Racers.Num() + 1;
	NewRacer.CurrentLap = 1;
	NewRacer.LastCheckpoint = 0;

	Racers.Add(NewRacer);

	// Start replay recording for player
	if (bIsPlayer && ReplaySubsystem && RaceConfig.bEnableGhost)
	{
		ReplaySubsystem->StartRecording();
	}
}

void AMGRaceGameMode::UnregisterRacer(AController* Controller)
{
	int32 Index = FindRacerIndex(Controller);
	if (Index != INDEX_NONE)
	{
		Racers.RemoveAt(Index);
		UpdatePositions();
	}
}

FMGRacerState AMGRaceGameMode::GetRacerState(AController* Controller) const
{
	int32 Index = FindRacerIndex(Controller);
	if (Index != INDEX_NONE)
	{
		return Racers[Index];
	}
	return FMGRacerState();
}

int32 AMGRaceGameMode::GetFinishedRacerCount() const
{
	int32 Count = 0;
	for (const FMGRacerState& Racer : Racers)
	{
		if (Racer.bHasFinished)
		{
			Count++;
		}
	}
	return Count;
}

// ==========================================
// CHECKPOINT HANDLING
// ==========================================

void AMGRaceGameMode::ReportCheckpointCrossed(AController* Controller, int32 CheckpointIndex)
{
	if (CurrentPhase != EMGRacePhase::Racing)
	{
		return;
	}

	int32 RacerIndex = FindRacerIndex(Controller);
	if (RacerIndex == INDEX_NONE)
	{
		return;
	}

	FMGRacerState& Racer = Racers[RacerIndex];

	// Validate checkpoint order (must be sequential)
	int32 ExpectedCheckpoint = Racer.LastCheckpoint + 1;

	// Get total checkpoint count from track subsystem
	int32 TotalCheckpoints = 0;
	if (TrackSubsystem)
	{
		TotalCheckpoints = TrackSubsystem->GetCheckpointCount();
	}

	// Handle lap wrap-around
	if (TotalCheckpoints > 0 && ExpectedCheckpoint >= TotalCheckpoints)
	{
		ExpectedCheckpoint = 0;
	}

	if (CheckpointIndex == ExpectedCheckpoint)
	{
		Racer.LastCheckpoint = CheckpointIndex;

		// Update distance for position calculation
		if (Racer.Vehicle)
		{
			// Calculate distance based on checkpoint progress
			float CheckpointProgress = (float)(Racer.LastCheckpoint + 1) / FMath::Max(1, TotalCheckpoints);
			float LapDistance = 1000.0f; // Base lap distance units
			Racer.DistanceTraveled = ((Racer.CurrentLap - 1) * LapDistance) + (CheckpointProgress * LapDistance);
		}
	}

	UpdatePositions();
}

void AMGRaceGameMode::ReportFinishLineCrossed(AController* Controller)
{
	if (CurrentPhase != EMGRacePhase::Racing)
	{
		return;
	}

	int32 RacerIndex = FindRacerIndex(Controller);
	if (RacerIndex == INDEX_NONE)
	{
		return;
	}

	FMGRacerState& Racer = Racers[RacerIndex];

	// Check if all checkpoints were hit
	int32 TotalCheckpoints = 0;
	if (TrackSubsystem)
	{
		TotalCheckpoints = TrackSubsystem->GetCheckpointCount();
	}

	// Validate that racer passed all checkpoints
	if (TotalCheckpoints > 0 && Racer.LastCheckpoint < TotalCheckpoints - 1)
	{
		// Missed checkpoints - don't count this crossing
		return;
	}

	// Calculate lap time
	float LapTime = RaceTime - Racer.CurrentLapStartTime;
	Racer.LapTimes.Add(LapTime);

	// Update best lap
	if (LapTime < Racer.BestLapTime)
	{
		Racer.BestLapTime = LapTime;
	}

	// Broadcast lap completion
	OnRacerLapCompleted.Broadcast(Racer, Racer.CurrentLap);

	// Check if race completed
	if (Racer.CurrentLap >= RaceConfig.LapCount)
	{
		ProcessRacerFinish(Racer);
	}
	else
	{
		// Start new lap
		Racer.CurrentLap++;
		Racer.CurrentLapStartTime = RaceTime;
		Racer.LastCheckpoint = 0;
	}

	UpdatePositions();

	// Check if all racers finished
	if (GetFinishedRacerCount() >= Racers.Num())
	{
		EndRace();
	}
}

// ==========================================
// RESULTS
// ==========================================

FMGFinalRaceResult AMGRaceGameMode::GetPlayerResult() const
{
	for (const FMGFinalRaceResult& Result : FinalResults)
	{
		if (Result.bIsPlayer)
		{
			return Result;
		}
	}
	return FMGFinalRaceResult();
}

// ==========================================
// INTERNAL
// ==========================================

void AMGRaceGameMode::UpdateCountdown(float DeltaTime)
{
	CountdownRemaining -= DeltaTime;

	int32 CurrentTick = FMath::CeilToInt(CountdownRemaining);
	if (CurrentTick != LastCountdownTick && CurrentTick >= 0)
	{
		LastCountdownTick = CurrentTick;
		OnCountdownTick.Broadcast(CurrentTick);
	}

	if (CountdownRemaining <= 0.0f)
	{
		StartRace();
	}
}

void AMGRaceGameMode::UpdateRace(float DeltaTime)
{
	RaceTime += DeltaTime;

	// Update racer times
	for (FMGRacerState& Racer : Racers)
	{
		if (!Racer.bHasFinished && !Racer.bDNF)
		{
			Racer.TotalTime = RaceTime;
		}
	}

	// Update positions periodically
	PositionUpdateAccumulator += DeltaTime;
	if (PositionUpdateAccumulator >= PositionUpdateInterval)
	{
		PositionUpdateAccumulator = 0.0f;
		UpdatePositions();
	}

	// Check max race time
	if (RaceConfig.MaxRaceTime > 0.0f && RaceTime >= RaceConfig.MaxRaceTime)
	{
		// Mark remaining racers as DNF
		for (FMGRacerState& Racer : Racers)
		{
			if (!Racer.bHasFinished)
			{
				Racer.bDNF = true;
			}
		}
		EndRace();
	}
}

void AMGRaceGameMode::UpdatePositions()
{
	if (Racers.Num() == 0)
	{
		return;
	}

	// Sort racers by position score
	TArray<TPair<int32, float>> RacerScores;
	for (int32 i = 0; i < Racers.Num(); i++)
	{
		float Score = CalculatePositionScore(Racers[i]);
		RacerScores.Add(TPair<int32, float>(i, Score));
	}

	// Sort by score (higher is better position)
	RacerScores.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	// Assign positions
	for (int32 i = 0; i < RacerScores.Num(); i++)
	{
		int32 RacerIndex = RacerScores[i].Key;

		// Finished racers keep their finish position
		if (!Racers[RacerIndex].bHasFinished)
		{
			Racers[RacerIndex].Position = i + 1;
		}
	}

	OnPositionsUpdated.Broadcast(Racers);
}

float AMGRaceGameMode::CalculatePositionScore(const FMGRacerState& Racer) const
{
	// Finished racers always rank by finish position
	if (Racer.bHasFinished)
	{
		return 1000000.0f - (Racer.Position * 1000.0f) + (10000.0f - Racer.FinishTime);
	}

	// DNF racers rank last
	if (Racer.bDNF)
	{
		return -1000000.0f;
	}

	// Score based on lap and checkpoint progress
	float Score = 0.0f;

	// Lap progress (major factor)
	Score += Racer.CurrentLap * 100000.0f;

	// Checkpoint progress
	Score += Racer.LastCheckpoint * 1000.0f;

	// Distance traveled (fine-grained)
	Score += Racer.DistanceTraveled;

	return Score;
}

void AMGRaceGameMode::SetPhase(EMGRacePhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	CurrentPhase = NewPhase;
	OnRacePhaseChanged.Broadcast(NewPhase);
}

void AMGRaceGameMode::StartRace()
{
	SetPhase(EMGRacePhase::Racing);
	RaceTime = 0.0f;
	PositionUpdateAccumulator = 0.0f;

	// Initialize all racer lap start times
	for (FMGRacerState& Racer : Racers)
	{
		Racer.CurrentLapStartTime = 0.0f;
		Racer.TotalTime = 0.0f;
	}

	OnRaceStarted.Broadcast();
	UpdatePositions();
}

void AMGRaceGameMode::EndRace()
{
	SetPhase(EMGRacePhase::Finished);

	// Stop replay recording
	if (ReplaySubsystem)
	{
		ReplaySubsystem->StopRecording();
	}

	OnRaceFinished.Broadcast();

	// Calculate results after a short delay to allow final position updates
	CalculateResults();
}

void AMGRaceGameMode::ProcessRacerFinish(FMGRacerState& Racer)
{
	Racer.bHasFinished = true;
	Racer.FinishTime = RaceTime;
	Racer.Position = NextFinishPosition++;

	OnRacerFinished.Broadcast(Racer, Racer.Position);
}

void AMGRaceGameMode::CalculateResults()
{
	FinalResults.Empty();

	// Sort racers by finish position (finished first, then by race progress)
	TArray<FMGRacerState> SortedRacers = Racers;
	SortedRacers.Sort([](const FMGRacerState& A, const FMGRacerState& B)
	{
		// Both finished - sort by finish time
		if (A.bHasFinished && B.bHasFinished)
		{
			return A.Position < B.Position;
		}
		// Finished before not finished
		if (A.bHasFinished != B.bHasFinished)
		{
			return A.bHasFinished;
		}
		// Neither finished - sort by progress
		return A.CurrentLap > B.CurrentLap ||
			(A.CurrentLap == B.CurrentLap && A.LastCheckpoint > B.LastCheckpoint);
	});

	// Create results
	int32 Position = 1;
	for (const FMGRacerState& Racer : SortedRacers)
	{
		FMGFinalRaceResult Result;
		Result.PlayerName = Racer.PlayerName;
		Result.Position = Position++;
		Result.TotalTime = Racer.bHasFinished ? Racer.FinishTime : Racer.TotalTime;
		Result.BestLap = Racer.BestLapTime < FLT_MAX ? Racer.BestLapTime : 0.0f;
		Result.LapTimes = Racer.LapTimes;
		Result.bDNF = Racer.bDNF || !Racer.bHasFinished;
		Result.bIsPlayer = Racer.bIsPlayer;
		Result.VehicleID = Racer.VehicleID;

		// Calculate rewards
		CalculateRewards(Result);

		FinalResults.Add(Result);
	}

	SetPhase(EMGRacePhase::Results);
	OnRaceResultsReady.Broadcast(FinalResults);
}

void AMGRaceGameMode::CalculateRewards(FMGFinalRaceResult& Result)
{
	// Base rewards by position
	const int32 PositionCashRewards[] = { 5000, 3500, 2500, 2000, 1500, 1200, 1000, 800 };
	const int32 PositionRepRewards[] = { 500, 350, 250, 200, 150, 120, 100, 80 };

	int32 PositionIndex = FMath::Clamp(Result.Position - 1, 0, 7);

	if (!Result.bDNF)
	{
		Result.CashEarned = PositionCashRewards[PositionIndex];
		Result.ReputationEarned = PositionRepRewards[PositionIndex];

		// Ranked match multiplier
		if (RaceConfig.bIsRanked)
		{
			Result.CashEarned = FMath::RoundToInt(Result.CashEarned * 1.5f);
			Result.ReputationEarned = FMath::RoundToInt(Result.ReputationEarned * 2.0f);
		}

		// Best lap bonus
		bool bHasFastestLap = true;
		for (const FMGFinalRaceResult& Other : FinalResults)
		{
			if (Other.BestLap > 0.0f && Other.BestLap < Result.BestLap)
			{
				bHasFastestLap = false;
				break;
			}
		}
		if (bHasFastestLap && Result.BestLap > 0.0f)
		{
			Result.CashEarned += 500;
			Result.ReputationEarned += 50;
		}
	}
	else
	{
		// DNF gets minimal rewards
		Result.CashEarned = 100;
		Result.ReputationEarned = 10;
	}
}

int32 AMGRaceGameMode::FindRacerIndex(AController* Controller) const
{
	for (int32 i = 0; i < Racers.Num(); i++)
	{
		if (Racers[i].Controller == Controller)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

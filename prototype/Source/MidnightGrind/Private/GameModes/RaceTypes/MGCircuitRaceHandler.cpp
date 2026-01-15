// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGCircuitRaceHandler.h"
#include "GameModes/MGRaceGameMode.h"

UMGCircuitRaceHandler::UMGCircuitRaceHandler()
{
}

void UMGCircuitRaceHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	Super::Initialize(InGameMode);

	if (InGameMode)
	{
		TotalLaps = InGameMode->GetRaceConfig().LapCount;
	}
}

void UMGCircuitRaceHandler::Reset()
{
	Super::Reset();

	BestLapTime = 0.0f;
	BestLapRacerIndex = -1;
	RacerLapTimes.Empty();
	HasCrossedStart.Empty();
	FinishOrder = 0;
	FinishPositions.Empty();
}

void UMGCircuitRaceHandler::OnRaceStarted()
{
	Super::OnRaceStarted();

	// All racers start at lap 0, will become lap 1 when they cross start line
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		TArray<FMGRacerData> Racers = GM->GetAllRacers();
		for (const FMGRacerData& Racer : Racers)
		{
			RacerLapTimes.Add(Racer.RacerIndex, TArray<float>());
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Circuit Race: Started with %d laps"), TotalLaps);
}

void UMGCircuitRaceHandler::OnRaceTick(float DeltaTime)
{
	Super::OnRaceTick(DeltaTime);

	// Circuit races don't need per-tick updates beyond base timing
}

void UMGCircuitRaceHandler::OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex)
{
	Super::OnCheckpointPassed(RacerIndex, CheckpointIndex);

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	// Checkpoint 0 is the start/finish line
	if (CheckpointIndex == 0)
	{
		// Track that this racer has crossed start
		if (!HasCrossedStart.Contains(RacerIndex))
		{
			HasCrossedStart.Add(RacerIndex);
		}
	}
}

void UMGCircuitRaceHandler::OnLapCompleted(int32 RacerIndex, float LapTime)
{
	Super::OnLapCompleted(RacerIndex, LapTime);

	// Store lap time
	if (TArray<float>* LapTimes = RacerLapTimes.Find(RacerIndex))
	{
		LapTimes->Add(LapTime);
	}

	// Check for new best lap
	if (BestLapTime <= 0.0f || LapTime < BestLapTime)
	{
		BestLapTime = LapTime;
		BestLapRacerIndex = RacerIndex;

		UE_LOG(LogTemp, Log, TEXT("Circuit Race: New best lap %.3f by racer %d"), LapTime, RacerIndex);
	}

	// Check completion
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		FMGRacerData RacerData = GM->GetRacerData(RacerIndex);

		// Lap completed broadcast includes what lap they just finished
		int32 CompletedLap = RacerData.CurrentLap - 1; // CurrentLap is now the next lap
		if (CompletedLap >= TotalLaps)
		{
			// Race complete for this racer
			FinishOrder++;
			FinishPositions.Add(RacerIndex);

			UE_LOG(LogTemp, Log, TEXT("Circuit Race: Racer %d finished in position %d"), RacerIndex, FinishOrder);
		}
		else if (CompletedLap == TotalLaps - 1)
		{
			// Final lap started
			UE_LOG(LogTemp, Log, TEXT("Circuit Race: Racer %d starting final lap"), RacerIndex);
		}
	}
}

EMGRaceCompletionResult UMGCircuitRaceHandler::CheckCompletionCondition(int32 RacerIndex)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return EMGRaceCompletionResult::InProgress;
	}

	FMGRacerData RacerData = GM->GetRacerData(RacerIndex);

	// Check if already marked as finished
	if (RacerData.bFinished)
	{
		return EMGRaceCompletionResult::Finished;
	}

	// Check if DNF
	if (RacerData.bDNF)
	{
		return EMGRaceCompletionResult::DNF;
	}

	// Check lap count - CurrentLap is 1-indexed, so when they're on lap TotalLaps+1, they've finished
	if (RacerData.CurrentLap > TotalLaps)
	{
		return EMGRaceCompletionResult::Finished;
	}

	// Check time limit
	FMGRaceConfig Config = GM->GetRaceConfig();
	if (Config.TimeLimit > 0.0f && GM->GetRaceTime() >= Config.TimeLimit)
	{
		// Time's up - anyone not finished is DNF
		return EMGRaceCompletionResult::DNF;
	}

	return EMGRaceCompletionResult::InProgress;
}

void UMGCircuitRaceHandler::CalculatePositions(TArray<int32>& OutPositions)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	OutPositions.SetNum(Racers.Num());

	// Create sortable list
	TArray<TPair<int32, float>> RacerProgress;
	for (const FMGRacerData& Racer : Racers)
	{
		// Progress = total distance (laps * checkpoints + current progress)
		float Progress = Racer.TotalDistance;

		// Finished racers get max progress + inverse finish time for tiebreaker
		if (Racer.bFinished)
		{
			Progress = 1000000.0f - Racer.FinishTime; // Higher = better
		}
		else if (Racer.bDNF)
		{
			Progress = -1.0f; // DNF at back
		}

		RacerProgress.Add(TPair<int32, float>(Racer.RacerIndex, Progress));
	}

	// Sort by progress (descending)
	RacerProgress.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	// Assign positions
	for (int32 i = 0; i < RacerProgress.Num(); ++i)
	{
		OutPositions[RacerProgress[i].Key] = i + 1;
	}
}

FText UMGCircuitRaceHandler::GetDisplayName() const
{
	return NSLOCTEXT("RaceType", "CircuitName", "Circuit Race");
}

FText UMGCircuitRaceHandler::GetDescription() const
{
	return NSLOCTEXT("RaceType", "CircuitDesc", "Complete all laps around the track. First across the finish line wins!");
}

FText UMGCircuitRaceHandler::GetProgressFormat() const
{
	return NSLOCTEXT("RaceType", "CircuitProgress", "LAP {0}/{1}");
}

int64 UMGCircuitRaceHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	int64 BaseCredits = Super::CalculateCreditsForPosition(Position, TotalRacers);

	// Lap bonus - more laps = more reward
	float LapMultiplier = 1.0f + (TotalLaps - 3) * 0.1f;
	LapMultiplier = FMath::Clamp(LapMultiplier, 1.0f, 2.0f);

	return static_cast<int64>(BaseCredits * LapMultiplier);
}

int32 UMGCircuitRaceHandler::GetRacerCurrentLap(int32 RacerIndex) const
{
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		FMGRacerData RacerData = GM->GetRacerData(RacerIndex);
		return FMath::Clamp(RacerData.CurrentLap, 1, TotalLaps);
	}
	return 1;
}

TArray<float> UMGCircuitRaceHandler::GetRacerLapTimes(int32 RacerIndex) const
{
	const TArray<float>* LapTimes = RacerLapTimes.Find(RacerIndex);
	return LapTimes ? *LapTimes : TArray<float>();
}

bool UMGCircuitRaceHandler::IsOnFinalLap(int32 RacerIndex) const
{
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		FMGRacerData RacerData = GM->GetRacerData(RacerIndex);
		return RacerData.CurrentLap == TotalLaps;
	}
	return false;
}

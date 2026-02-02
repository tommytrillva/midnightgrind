// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGSprintRaceHandler.h"
#include "GameModes/MGRaceGameMode.h"

UMGSprintRaceHandler::UMGSprintRaceHandler()
{
}

void UMGSprintRaceHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	Super::Initialize(InGameMode);

	if (InGameMode)
	{
		TotalSectors = InGameMode->GetCheckpointCount();
		FinishCheckpointIndex = TotalSectors - 1; // Last checkpoint is finish

		// Initialize best sector tracking
		BestSectorTimes.SetNum(TotalSectors);
		BestSectorRacers.SetNum(TotalSectors);
		for (int32 i = 0; i < TotalSectors; ++i)
		{
			BestSectorTimes[i] = 0.0f;
			BestSectorRacers[i] = -1;
		}
	}
}

void UMGSprintRaceHandler::Reset()
{
	Super::Reset();

	RacerProgress.Empty();
	RacerSectorTimes.Empty();
	FinishOrder = 0;

	for (int32 i = 0; i < BestSectorTimes.Num(); ++i)
	{
		BestSectorTimes[i] = 0.0f;
		BestSectorRacers[i] = -1;
	}
}

void UMGSprintRaceHandler::OnRaceStarted()
{
	Super::OnRaceStarted();

	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		TArray<FMGRacerData> Racers = GM->GetAllRacers();
		for (const FMGRacerData& Racer : Racers)
		{
			RacerProgress.Add(Racer.RacerIndex, 0.0f);

			TArray<float> SectorTimes;
			SectorTimes.SetNum(TotalSectors);
			for (int32 i = 0; i < TotalSectors; ++i)
			{
				SectorTimes[i] = 0.0f;
			}
			RacerSectorTimes.Add(Racer.RacerIndex, SectorTimes);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Sprint Race: Started with %d sectors"), TotalSectors);
}

void UMGSprintRaceHandler::OnRaceTick(float MGDeltaTime)
{
	Super::OnRaceTick(DeltaTime);

	// Could update continuous progress here if needed
	// For now, we rely on checkpoint-based progress
}

void UMGSprintRaceHandler::OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	FMGRacerData RacerData = GM->GetRacerData(RacerIndex);

	// Record sector time
	if (TArray<float>* SectorTimes = RacerSectorTimes.Find(RacerIndex))
	{
		if (SectorTimes->IsValidIndex(CheckpointIndex))
		{
			// Sector time = current race time (cumulative)
			// For actual sector delta, would need to subtract previous checkpoint time
			float SectorTime = RacerData.TotalTime;
			if (CheckpointIndex > 0 && (*SectorTimes)[CheckpointIndex - 1] > 0.0f)
			{
				SectorTime = RacerData.TotalTime - GetSectorTime(RacerIndex, CheckpointIndex - 1);
			}

			(*SectorTimes)[CheckpointIndex] = SectorTime;

			// Check for best sector
			if (BestSectorTimes[CheckpointIndex] <= 0.0f || SectorTime < BestSectorTimes[CheckpointIndex])
			{
				BestSectorTimes[CheckpointIndex] = SectorTime;
				BestSectorRacers[CheckpointIndex] = RacerIndex;

				UE_LOG(LogTemp, Log, TEXT("Sprint Race: New best sector %d time %.3f by racer %d"),
					CheckpointIndex, SectorTime, RacerIndex);
			}

			// Broadcast sector completion
			OnSectorComplete.Broadcast(RacerIndex, CheckpointIndex, SectorTime);
		}
	}

	// Update progress
	UpdateRacerProgress(RacerIndex, CheckpointIndex);

	// Check if this is the finish line
	if (CheckpointIndex == FinishCheckpointIndex)
	{
		FinishOrder++;
		UE_LOG(LogTemp, Log, TEXT("Sprint Race: Racer %d finished in position %d"), RacerIndex, FinishOrder);
	}
}

EMGRaceCompletionResult UMGSprintRaceHandler::CheckCompletionCondition(int32 RacerIndex)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return EMGRaceCompletionResult::InProgress;
	}

	FMGRacerData RacerData = GM->GetRacerData(RacerIndex);

	if (RacerData.bFinished)
	{
		return EMGRaceCompletionResult::Finished;
	}

	if (RacerData.bDNF)
	{
		return EMGRaceCompletionResult::DNF;
	}

	// In sprint, finish when last checkpoint is passed
	// This is tracked by the game mode through checkpoint passing
	if (RacerData.CurrentCheckpoint == 0 && RacerData.CurrentLap > 0)
	{
		// Wrapped around - finished the sprint
		return EMGRaceCompletionResult::Finished;
	}

	// Check time limit
	FMGRaceConfig Config = GM->GetRaceConfig();
	if (Config.TimeLimit > 0.0f && GM->GetRaceTime() >= Config.TimeLimit)
	{
		return EMGRaceCompletionResult::DNF;
	}

	return EMGRaceCompletionResult::InProgress;
}

void UMGSprintRaceHandler::CalculatePositions(TArray<int32>& OutPositions)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	OutPositions.SetNum(Racers.Num());

	// Sort by progress (checkpoint + distance)
	TArray<TPair<int32, float>> RacerRanking;
	for (const FMGRacerData& Racer : Racers)
	{
		float Score;

		if (Racer.bFinished)
		{
			// Finished racers: high base score minus finish time
			Score = 1000000.0f - Racer.FinishTime;
		}
		else if (Racer.bDNF)
		{
			// DNF: negative score
			Score = -1.0f;
		}
		else
		{
			// In progress: checkpoint progress + fractional distance
			Score = Racer.CurrentCheckpoint * 1000.0f + Racer.TotalDistance;
		}

		RacerRanking.Add(TPair<int32, float>(Racer.RacerIndex, Score));
	}

	// Sort descending (higher score = better position)
	RacerRanking.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	// Assign positions
	for (int32 i = 0; i < RacerRanking.Num(); ++i)
	{
		OutPositions[RacerRanking[i].Key] = i + 1;
	}
}

FText UMGSprintRaceHandler::GetDisplayName() const
{
	return NSLOCTEXT("RaceType", "SprintName", "Sprint Race");
}

FText UMGSprintRaceHandler::GetDescription() const
{
	return NSLOCTEXT("RaceType", "SprintDesc", "Point-to-point racing. First to the finish line wins!");
}

FText UMGSprintRaceHandler::GetProgressFormat() const
{
	// Shows distance remaining or sector
	return NSLOCTEXT("RaceType", "SprintProgress", "SECTOR {0}/{1}");
}

int64 UMGSprintRaceHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	int64 BaseCredits = Super::CalculateCreditsForPosition(Position, TotalRacers);

	// Sprint races are typically shorter, so slightly lower base reward
	// but bonus for sector performance could be added separately
	return static_cast<int64>(BaseCredits * 0.8f);
}

float UMGSprintRaceHandler::GetDistanceRemaining(int32 RacerIndex) const
{
	const float* Progress = RacerProgress.Find(RacerIndex);
	if (Progress && TotalDistance > 0.0f)
	{
		return FMath::Max(0.0f, TotalDistance - *Progress);
	}
	return TotalDistance;
}

float UMGSprintRaceHandler::GetProgressPercentage(int32 RacerIndex) const
{
	const float* Progress = RacerProgress.Find(RacerIndex);
	if (Progress && TotalDistance > 0.0f)
	{
		return FMath::Clamp((*Progress / TotalDistance) * 100.0f, 0.0f, 100.0f);
	}
	return 0.0f;
}

int32 UMGSprintRaceHandler::GetCurrentSector(int32 RacerIndex) const
{
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		FMGRacerData RacerData = GM->GetRacerData(RacerIndex);
		return RacerData.CurrentCheckpoint;
	}
	return 0;
}

float UMGSprintRaceHandler::GetSectorTime(int32 RacerIndex, int32 SectorIndex) const
{
	const TArray<float>* SectorTimes = RacerSectorTimes.Find(RacerIndex);
	if (SectorTimes && SectorTimes->IsValidIndex(SectorIndex))
	{
		return (*SectorTimes)[SectorIndex];
	}
	return 0.0f;
}

float UMGSprintRaceHandler::GetBestSectorTime(int32 SectorIndex) const
{
	if (BestSectorTimes.IsValidIndex(SectorIndex))
	{
		return BestSectorTimes[SectorIndex];
	}
	return 0.0f;
}

void UMGSprintRaceHandler::UpdateRacerProgress(int32 RacerIndex, int32 CheckpointIndex)
{
	if (TotalSectors <= 0)
	{
		return;
	}

	// Calculate progress as percentage of track based on checkpoint
	float ProgressPercent = static_cast<float>(CheckpointIndex + 1) / static_cast<float>(TotalSectors);
	float Progress = TotalDistance * ProgressPercent;

	RacerProgress.Add(RacerIndex, Progress);
}

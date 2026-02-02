// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "GameModes/MGRaceGameMode.h"

// Forward declarations for specific handlers
#include "GameModes/RaceTypes/MGCircuitRaceHandler.h"
#include "GameModes/RaceTypes/MGSprintRaceHandler.h"
#include "GameModes/RaceTypes/MGDriftRaceHandler.h"
#include "GameModes/RaceTypes/MGDragRaceHandler.h"
#include "GameModes/RaceTypes/MGTimeTrialHandler.h"

// ==========================================
// UMGRaceTypeHandler
// ==========================================

UMGRaceTypeHandler::UMGRaceTypeHandler()
{
}

void UMGRaceTypeHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	GameMode = InGameMode;
	RacerScores.Empty();
}

void UMGRaceTypeHandler::Activate()
{
	bIsActive = true;
}

void UMGRaceTypeHandler::Deactivate()
{
	bIsActive = false;
}

void UMGRaceTypeHandler::Reset()
{
	RacerScores.Empty();
}

void UMGRaceTypeHandler::OnCountdownStarted()
{
	// Base implementation - override in subclasses
}

void UMGRaceTypeHandler::OnRaceStarted()
{
	// Base implementation - override in subclasses
}

void UMGRaceTypeHandler::OnRaceTick(float DeltaTime)
{
	// Base implementation - override in subclasses
}

void UMGRaceTypeHandler::OnRacePaused()
{
	// Base implementation - override in subclasses
}

void UMGRaceTypeHandler::OnRaceResumed()
{
	// Base implementation - override in subclasses
}

void UMGRaceTypeHandler::OnRaceEnded()
{
	// Base implementation - override in subclasses
}

void UMGRaceTypeHandler::OnCheckpointPassed(int32 RacerIndex, int32 CheckpointIndex)
{
	// Broadcast sector completion
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		FMGRacerData RacerData = GM->GetRacerData(RacerIndex);
		OnSectorComplete.Broadcast(RacerIndex, CheckpointIndex, RacerData.CurrentLapTime);
	}
}

void UMGRaceTypeHandler::OnLapCompleted(int32 RacerIndex, float LapTime)
{
	// Base implementation - override in subclasses
}

EMGRaceCompletionResult UMGRaceTypeHandler::CheckCompletionCondition(int32 RacerIndex)
{
	// Base implementation - override in subclasses
	return EMGRaceCompletionResult::InProgress;
}

void UMGRaceTypeHandler::CalculatePositions(TArray<int32>& OutPositions)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	OutPositions.SetNum(Racers.Num());

	// Default: sort by progress (lap + checkpoint)
	TArray<TPair<int32, float>> RacerProgress;
	for (const FMGRacerData& Racer : Racers)
	{
		RacerProgress.Add(TPair<int32, float>(Racer.RacerIndex, Racer.TotalDistance));
	}

	RacerProgress.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value; // Higher progress = better position
	});

	for (int32 i = 0; i < RacerProgress.Num(); ++i)
	{
		OutPositions[RacerProgress[i].Key] = i + 1;
	}
}

float UMGRaceTypeHandler::GetRacerScore(int32 RacerIndex) const
{
	const float* Score = RacerScores.Find(RacerIndex);
	return Score ? *Score : 0.0f;
}

FText UMGRaceTypeHandler::GetProgressFormat() const
{
	return NSLOCTEXT("RaceType", "DefaultProgress", "Lap {0}/{1}");
}

int64 UMGRaceTypeHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	// Base credits by position
	static const int64 BaseCredits[] = {
		10000,  // 1st
		7000,   // 2nd
		5000,   // 3rd
		3500,   // 4th
		2500,   // 5th
		2000,   // 6th
		1500,   // 7th
		1000    // 8th+
	};

	int32 Index = FMath::Clamp(Position - 1, 0, 7);
	int64 Credits = BaseCredits[Index];

	// Scale slightly by field size
	float FieldMultiplier = 1.0f + (TotalRacers - 4) * 0.05f;
	FieldMultiplier = FMath::Clamp(FieldMultiplier, 1.0f, 1.5f);

	return static_cast<int64>(Credits * FieldMultiplier);
}

int32 UMGRaceTypeHandler::CalculateXPForPosition(int32 Position, int32 TotalRacers) const
{
	// Base XP by position
	static const int32 BaseXP[] = {
		500,    // 1st
		350,    // 2nd
		250,    // 3rd
		175,    // 4th
		125,    // 5th
		100,    // 6th
		75,     // 7th
		50      // 8th+
	};

	int32 Index = FMath::Clamp(Position - 1, 0, 7);
	return BaseXP[Index];
}

int32 UMGRaceTypeHandler::CalculateReputationEarned(int32 Position, bool bWon) const
{
	// Base reputation by position
	int32 BaseRep = FMath::Max(0, 100 - (Position - 1) * 15);

	// Bonus for winning
	if (bWon)
	{
		BaseRep += 50;
	}

	return BaseRep;
}

AMGRaceGameMode* UMGRaceTypeHandler::GetGameMode() const
{
	return GameMode.Get();
}

void UMGRaceTypeHandler::BroadcastScoreUpdate(int32 RacerIndex, float Delta, float Total, const FText& Reason, float Multiplier)
{
	FMGScoreUpdate Update;
	Update.RacerIndex = RacerIndex;
	Update.ScoreDelta = Delta;
	Update.TotalScore = Total;
	Update.ScoreReason = Reason;
	Update.Multiplier = Multiplier;

	OnScoreUpdated.Broadcast(Update);
}

// ==========================================
// UMGRaceTypeFactory
// ==========================================

UMGRaceTypeHandler* UMGRaceTypeFactory::CreateRaceTypeHandler(UObject* WorldContextObject, EMGRaceType RaceType)
{
	TSubclassOf<UMGRaceTypeHandler> HandlerClass = GetHandlerClassForType(RaceType);

	if (HandlerClass)
	{
		return NewObject<UMGRaceTypeHandler>(WorldContextObject, HandlerClass);
	}

	return nullptr;
}

TSubclassOf<UMGRaceTypeHandler> UMGRaceTypeFactory::GetHandlerClassForType(EMGRaceType RaceType)
{
	switch (RaceType)
	{
	case EMGRaceType::Circuit:
	case EMGRaceType::PinkSlip:
		return UMGCircuitRaceHandler::StaticClass();

	case EMGRaceType::Sprint:
		return UMGSprintRaceHandler::StaticClass();

	case EMGRaceType::Drift:
		return UMGDriftRaceHandler::StaticClass();

	case EMGRaceType::Drag:
		return UMGDragRaceHandler::StaticClass();

	case EMGRaceType::TimeTrial:
		return UMGTimeTrialHandler::StaticClass();

	default:
		return UMGCircuitRaceHandler::StaticClass();
	}
}

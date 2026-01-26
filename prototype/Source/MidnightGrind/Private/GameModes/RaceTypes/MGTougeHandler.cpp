// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGTougeHandler.h"
#include "GameModes/MGRaceConfiguration.h"

UMGTougeHandler::UMGTougeHandler()
{
}

void UMGTougeHandler::InitializeRace(const FMGRaceConfiguration& Config)
{
	Super::InitializeRace(Config);

	CurrentPhase = EMGTougePhase::FirstRun_P1Leads;
	CurrentRunNumber = 1;
	CurrentLeaderIndex = 0;
	bRaceComplete = false;
	OverallWinnerIndex = -1;
	TransitionTimer = 0.0f;
	bParticipant0Finished = false;
	bParticipant1Finished = false;
	CompletedRuns.Empty();

	// Initialize participants
	if (Config.Participants.Num() >= 2)
	{
		Participants[0].Vehicle = Config.Participants[0].Vehicle;
		Participants[1].Vehicle = Config.Participants[1].Vehicle;
	}

	// Initialize first run
	CurrentRun = FMGTougeRunData();
	CurrentRun.LeaderIndex = 0;
}

void UMGTougeHandler::StartRace()
{
	Super::StartRace();

	SetPhase(EMGTougePhase::FirstRun_P1Leads);
}

void UMGTougeHandler::UpdateRace(float DeltaTime)
{
	Super::UpdateRace(DeltaTime);

	if (bRaceComplete)
	{
		return;
	}

	switch (CurrentPhase)
	{
	case EMGTougePhase::FirstRun_P1Leads:
	case EMGTougePhase::FirstRun_P2Leads:
	case EMGTougePhase::SecondRun:
	case EMGTougePhase::Tiebreaker:
		CurrentRun.RunTime += DeltaTime;
		UpdateGap(DeltaTime);
		CheckRunCompletion();
		break;

	case EMGTougePhase::Transition:
		TransitionTimer -= DeltaTime;
		if (TransitionTimer <= 0.0f)
		{
			StartNextRun();
		}
		break;

	default:
		break;
	}
}

void UMGTougeHandler::EndRace()
{
	Super::EndRace();
	SetPhase(EMGTougePhase::Complete);
}

bool UMGTougeHandler::IsRaceComplete() const
{
	return bRaceComplete;
}

TArray<FMGRaceResult> UMGTougeHandler::GetResults() const
{
	TArray<FMGRaceResult> Results;

	for (int32 i = 0; i < 2; ++i)
	{
		FMGRaceResult Result;
		Result.ParticipantIndex = i;
		Result.Vehicle = Participants[i].Vehicle;
		Result.Position = (i == OverallWinnerIndex) ? 1 : 2;
		Result.TotalTime = Participants[i].BestTime;
		Result.bFinished = bRaceComplete;
		Results.Add(Result);
	}

	Results.Sort([](const FMGRaceResult& A, const FMGRaceResult& B)
	{
		return A.Position < B.Position;
	});

	return Results;
}

FText UMGTougeHandler::GetRaceTypeName() const
{
	return NSLOCTEXT("MG", "TougeDuel", "Touge Duel");
}

FMGTougeParticipant UMGTougeHandler::GetParticipant(int32 Index) const
{
	if (Index >= 0 && Index < 2)
	{
		return Participants[Index];
	}
	return FMGTougeParticipant();
}

void UMGTougeHandler::ReportCrash(int32 ParticipantIndex)
{
	if (ParticipantIndex < 0 || ParticipantIndex >= 2)
	{
		return;
	}

	Participants[ParticipantIndex].Crashes++;
	OnCrash.Broadcast(ParticipantIndex);

	// Determine result based on who crashed
	if (ParticipantIndex == CurrentLeaderIndex)
	{
		// Leader crashed - chaser wins
		CompleteRun(EMGTougeResultType::LeaderCrashed, 1 - CurrentLeaderIndex);
	}
	else
	{
		// Chaser crashed - leader wins
		CompleteRun(EMGTougeResultType::ChaserCrashed, CurrentLeaderIndex);
	}
}

void UMGTougeHandler::ReportFinish(int32 ParticipantIndex)
{
	if (ParticipantIndex == 0)
	{
		bParticipant0Finished = true;
	}
	else if (ParticipantIndex == 1)
	{
		bParticipant1Finished = true;
	}

	// Check if both finished
	if (bParticipant0Finished && bParticipant1Finished)
	{
		// Determine winner based on gap at finish
		if (CurrentRun.GapDistance > 0)
		{
			// Leader maintained gap
			CompleteRun(EMGTougeResultType::LeaderPulledAway, CurrentLeaderIndex);
		}
		else
		{
			// Chaser caught up
			CompleteRun(EMGTougeResultType::ChaserCaughtUp, 1 - CurrentLeaderIndex);
		}
	}
}

void UMGTougeHandler::UpdateGap(float DeltaTime)
{
	if (!Participants[0].Vehicle.IsValid() || !Participants[1].Vehicle.IsValid())
	{
		return;
	}

	FVector LeaderPos = Participants[CurrentLeaderIndex].Vehicle->GetActorLocation();
	FVector ChaserPos = Participants[1 - CurrentLeaderIndex].Vehicle->GetActorLocation();

	// Calculate gap (positive = leader ahead, negative = chaser ahead)
	FVector LeaderForward = Participants[CurrentLeaderIndex].Vehicle->GetActorForwardVector();
	FVector ToChaserVec = ChaserPos - LeaderPos;

	// Project onto forward direction
	float GapAlongPath = -FVector::DotProduct(ToChaserVec, LeaderForward);

	float OldGap = CurrentRun.GapDistance;
	CurrentRun.GapDistance = GapAlongPath;

	// Broadcast gap change if significant
	if (FMath::Abs(OldGap - CurrentRun.GapDistance) > 100.0f) // 1 meter threshold
	{
		OnGapChanged.Broadcast(CurrentRun.GapDistance, CurrentRun.GapDistance > 0.0f);
	}
}

void UMGTougeHandler::CheckRunCompletion()
{
	// Check time limit
	if (CurrentRun.RunTime >= MaxRunDuration)
	{
		// Time expired - whoever has advantage wins
		if (CurrentRun.GapDistance > 0)
		{
			CompleteRun(EMGTougeResultType::TimeExpired, CurrentLeaderIndex);
		}
		else
		{
			CompleteRun(EMGTougeResultType::TimeExpired, 1 - CurrentLeaderIndex);
		}
		return;
	}

	// Check for decisive gap
	if (CurrentRun.GapDistance > LeaderVictoryGap)
	{
		// Leader pulled away too far
		CompleteRun(EMGTougeResultType::LeaderPulledAway, CurrentLeaderIndex);
		return;
	}

	if (CurrentRun.GapDistance < -ChaserVictoryGap)
	{
		// Chaser passed the leader
		CompleteRun(EMGTougeResultType::ChaserCaughtUp, 1 - CurrentLeaderIndex);
		return;
	}
}

void UMGTougeHandler::StartNextRun()
{
	CurrentRunNumber++;

	// Reset finish flags
	bParticipant0Finished = false;
	bParticipant1Finished = false;

	// Swap leader
	CurrentLeaderIndex = 1 - CurrentLeaderIndex;

	// Initialize new run
	CurrentRun = FMGTougeRunData();
	CurrentRun.LeaderIndex = CurrentLeaderIndex;

	// Set appropriate phase
	if (CurrentRunNumber == 2)
	{
		SetPhase(EMGTougePhase::SecondRun);
	}
	else
	{
		SetPhase(EMGTougePhase::Tiebreaker);
	}
}

void UMGTougeHandler::CompleteRun(EMGTougeResultType Result, int32 RunWinnerIndex)
{
	CurrentRun.Result = Result;
	CurrentRun.RunWinnerIndex = RunWinnerIndex;

	// Update participant stats
	if (RunWinnerIndex >= 0 && RunWinnerIndex < 2)
	{
		Participants[RunWinnerIndex].RoundsWon++;

		// Track best time
		if (Participants[RunWinnerIndex].BestTime == 0.0f ||
			CurrentRun.RunTime < Participants[RunWinnerIndex].BestTime)
		{
			Participants[RunWinnerIndex].BestTime = CurrentRun.RunTime;
		}

		// Track specific outcomes
		if (Result == EMGTougeResultType::ChaserCaughtUp)
		{
			Participants[CurrentLeaderIndex].TimesCaughtAsLeader++;
		}
		else if (Result == EMGTougeResultType::LeaderPulledAway)
		{
			Participants[1 - CurrentLeaderIndex].TimesLostAsChaser++;
		}
	}

	// Store completed run
	CompletedRuns.Add(CurrentRun);

	OnRunComplete.Broadcast(CurrentRunNumber, RunWinnerIndex, Result);

	// Check for overall winner
	int32 Winner = -1;
	if (CheckForOverallWinner(Winner))
	{
		OverallWinnerIndex = Winner;
		bRaceComplete = true;
		SetPhase(EMGTougePhase::Complete);
	}
	else
	{
		// More runs needed
		SetPhase(EMGTougePhase::Transition);
		TransitionTimer = TransitionDuration;
	}
}

bool UMGTougeHandler::CheckForOverallWinner(int32& OutWinnerIndex)
{
	if (bBestOfThree)
	{
		// Best of 3 - need 2 wins
		if (Participants[0].RoundsWon >= 2)
		{
			OutWinnerIndex = 0;
			return true;
		}
		if (Participants[1].RoundsWon >= 2)
		{
			OutWinnerIndex = 1;
			return true;
		}
	}
	else
	{
		// First to win
		if (Participants[0].RoundsWon >= 1)
		{
			OutWinnerIndex = 0;
			return true;
		}
		if (Participants[1].RoundsWon >= 1)
		{
			OutWinnerIndex = 1;
			return true;
		}
	}

	return false;
}

void UMGTougeHandler::SetPhase(EMGTougePhase NewPhase)
{
	if (CurrentPhase != NewPhase)
	{
		CurrentPhase = NewPhase;
		OnPhaseChanged.Broadcast(NewPhase, CurrentLeaderIndex);
	}
}

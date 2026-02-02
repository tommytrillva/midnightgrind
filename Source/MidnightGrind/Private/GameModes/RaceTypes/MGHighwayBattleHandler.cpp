// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGHighwayBattleHandler.h"
#include "GameModes/MGRaceConfiguration.h"

UMGHighwayBattleHandler::UMGHighwayBattleHandler()
{
}

void UMGHighwayBattleHandler::InitializeRace(const FMGRaceConfiguration& Config)
{
	Super::InitializeRace(Config);

	CurrentState = EMGHighwayBattleState::WaitingToStart;
	bRaceComplete = false;
	WinnerIndex = -1;
	CurrentGap = 0.0f;
	LeaderIndex = -1;
	TimeInDecisiveGap = 0.0f;
	TotalRaceTime = 0.0f;

	// Initialize participants from config
	if (Config.Participants.Num() >= 2)
	{
		Participants[0].Vehicle = Config.Participants[0].Vehicle;
		Participants[1].Vehicle = Config.Participants[1].Vehicle;

		// Get race start position
		if (Participants[0].Vehicle.IsValid())
		{
			RaceStartPosition = Participants[0].Vehicle->GetActorLocation();
			RaceDirection = Participants[0].Vehicle->GetActorForwardVector();
		}
	}
}

void UMGHighwayBattleHandler::StartRace()
{
	Super::StartRace();

	CurrentState = EMGHighwayBattleState::Racing;
	TotalRaceTime = 0.0f;
}

void UMGHighwayBattleHandler::UpdateRace(float MGDeltaTime)
{
	Super::UpdateRace(DeltaTime);

	if (bRaceComplete)
	{
		return;
	}

	TotalRaceTime += DeltaTime;

	// Check max duration
	if (MaxRaceDuration > 0.0f && TotalRaceTime >= MaxRaceDuration)
	{
		// Time expired - whoever is ahead wins
		if (LeaderIndex >= 0)
		{
			WinnerIndex = LeaderIndex;
			CurrentState = EMGHighwayBattleState::Finished;
			bRaceComplete = true;
			OnGapAchieved.Broadcast(Participants[WinnerIndex].Vehicle.Get(), CurrentGap);
		}
		return;
	}

	UpdateParticipants(DeltaTime);
	UpdateGap();
	UpdateBattleState(DeltaTime);
}

void UMGHighwayBattleHandler::EndRace()
{
	Super::EndRace();
	CurrentState = EMGHighwayBattleState::Finished;
}

bool UMGHighwayBattleHandler::IsRaceComplete() const
{
	return bRaceComplete;
}

TArray<FMGRaceResult> UMGHighwayBattleHandler::GetResults() const
{
	TArray<FMGRaceResult> Results;

	for (int32 i = 0; i < 2; ++i)
	{
		FMGRaceResult Result;
		Result.ParticipantIndex = i;
		Result.Vehicle = Participants[i].Vehicle;
		Result.Position = (i == WinnerIndex) ? 1 : 2;
		Result.TotalTime = TotalRaceTime;
		Result.TopSpeed = Participants[i].TopSpeedAchieved;
		Result.bFinished = bRaceComplete;
		Results.Add(Result);
	}

	// Sort by position
	Results.Sort([](const FMGRaceResult& A, const FMGRaceResult& B)
	{
		return A.Position < B.Position;
	});

	return Results;
}

FText UMGHighwayBattleHandler::GetRaceTypeName() const
{
	return NSLOCTEXT("MG", "HighwayBattle", "Highway Battle");
}

FMGHighwayBattleParticipant UMGHighwayBattleHandler::GetParticipant(int32 Index) const
{
	if (Index >= 0 && Index < 2)
	{
		return Participants[Index];
	}
	return FMGHighwayBattleParticipant();
}

AActor* UMGHighwayBattleHandler::GetCurrentLeader() const
{
	if (LeaderIndex >= 0 && LeaderIndex < 2)
	{
		return Participants[LeaderIndex].Vehicle.Get();
	}
	return nullptr;
}

float UMGHighwayBattleHandler::GetDecisiveGapProgress() const
{
	if (CurrentState == EMGHighwayBattleState::DecisiveGap)
	{
		return TimeInDecisiveGap / RequiredGapTime;
	}
	return 0.0f;
}

void UMGHighwayBattleHandler::UpdateParticipants(float MGDeltaTime)
{
	for (int32 i = 0; i < 2; ++i)
	{
		if (!Participants[i].Vehicle.IsValid())
		{
			continue;
		}

		AActor* Vehicle = Participants[i].Vehicle.Get();

		// Update speed
		FVector Velocity = Vehicle->GetVelocity();
		float SpeedCmPerSec = Velocity.Size();
		Participants[i].CurrentSpeed = SpeedCmPerSec * 0.036f; // cm/s to km/h

		// Update top speed
		if (Participants[i].CurrentSpeed > Participants[i].TopSpeedAchieved)
		{
			Participants[i].TopSpeedAchieved = Participants[i].CurrentSpeed;
		}

		// Update distance traveled
		Participants[i].DistanceTraveled += SpeedCmPerSec * DeltaTime;
	}
}

void UMGHighwayBattleHandler::UpdateGap()
{
	CurrentGap = CalculateGapDistance();

	int32 NewLeader = DetermineLeader();

	if (NewLeader != LeaderIndex && NewLeader >= 0)
	{
		int32 OldLeader = LeaderIndex;
		LeaderIndex = NewLeader;

		// Update leader flags
		Participants[0].bIsLeader = (LeaderIndex == 0);
		Participants[1].bIsLeader = (LeaderIndex == 1);

		// Update distance to opponent
		Participants[0].DistanceToOpponent = CurrentGap;
		Participants[1].DistanceToOpponent = CurrentGap;

		OnLeadChanged.Broadcast(Participants[LeaderIndex].Vehicle.Get(), CurrentGap);

		// Reset decisive gap timer on lead change
		TimeInDecisiveGap = 0.0f;
	}
}

void UMGHighwayBattleHandler::UpdateBattleState(float MGDeltaTime)
{
	// Check minimum speed requirement
	bool bBothFastEnough = true;
	for (int32 i = 0; i < 2; ++i)
	{
		if (Participants[i].CurrentSpeed < MinimumSpeedForGap)
		{
			bBothFastEnough = false;
			break;
		}
	}

	if (!bBothFastEnough)
	{
		// Gap doesn't count if someone is too slow
		TimeInDecisiveGap = 0.0f;
		if (CurrentState != EMGHighwayBattleState::Racing)
		{
			CurrentState = EMGHighwayBattleState::Racing;
		}
		return;
	}

	if (CurrentGap >= RequiredGap)
	{
		// Has decisive gap
		if (CurrentState != EMGHighwayBattleState::DecisiveGap)
		{
			CurrentState = EMGHighwayBattleState::DecisiveGap;
			TimeInDecisiveGap = 0.0f;
		}

		TimeInDecisiveGap += DeltaTime;

		if (LeaderIndex >= 0)
		{
			Participants[LeaderIndex].TimeWithDecisiveGap = TimeInDecisiveGap;
			OnDecisiveGapProgress.Broadcast(Participants[LeaderIndex].Vehicle.Get(), TimeInDecisiveGap / RequiredGapTime);
		}

		// Check for victory
		if (TimeInDecisiveGap >= RequiredGapTime)
		{
			WinnerIndex = LeaderIndex;
			CurrentState = EMGHighwayBattleState::Finished;
			bRaceComplete = true;

			OnGapAchieved.Broadcast(Participants[WinnerIndex].Vehicle.Get(), CurrentGap);
		}
	}
	else if (CurrentGap > 5000.0f) // 50 meters - building gap
	{
		CurrentState = EMGHighwayBattleState::BuildingGap;
		TimeInDecisiveGap = 0.0f;
	}
	else
	{
		CurrentState = EMGHighwayBattleState::Racing;
		TimeInDecisiveGap = 0.0f;
	}
}

float UMGHighwayBattleHandler::CalculateGapDistance() const
{
	if (!Participants[0].Vehicle.IsValid() || !Participants[1].Vehicle.IsValid())
	{
		return 0.0f;
	}

	FVector Pos0 = Participants[0].Vehicle->GetActorLocation();
	FVector Pos1 = Participants[1].Vehicle->GetActorLocation();

	// Project onto race direction for accurate gap measurement
	float Progress0 = FVector::DotProduct(Pos0 - RaceStartPosition, RaceDirection);
	float Progress1 = FVector::DotProduct(Pos1 - RaceStartPosition, RaceDirection);

	return FMath::Abs(Progress0 - Progress1);
}

int32 UMGHighwayBattleHandler::DetermineLeader() const
{
	if (!Participants[0].Vehicle.IsValid() || !Participants[1].Vehicle.IsValid())
	{
		return -1;
	}

	FVector Pos0 = Participants[0].Vehicle->GetActorLocation();
	FVector Pos1 = Participants[1].Vehicle->GetActorLocation();

	// Project onto race direction
	float Progress0 = FVector::DotProduct(Pos0 - RaceStartPosition, RaceDirection);
	float Progress1 = FVector::DotProduct(Pos1 - RaceStartPosition, RaceDirection);

	return (Progress0 > Progress1) ? 0 : 1;
}

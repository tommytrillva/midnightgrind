// Copyright Midnight Grind. All Rights Reserved.

#include "Core/MGGameState.h"
#include "Core/MGPlayerState.h"
#include "Net/UnrealNetwork.h"

AMGGameState::AMGGameState()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
	NetUpdateFrequency = 30.0f;
}

void AMGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGGameState, CurrentRacePhase);
	DOREPLIFETIME(AMGGameState, RaceSettings);
	DOREPLIFETIME(AMGGameState, CountdownTime);
	DOREPLIFETIME(AMGGameState, RaceElapsedTime);
	DOREPLIFETIME(AMGGameState, RaceStartServerTime);
	DOREPLIFETIME(AMGGameState, Positions);
	DOREPLIFETIME(AMGGameState, BestOverallLapTime);
	DOREPLIFETIME(AMGGameState, BestLapHolder);
	DOREPLIFETIME(AMGGameState, FinishedCount);
	DOREPLIFETIME(AMGGameState, TotalRacerCount);
}

void AMGGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		// Update countdown on server
		if (CurrentRacePhase == EMGGlobalRacePhase::Countdown)
		{
			TickCountdown(DeltaSeconds);
		}

		// Update race time
		if (CurrentRacePhase == EMGGlobalRacePhase::Racing || CurrentRacePhase == EMGGlobalRacePhase::Finishing)
		{
			RaceElapsedTime = GetServerWorldTimeSeconds() - RaceStartServerTime;
		}
	}
	else
	{
		// Client-side countdown tick detection
		if (CurrentRacePhase == EMGGlobalRacePhase::Countdown)
		{
			int32 CurrentTick = FMath::CeilToInt(CountdownTime);
			if (CurrentTick != PreviousCountdownTick && CurrentTick > 0)
			{
				OnCountdownUpdate.Broadcast(CurrentTick);
				PreviousCountdownTick = CurrentTick;
			}
		}
	}
}

void AMGGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if (AMGPlayerState* MGPlayerState = Cast<AMGPlayerState>(PlayerState))
	{
		OnPlayerJoined.Broadcast(MGPlayerState);
	}
}

void AMGGameState::RemovePlayerState(APlayerState* PlayerState)
{
	if (AMGPlayerState* MGPlayerState = Cast<AMGPlayerState>(PlayerState))
	{
		OnPlayerLeft.Broadcast(MGPlayerState);
	}

	Super::RemovePlayerState(PlayerState);
}

int32 AMGGameState::GetPositionForPlayer(AMGPlayerState* PlayerState) const
{
	for (const FMGRacePositionEntry& Entry : Positions)
	{
		if (Entry.PlayerState == PlayerState)
		{
			return Entry.Position;
		}
	}
	return 0;
}

AMGPlayerState* AMGGameState::GetLeader() const
{
	if (Positions.Num() > 0)
	{
		return Positions[0].PlayerState;
	}
	return nullptr;
}

TArray<AMGPlayerState*> AMGGameState::GetMGPlayerStates() const
{
	TArray<AMGPlayerState*> Result;
	for (APlayerState* PS : PlayerArray)
	{
		if (AMGPlayerState* MGPS = Cast<AMGPlayerState>(PS))
		{
			Result.Add(MGPS);
		}
	}
	return Result;
}

int32 AMGGameState::GetReadyPlayerCount() const
{
	int32 ReadyCount = 0;
	for (APlayerState* PS : PlayerArray)
	{
		if (AMGPlayerState* MGPS = Cast<AMGPlayerState>(PS))
		{
			if (MGPS->IsReady())
			{
				ReadyCount++;
			}
		}
	}
	return ReadyCount;
}

bool AMGGameState::AreAllPlayersReady() const
{
	if (PlayerArray.Num() == 0)
	{
		return false;
	}

	for (APlayerState* PS : PlayerArray)
	{
		if (AMGPlayerState* MGPS = Cast<AMGPlayerState>(PS))
		{
			if (!MGPS->IsReady())
			{
				return false;
			}
		}
	}
	return true;
}

AMGPlayerState* AMGGameState::GetSessionHost() const
{
	for (APlayerState* PS : PlayerArray)
	{
		if (AMGPlayerState* MGPS = Cast<AMGPlayerState>(PS))
		{
			if (MGPS->IsSessionHost())
			{
				return MGPS;
			}
		}
	}
	return nullptr;
}

// ==========================================
// SERVER FUNCTIONS
// ==========================================

void AMGGameState::AuthSetRacePhase(EMGGlobalRacePhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentRacePhase != NewPhase)
	{
		CurrentRacePhase = NewPhase;
		OnRep_RacePhase();
	}
}

void AMGGameState::AuthSetRaceSettings(const FMGReplicatedRaceSettings& Settings)
{
	if (!HasAuthority())
	{
		return;
	}

	RaceSettings = Settings;
}

void AMGGameState::AuthStartCountdown(float Duration)
{
	if (!HasAuthority())
	{
		return;
	}

	CountdownTime = Duration;
	PreviousCountdownTick = FMath::CeilToInt(Duration);
	AuthSetRacePhase(EMGGlobalRacePhase::Countdown);
}

void AMGGameState::AuthStartRace()
{
	if (!HasAuthority())
	{
		return;
	}

	RaceStartServerTime = GetServerWorldTimeSeconds();
	RaceElapsedTime = 0.0f;
	FinishedCount = 0;
	TotalRacerCount = PlayerArray.Num(); // + AI count
	BestOverallLapTime = 0.0f;
	BestLapHolder = nullptr;

	AuthSetRacePhase(EMGGlobalRacePhase::Racing);
	OnRaceStart.Broadcast();
}

void AMGGameState::AuthMarkPlayerFinished(AMGPlayerState* PlayerState, float FinishTime)
{
	if (!HasAuthority() || !PlayerState)
	{
		return;
	}

	FinishedCount++;

	// Update their position entry
	for (FMGRacePositionEntry& Entry : Positions)
	{
		if (Entry.PlayerState == PlayerState)
		{
			Entry.bHasFinished = true;
			Entry.FinishTime = FinishTime;
			break;
		}
	}

	OnRacerFinished.Broadcast(PlayerState);

	// Check if all racers finished
	if (FinishedCount >= TotalRacerCount)
	{
		AuthSetRacePhase(EMGGlobalRacePhase::Results);
		OnAllRacersFinished.Broadcast();
	}
	else if (CurrentRacePhase == EMGGlobalRacePhase::Racing)
	{
		// First player finished, enter finishing phase
		AuthSetRacePhase(EMGGlobalRacePhase::Finishing);
	}
}

void AMGGameState::AuthUpdatePositions(const TArray<FMGRacePositionEntry>& NewPositions)
{
	if (!HasAuthority())
	{
		return;
	}

	Positions = NewPositions;
	OnRep_Positions();
}

void AMGGameState::AuthReportBestLap(AMGPlayerState* PlayerState, float LapTime)
{
	if (!HasAuthority())
	{
		return;
	}

	if (BestOverallLapTime <= 0.0f || LapTime < BestOverallLapTime)
	{
		BestOverallLapTime = LapTime;
		BestLapHolder = PlayerState;
		OnNewBestLap.Broadcast(PlayerState, LapTime);
	}
}

void AMGGameState::AuthEndRace()
{
	if (!HasAuthority())
	{
		return;
	}

	AuthSetRacePhase(EMGGlobalRacePhase::Results);
}

// ==========================================
// REP NOTIFIES
// ==========================================

void AMGGameState::OnRep_RacePhase()
{
	OnRacePhaseChanged.Broadcast(CurrentRacePhase);

	// Reset countdown tick on phase change
	if (CurrentRacePhase == EMGGlobalRacePhase::Countdown)
	{
		PreviousCountdownTick = FMath::CeilToInt(CountdownTime);
	}
}

void AMGGameState::OnRep_Positions()
{
	OnPositionsUpdated.Broadcast(Positions);
}

// ==========================================
// INTERNAL
// ==========================================

void AMGGameState::TickCountdown(float DeltaSeconds)
{
	CountdownTime -= DeltaSeconds;

	int32 CurrentTick = FMath::CeilToInt(CountdownTime);

	if (CurrentTick != PreviousCountdownTick && CurrentTick >= 0)
	{
		OnCountdownUpdate.Broadcast(CurrentTick);
		PreviousCountdownTick = CurrentTick;
	}

	if (CountdownTime <= 0.0f)
	{
		AuthStartRace();
	}
}

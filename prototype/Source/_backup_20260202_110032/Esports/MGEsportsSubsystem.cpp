// Copyright Midnight Grind. All Rights Reserved.

#include "Esports/MGEsportsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGEsportsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UMGEsportsSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDirectorTimerHandle);
	}
	Super::Deinitialize();
}

void UMGEsportsSubsystem::CreateTournament(const FMGTournamentInfo& Info)
{
	FMGTournamentInfo NewTournament = Info;
	if (NewTournament.TournamentID.IsEmpty())
	{
		NewTournament.TournamentID = FGuid::NewGuid().ToString();
	}

	Tournaments.Add(NewTournament.TournamentID, NewTournament);
	TournamentBrackets.Add(NewTournament.TournamentID, TArray<FMGTournamentMatch>());
	TournamentParticipants.Add(NewTournament.TournamentID, TArray<FMGParticipantStats>());
}

void UMGEsportsSubsystem::StartTournament(const FString& TournamentID)
{
	if (FMGTournamentInfo* Tournament = Tournaments.Find(TournamentID))
	{
		Tournament->bIsLive = true;
		Tournament->CurrentRound = 1;
		GenerateBracket(TournamentID);
	}
}

void UMGEsportsSubsystem::AdvanceToNextRound(const FString& TournamentID)
{
	if (FMGTournamentInfo* Tournament = Tournaments.Find(TournamentID))
	{
		if (Tournament->CurrentRound < Tournament->TotalRounds)
		{
			Tournament->CurrentRound++;
			OnTournamentAdvanced.Broadcast(*Tournament, Tournament->CurrentRound);
		}
	}
}

void UMGEsportsSubsystem::RegisterParticipant(const FString& TournamentID, const FMGParticipantStats& Participant)
{
	if (TArray<FMGParticipantStats>* Participants = TournamentParticipants.Find(TournamentID))
	{
		if (FMGTournamentInfo* Tournament = Tournaments.Find(TournamentID))
		{
			if (Participants->Num() < Tournament->MaxParticipants)
			{
				FMGParticipantStats NewParticipant = Participant;
				NewParticipant.Seed = Participants->Num() + 1;
				Participants->Add(NewParticipant);
			}
		}
	}
}

FMGTournamentInfo UMGEsportsSubsystem::GetTournamentInfo(const FString& TournamentID) const
{
	if (const FMGTournamentInfo* Tournament = Tournaments.Find(TournamentID))
	{
		return *Tournament;
	}
	return FMGTournamentInfo();
}

TArray<FMGTournamentMatch> UMGEsportsSubsystem::GetBracket(const FString& TournamentID) const
{
	if (const TArray<FMGTournamentMatch>* Bracket = TournamentBrackets.Find(TournamentID))
	{
		return *Bracket;
	}
	return TArray<FMGTournamentMatch>();
}

TArray<FMGParticipantStats> UMGEsportsSubsystem::GetStandings(const FString& TournamentID) const
{
	if (const TArray<FMGParticipantStats>* Participants = TournamentParticipants.Find(TournamentID))
	{
		TArray<FMGParticipantStats> Sorted = *Participants;
		Sorted.Sort([](const FMGParticipantStats& A, const FMGParticipantStats& B)
		{
			if (A.TotalPoints != B.TotalPoints)
				return A.TotalPoints > B.TotalPoints;
			return A.AverageFinishPosition < B.AverageFinishPosition;
		});
		return Sorted;
	}
	return TArray<FMGParticipantStats>();
}

void UMGEsportsSubsystem::StartMatch(const FString& MatchID)
{
	// Find match in brackets
	for (auto& Pair : TournamentBrackets)
	{
		for (FMGTournamentMatch& Match : Pair.Value)
		{
			if (Match.MatchID == MatchID)
			{
				Match.State = EMGMatchState::InProgress;
				CurrentMatch = Match;
				OnMatchStateChanged.Broadcast(CurrentMatch);
				return;
			}
		}
	}
}

void UMGEsportsSubsystem::EndMatch(const FString& MatchID, const FString& WinnerID)
{
	for (auto& Pair : TournamentBrackets)
	{
		for (FMGTournamentMatch& Match : Pair.Value)
		{
			if (Match.MatchID == MatchID)
			{
				Match.State = EMGMatchState::Completed;
				Match.WinnerID = WinnerID;
				CurrentMatch = Match;

				// Update participant stats
				if (TArray<FMGParticipantStats>* Participants = TournamentParticipants.Find(Match.TournamentID))
				{
					for (FMGParticipantStats& Participant : *Participants)
					{
						if (Participant.PlayerID == WinnerID)
						{
							Participant.Wins++;
							Participant.TotalPoints += 3;
						}
						else if (Match.ParticipantIDs.Contains(Participant.PlayerID))
						{
							Participant.Losses++;
						}
					}
				}

				OnMatchStateChanged.Broadcast(CurrentMatch);
				return;
			}
		}
	}
}

void UMGEsportsSubsystem::RecordRaceResult(const FString& MatchID, const TArray<FString>& FinishOrder)
{
	// Update stats for all finishers
	if (TArray<FMGParticipantStats>* Participants = TournamentParticipants.Find(CurrentMatch.TournamentID))
	{
		for (int32 Position = 0; Position < FinishOrder.Num(); Position++)
		{
			const FString& PlayerID = FinishOrder[Position];
			for (FMGParticipantStats& Participant : *Participants)
			{
				if (Participant.PlayerID == PlayerID)
				{
					// Update average position
					int32 TotalRaces = Participant.Wins + Participant.Losses;
					float CurrentTotal = Participant.AverageFinishPosition * TotalRaces;
					Participant.AverageFinishPosition = (CurrentTotal + Position + 1) / (TotalRaces + 1);

					// Points based on position
					int32 Points[] = { 25, 18, 15, 12, 10, 8, 6, 4, 2, 1 };
					if (Position < 10)
					{
						Participant.TotalPoints += Points[Position];
					}
					break;
				}
			}
		}
	}
}

void UMGEsportsSubsystem::JoinAsCaster(const FMGCasterInfo& CasterInfo)
{
	LocalCaster = CasterInfo;
	bIsCasting = true;

	// Initialize caster tools
	CasterTools = FMGCasterToolsState();
	CasterTools.bShowExtendedStats = true;
	CasterTools.bShowGapTiming = true;
}

void UMGEsportsSubsystem::LeaveCaster()
{
	bIsCasting = false;
	DisableAutoDirector();
}

void UMGEsportsSubsystem::SetCasterToolsState(const FMGCasterToolsState& State)
{
	CasterTools = State;
}

void UMGEsportsSubsystem::FocusOnPlayer(const FString& PlayerID)
{
	CasterTools.FocusedPlayerID = PlayerID;

	if (bAutoDirectorEnabled)
	{
		OverrideAutoDirector(PlayerID, 10.0f);
	}
}

void UMGEsportsSubsystem::SetComparisonPlayers(const TArray<FString>& PlayerIDs)
{
	CasterTools.ComparePlayerIDs = PlayerIDs;
}

FMGParticipantStats UMGEsportsSubsystem::GetLivePlayerStats(const FString& PlayerID) const
{
	if (const TArray<FMGParticipantStats>* Participants = TournamentParticipants.Find(CurrentMatch.TournamentID))
	{
		for (const FMGParticipantStats& Participant : *Participants)
		{
			if (Participant.PlayerID == PlayerID)
			{
				return Participant;
			}
		}
	}
	return FMGParticipantStats();
}

void UMGEsportsSubsystem::EnableAutoDirector(const FMGAutoDirectorSettings& Settings)
{
	AutoDirectorSettings = Settings;
	bAutoDirectorEnabled = true;
	TimeSinceLastCameraSwitch = 0.0f;

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGEsportsSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			AutoDirectorTimerHandle,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->UpdateAutoDirector(0.1f);
				}
			},
			0.1f,
			true
		);
	}
}

void UMGEsportsSubsystem::DisableAutoDirector()
{
	bAutoDirectorEnabled = false;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoDirectorTimerHandle);
	}
}

void UMGEsportsSubsystem::OverrideAutoDirector(const FString& FocusPlayerID, float Duration)
{
	CurrentAutoDirectorFocus = FocusPlayerID;
	TimeSinceLastCameraSwitch = AutoDirectorSettings.MaxCameraDuration - Duration;
	OnAutoDirectorCameraSwitch.Broadcast(FocusPlayerID);
}

void UMGEsportsSubsystem::MarkReplayMoment(const FText& Label)
{
	FMGInstantReplay Replay;
	Replay.ReplayID = FGuid::NewGuid().ToString();
	Replay.Label = Label;
	Replay.StartTime = 0.0f; // Would be actual game time
	Replay.EndTime = 5.0f;
	Replay.bIsAutoGenerated = false;

	AvailableReplays.Add(Replay);
	OnReplayAvailable.Broadcast(Replay);

	// Limit stored replays
	while (AvailableReplays.Num() > MaxReplaysStored)
	{
		AvailableReplays.RemoveAt(0);
	}
}

void UMGEsportsSubsystem::PlayInstantReplay(const FMGInstantReplay& Replay)
{
	bReplayPlaying = true;
	// Would trigger actual replay system
}

void UMGEsportsSubsystem::StopReplay()
{
	bReplayPlaying = false;
}

void UMGEsportsSubsystem::UpdateAutoDirector(float DeltaTime)
{
	if (!bAutoDirectorEnabled || !IsMatchInProgress())
		return;

	TimeSinceLastCameraSwitch += DeltaTime;

	// Check if we should switch cameras
	if (TimeSinceLastCameraSwitch >= AutoDirectorSettings.MinCameraDuration)
	{
		FString NewFocus = DetermineNextFocus();

		if (!NewFocus.IsEmpty() && NewFocus != CurrentAutoDirectorFocus)
		{
			CurrentAutoDirectorFocus = NewFocus;
			TimeSinceLastCameraSwitch = 0.0f;
			OnAutoDirectorCameraSwitch.Broadcast(NewFocus);
		}
	}

	// Force switch after max duration
	if (TimeSinceLastCameraSwitch >= AutoDirectorSettings.MaxCameraDuration)
	{
		FString NewFocus = DetermineNextFocus();
		if (!NewFocus.IsEmpty())
		{
			CurrentAutoDirectorFocus = NewFocus;
			TimeSinceLastCameraSwitch = 0.0f;
			OnAutoDirectorCameraSwitch.Broadcast(NewFocus);
		}
	}
}

void UMGEsportsSubsystem::GenerateAutoReplay(const TArray<FString>& InvolvedPlayers, float Duration)
{
	if (!AutoDirectorSettings.bAutoReplay)
		return;

	FMGInstantReplay Replay;
	Replay.ReplayID = FGuid::NewGuid().ToString();
	Replay.Label = FText::FromString(TEXT("Auto Replay"));
	Replay.InvolvedPlayers = InvolvedPlayers;
	Replay.StartTime = 0.0f;
	Replay.EndTime = Duration;
	Replay.bIsAutoGenerated = true;

	AvailableReplays.Add(Replay);
	OnReplayAvailable.Broadcast(Replay);
}

void UMGEsportsSubsystem::GenerateBracket(const FString& TournamentID)
{
	const FMGTournamentInfo* Tournament = Tournaments.Find(TournamentID);
	const TArray<FMGParticipantStats>* Participants = TournamentParticipants.Find(TournamentID);
	TArray<FMGTournamentMatch>* Bracket = TournamentBrackets.Find(TournamentID);

	if (!Tournament || !Participants || !Bracket)
		return;

	Bracket->Empty();

	// Simple single elimination bracket generation
	int32 NumParticipants = Participants->Num();
	int32 NumFirstRoundMatches = NumParticipants / 2;

	for (int32 i = 0; i < NumFirstRoundMatches; i++)
	{
		FMGTournamentMatch Match;
		Match.MatchID = FGuid::NewGuid().ToString();
		Match.TournamentID = TournamentID;
		Match.Round = 1;
		Match.MatchNumber = i + 1;
		Match.State = EMGMatchState::Scheduled;

		// Seed matching (1 vs last, 2 vs second-to-last, etc.)
		int32 HighSeedIndex = i;
		int32 LowSeedIndex = NumParticipants - 1 - i;

		if (HighSeedIndex < Participants->Num())
			Match.ParticipantIDs.Add((*Participants)[HighSeedIndex].PlayerID);
		if (LowSeedIndex < Participants->Num() && LowSeedIndex != HighSeedIndex)
			Match.ParticipantIDs.Add((*Participants)[LowSeedIndex].PlayerID);

		Bracket->Add(Match);
	}
}

FString UMGEsportsSubsystem::DetermineNextFocus()
{
	// Would analyze race state to determine most interesting focus
	// Based on AutoDirectorMode:
	// - BattlesFocus: Find closest battles
	// - LeaderFocus: Focus on P1
	// - DramaFocus: Follow rivalries, comebacks, close finishes
	// - Balanced: Rotate through interesting situations

	if (CurrentMatch.ParticipantIDs.Num() > 0)
	{
		// For now, cycle through participants
		int32 CurrentIndex = CurrentMatch.ParticipantIDs.Find(CurrentAutoDirectorFocus);
		int32 NextIndex = (CurrentIndex + 1) % CurrentMatch.ParticipantIDs.Num();
		return CurrentMatch.ParticipantIDs[NextIndex];
	}

	return FString();
}

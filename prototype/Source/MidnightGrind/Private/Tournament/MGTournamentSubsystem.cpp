// Copyright Midnight Grind. All Rights Reserved.

#include "Tournament/MGTournamentSubsystem.h"
#include "Online/MGOnlineSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGTournamentSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Get online subsystem
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		OnlineSubsystem = GameInstance->GetSubsystem<UMGOnlineSubsystem>();
	}

	// Mock player ID
	LocalPlayerID = TEXT("LocalPlayer_001");

	// Load mock tournament data
	LoadMockTournaments();

	// Start state update timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			StateUpdateTimerHandle,
			this,
			&UMGTournamentSubsystem::UpdateTournamentStates,
			60.0f, // Check every minute
			true
		);
	}
}

void UMGTournamentSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StateUpdateTimerHandle);
	}
	Super::Deinitialize();
}

// ==========================================
// TOURNAMENT BROWSING
// ==========================================

TArray<FMGTournamentData> UMGTournamentSubsystem::GetAvailableTournaments() const
{
	TArray<FMGTournamentData> Available;
	for (const FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.State != EMGTournamentState::Completed &&
			Tournament.State != EMGTournamentState::Cancelled)
		{
			Available.Add(Tournament);
		}
	}
	return Available;
}

TArray<FMGTournamentData> UMGTournamentSubsystem::GetTournamentsByState(EMGTournamentState State) const
{
	TArray<FMGTournamentData> Result;
	for (const FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.State == State)
		{
			Result.Add(Tournament);
		}
	}
	return Result;
}

TArray<FMGTournamentData> UMGTournamentSubsystem::GetFeaturedTournaments() const
{
	TArray<FMGTournamentData> Featured;
	for (const FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.bIsFeatured &&
			Tournament.State != EMGTournamentState::Completed &&
			Tournament.State != EMGTournamentState::Cancelled)
		{
			Featured.Add(Tournament);
		}
	}
	return Featured;
}

bool UMGTournamentSubsystem::GetTournament(const FString& TournamentID, FMGTournamentData& OutTournament) const
{
	for (const FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			OutTournament = Tournament;
			return true;
		}
	}
	return false;
}

TArray<FMGTournamentData> UMGTournamentSubsystem::GetRegisteredTournaments() const
{
	TArray<FMGTournamentData> Registered;
	for (const FString& ID : RegisteredTournamentIDs)
	{
		FMGTournamentData Tournament;
		if (GetTournament(ID, Tournament))
		{
			Registered.Add(Tournament);
		}
	}
	return Registered;
}

// ==========================================
// REGISTRATION
// ==========================================

bool UMGTournamentSubsystem::CanRegisterForTournament(const FString& TournamentID, FText& OutReason) const
{
	FMGTournamentData Tournament;
	if (!GetTournament(TournamentID, Tournament))
	{
		OutReason = FText::FromString(TEXT("Tournament not found"));
		return false;
	}

	// Check state
	if (Tournament.State != EMGTournamentState::Registration)
	{
		OutReason = FText::FromString(TEXT("Registration is not open"));
		return false;
	}

	// Check if already registered
	if (IsRegisteredForTournament(TournamentID))
	{
		OutReason = FText::FromString(TEXT("Already registered"));
		return false;
	}

	// Check capacity
	if (Tournament.Participants.Num() >= Tournament.MaxParticipants)
	{
		OutReason = FText::FromString(TEXT("Tournament is full"));
		return false;
	}

	// Check requirements
	const FMGTournamentRequirements& Req = Tournament.Requirements;

	// Would check player level, rank, etc. here
	// For now, allow registration

	if (Req.bInviteOnly)
	{
		OutReason = FText::FromString(TEXT("This tournament is invite only"));
		return false;
	}

	return true;
}

bool UMGTournamentSubsystem::RegisterForTournament(const FString& TournamentID, FName VehicleID)
{
	FText Reason;
	if (!CanRegisterForTournament(TournamentID, Reason))
	{
		OnTournamentRegistration.Broadcast(TournamentID, false);
		return false;
	}

	// Find tournament
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			// Create participant
			FMGTournamentParticipant Participant;
			Participant.ParticipantID = LocalPlayerID;
			Participant.DisplayName = FText::FromString(TEXT("Player")); // Would get from profile
			Participant.VehicleID = VehicleID;
			Participant.Seed = Tournament.Participants.Num() + 1;

			Tournament.Participants.Add(Participant);
			RegisteredTournamentIDs.Add(TournamentID);

			OnTournamentRegistration.Broadcast(TournamentID, true);
			return true;
		}
	}

	OnTournamentRegistration.Broadcast(TournamentID, false);
	return false;
}

bool UMGTournamentSubsystem::RegisterTeamForTournament(const FString& TournamentID, const TArray<FString>& TeamMemberIDs, const FText& TeamName)
{
	FText Reason;
	if (!CanRegisterForTournament(TournamentID, Reason))
	{
		OnTournamentRegistration.Broadcast(TournamentID, false);
		return false;
	}

	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			if (Tournament.EntryType == EMGTournamentEntryType::Solo)
			{
				OnTournamentRegistration.Broadcast(TournamentID, false);
				return false;
			}

			FMGTournamentParticipant Participant;
			Participant.ParticipantID = FGuid::NewGuid().ToString();
			Participant.DisplayName = TeamName;
			Participant.TeamName = TeamName;
			Participant.MemberIDs = TeamMemberIDs;
			Participant.Seed = Tournament.Participants.Num() + 1;

			Tournament.Participants.Add(Participant);
			RegisteredTournamentIDs.Add(TournamentID);

			OnTournamentRegistration.Broadcast(TournamentID, true);
			return true;
		}
	}

	OnTournamentRegistration.Broadcast(TournamentID, false);
	return false;
}

bool UMGTournamentSubsystem::UnregisterFromTournament(const FString& TournamentID)
{
	FMGTournamentData Tournament;
	if (!GetTournament(TournamentID, Tournament))
	{
		return false;
	}

	// Can only unregister during registration
	if (Tournament.State != EMGTournamentState::Registration)
	{
		return false;
	}

	// Remove from participants
	for (FMGTournamentData& T : Tournaments)
	{
		if (T.TournamentID == TournamentID)
		{
			T.Participants.RemoveAll([this](const FMGTournamentParticipant& P) {
				return P.ParticipantID == LocalPlayerID || P.MemberIDs.Contains(LocalPlayerID);
			});
			break;
		}
	}

	RegisteredTournamentIDs.Remove(TournamentID);
	return true;
}

bool UMGTournamentSubsystem::IsRegisteredForTournament(const FString& TournamentID) const
{
	return RegisteredTournamentIDs.Contains(TournamentID);
}

int32 UMGTournamentSubsystem::GetRegisteredCount(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		return Tournament.Participants.Num();
	}
	return 0;
}

// ==========================================
// CHECK-IN
// ==========================================

bool UMGTournamentSubsystem::CheckInForTournament(const FString& TournamentID)
{
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			if (Tournament.State != EMGTournamentState::CheckIn)
			{
				return false;
			}

			for (FMGTournamentParticipant& Participant : Tournament.Participants)
			{
				if (Participant.ParticipantID == LocalPlayerID ||
					Participant.MemberIDs.Contains(LocalPlayerID))
				{
					Participant.bCheckedIn = true;
					return true;
				}
			}
		}
	}
	return false;
}

bool UMGTournamentSubsystem::IsCheckedIn(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentParticipant& Participant : Tournament.Participants)
		{
			if (Participant.ParticipantID == LocalPlayerID ||
				Participant.MemberIDs.Contains(LocalPlayerID))
			{
				return Participant.bCheckedIn;
			}
		}
	}
	return false;
}

FTimespan UMGTournamentSubsystem::GetTimeUntilCheckIn(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		return Tournament.Schedule.CheckInStart - FDateTime::Now();
	}
	return FTimespan::Zero();
}

FTimespan UMGTournamentSubsystem::GetTimeUntilCheckInCloses(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		return Tournament.Schedule.CheckInEnd - FDateTime::Now();
	}
	return FTimespan::Zero();
}

// ==========================================
// MATCH MANAGEMENT
// ==========================================

bool UMGTournamentSubsystem::GetCurrentMatch(FMGTournamentMatch& OutMatch) const
{
	if (ActiveMatchTournamentID.IsEmpty() || ActiveMatchID.IsEmpty())
	{
		// Find next pending match
		for (const FMGTournamentData& Tournament : Tournaments)
		{
			if (Tournament.State == EMGTournamentState::InProgress)
			{
				for (const FMGTournamentMatch& Match : Tournament.Matches)
				{
					if ((Match.Participant1ID == LocalPlayerID || Match.Participant2ID == LocalPlayerID) &&
						(Match.State == EMGMatchState::Pending || Match.State == EMGMatchState::ReadyToStart))
					{
						OutMatch = Match;
						return true;
					}
				}
			}
		}
		return false;
	}

	return GetMatch(ActiveMatchTournamentID, ActiveMatchID, OutMatch);
}

bool UMGTournamentSubsystem::GetMatch(const FString& TournamentID, const FString& MatchID, FMGTournamentMatch& OutMatch) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentMatch& Match : Tournament.Matches)
		{
			if (Match.MatchID == MatchID)
			{
				OutMatch = Match;
				return true;
			}
		}
	}
	return false;
}

TArray<FMGTournamentMatch> UMGTournamentSubsystem::GetMatchesForRound(const FString& TournamentID, int32 Round, EMGBracketSide Side) const
{
	TArray<FMGTournamentMatch> Result;

	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentMatch& Match : Tournament.Matches)
		{
			if (Match.Round == Round && Match.BracketSide == Side)
			{
				Result.Add(Match);
			}
		}
	}

	// Sort by match number
	Result.Sort([](const FMGTournamentMatch& A, const FMGTournamentMatch& B) {
		return A.MatchNumber < B.MatchNumber;
	});

	return Result;
}

void UMGTournamentSubsystem::ReportMatchReady(const FString& TournamentID, const FString& MatchID)
{
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			int32 Index = GetMatchIndex(Tournament, MatchID);
			if (Index != INDEX_NONE)
			{
				Tournament.Matches[Index].State = EMGMatchState::ReadyToStart;
				OnMatchReady.Broadcast(Tournament.Matches[Index]);
			}
			break;
		}
	}
}

void UMGTournamentSubsystem::ReportMatchResult(const FString& TournamentID, const FString& MatchID, const FString& WinnerID, int32 Score1, int32 Score2)
{
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			int32 Index = GetMatchIndex(Tournament, MatchID);
			if (Index != INDEX_NONE)
			{
				FMGTournamentMatch& Match = Tournament.Matches[Index];
				Match.WinnerID = WinnerID;
				Match.Score1 = Score1;
				Match.Score2 = Score2;
				Match.State = EMGMatchState::Completed;
				Match.EndTime = FDateTime::Now();
				Match.LoserID = (Match.Participant1ID == WinnerID) ? Match.Participant2ID : Match.Participant1ID;

				// Update participant stats
				int32 WinnerIndex = GetParticipantIndex(Tournament, WinnerID);
				int32 LoserIndex = GetParticipantIndex(Tournament, Match.LoserID);

				if (WinnerIndex != INDEX_NONE)
				{
					Tournament.Participants[WinnerIndex].Wins++;
				}
				if (LoserIndex != INDEX_NONE)
				{
					Tournament.Participants[LoserIndex].Losses++;
				}

				// Advance winner
				AdvanceWinner(Tournament, Match);

				// Handle loser based on format
				if (Tournament.Format == EMGTournamentFormat::DoubleElimination && Match.BracketSide == EMGBracketSide::Winners)
				{
					MoveToLosersBracket(Tournament, Match);
				}
				else if (Tournament.Format == EMGTournamentFormat::SingleElimination ||
						 Match.BracketSide == EMGBracketSide::Losers)
				{
					// Mark loser as eliminated
					if (LoserIndex != INDEX_NONE)
					{
						Tournament.Participants[LoserIndex].bEliminated = true;
					}
				}

				OnMatchCompleted.Broadcast(Match);

				// Check if round is complete
				if (IsRoundComplete(Tournament, Match.Round, Match.BracketSide))
				{
					AdvanceToNextRound(Tournament);
				}
			}
			break;
		}
	}
}

void UMGTournamentSubsystem::ForfeitMatch(const FString& TournamentID, const FString& MatchID)
{
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			int32 Index = GetMatchIndex(Tournament, MatchID);
			if (Index != INDEX_NONE)
			{
				FMGTournamentMatch& Match = Tournament.Matches[Index];

				// Determine who forfeited (assume local player)
				FString ForfeitingID = LocalPlayerID;
				FString WinnerID = (Match.Participant1ID == ForfeitingID) ? Match.Participant2ID : Match.Participant1ID;

				Match.State = EMGMatchState::Forfeited;
				Match.WinnerID = WinnerID;
				Match.LoserID = ForfeitingID;
				Match.EndTime = FDateTime::Now();

				// Mark forfeiter as eliminated
				int32 LoserIndex = GetParticipantIndex(Tournament, ForfeitingID);
				if (LoserIndex != INDEX_NONE)
				{
					Tournament.Participants[LoserIndex].bEliminated = true;
					Tournament.Participants[LoserIndex].Losses++;
				}

				int32 WinnerIndex = GetParticipantIndex(Tournament, WinnerID);
				if (WinnerIndex != INDEX_NONE)
				{
					Tournament.Participants[WinnerIndex].Wins++;
				}

				AdvanceWinner(Tournament, Match);
				OnMatchCompleted.Broadcast(Match);
			}
			break;
		}
	}
}

// ==========================================
// BRACKET QUERIES
// ==========================================

TArray<FMGBracketRound> UMGTournamentSubsystem::GetBracket(const FString& TournamentID, EMGBracketSide Side) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		if (Side == EMGBracketSide::Winners)
		{
			return Tournament.WinnersBracket;
		}
		else if (Side == EMGBracketSide::Losers)
		{
			return Tournament.LosersBracket;
		}
	}
	return TArray<FMGBracketRound>();
}

bool UMGTournamentSubsystem::GetParticipant(const FString& TournamentID, const FString& ParticipantID, FMGTournamentParticipant& OutParticipant) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentParticipant& Participant : Tournament.Participants)
		{
			if (Participant.ParticipantID == ParticipantID)
			{
				OutParticipant = Participant;
				return true;
			}
		}
	}
	return false;
}

TArray<FMGTournamentParticipant> UMGTournamentSubsystem::GetStandings(const FString& TournamentID) const
{
	TArray<FMGTournamentParticipant> Standings;

	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		Standings = Tournament.Participants;

		// Sort by placement, then wins, then points
		Standings.Sort([](const FMGTournamentParticipant& A, const FMGTournamentParticipant& B) {
			if (A.FinalPlacement > 0 && B.FinalPlacement > 0)
			{
				return A.FinalPlacement < B.FinalPlacement;
			}
			if (A.FinalPlacement > 0) return true;
			if (B.FinalPlacement > 0) return false;
			if (A.Wins != B.Wins) return A.Wins > B.Wins;
			return A.Points > B.Points;
		});
	}

	return Standings;
}

TArray<FMGTournamentParticipant> UMGTournamentSubsystem::GetGroupStandings(const FString& TournamentID, const FString& GroupID) const
{
	TArray<FMGTournamentParticipant> Standings;

	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentGroup& Group : Tournament.Groups)
		{
			if (Group.GroupID == GroupID)
			{
				for (const FString& ParticipantID : Group.ParticipantIDs)
				{
					FMGTournamentParticipant Participant;
					if (GetParticipant(TournamentID, ParticipantID, Participant))
					{
						Standings.Add(Participant);
					}
				}
				break;
			}
		}

		// Sort by points, then win-loss
		Standings.Sort([](const FMGTournamentParticipant& A, const FMGTournamentParticipant& B) {
			if (A.Points != B.Points) return A.Points > B.Points;
			return (A.Wins - A.Losses) > (B.Wins - B.Losses);
		});
	}

	return Standings;
}

bool UMGTournamentSubsystem::IsPlayerEliminated(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentParticipant& Participant : Tournament.Participants)
		{
			if (Participant.ParticipantID == LocalPlayerID ||
				Participant.MemberIDs.Contains(LocalPlayerID))
			{
				return Participant.bEliminated;
			}
		}
	}
	return true;
}

int32 UMGTournamentSubsystem::GetPlayerPlacement(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentParticipant& Participant : Tournament.Participants)
		{
			if (Participant.ParticipantID == LocalPlayerID ||
				Participant.MemberIDs.Contains(LocalPlayerID))
			{
				return Participant.FinalPlacement;
			}
		}
	}
	return 0;
}

// ==========================================
// TOURNAMENT CREATION
// ==========================================

FString UMGTournamentSubsystem::CreateTournament(const FMGTournamentData& TournamentData)
{
	FMGTournamentData NewTournament = TournamentData;
	NewTournament.TournamentID = FGuid::NewGuid().ToString();
	NewTournament.CreatedAt = FDateTime::Now();
	NewTournament.OrganizerID = LocalPlayerID;
	NewTournament.State = EMGTournamentState::Announced;

	Tournaments.Add(NewTournament);
	return NewTournament.TournamentID;
}

bool UMGTournamentSubsystem::CancelTournament(const FString& TournamentID)
{
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID && Tournament.OrganizerID == LocalPlayerID)
		{
			Tournament.State = EMGTournamentState::Cancelled;
			OnTournamentStateChanged.Broadcast(Tournament);
			return true;
		}
	}
	return false;
}

bool UMGTournamentSubsystem::StartTournament(const FString& TournamentID)
{
	for (FMGTournamentData& Tournament : Tournaments)
	{
		if (Tournament.TournamentID == TournamentID)
		{
			if (Tournament.Participants.Num() < Tournament.MinParticipants)
			{
				return false;
			}

			// Remove non-checked-in participants
			Tournament.Participants.RemoveAll([](const FMGTournamentParticipant& P) {
				return !P.bCheckedIn;
			});

			// Seed participants
			SeedParticipants(Tournament);

			// Generate bracket
			GenerateBracket(Tournament);

			Tournament.State = EMGTournamentState::InProgress;
			Tournament.CurrentRound = 1;

			OnTournamentStateChanged.Broadcast(Tournament);
			return true;
		}
	}
	return false;
}

// ==========================================
// STATS
// ==========================================

TArray<FMGTournamentData> UMGTournamentSubsystem::GetTournamentHistory(int32 Count) const
{
	TArray<FMGTournamentData> History;

	for (int32 i = TournamentHistory.Num() - 1; i >= 0 && History.Num() < Count; i--)
	{
		History.Add(TournamentHistory[i]);
	}

	return History;
}

// ==========================================
// UTILITY
// ==========================================

FText UMGTournamentSubsystem::GetRoundName(int32 TotalRounds, int32 CurrentRound, EMGBracketSide Side)
{
	int32 RoundsRemaining = TotalRounds - CurrentRound + 1;

	if (Side == EMGBracketSide::GrandFinals)
	{
		return FText::FromString(TEXT("Grand Finals"));
	}

	FString Prefix = (Side == EMGBracketSide::Losers) ? TEXT("Losers ") : TEXT("");

	switch (RoundsRemaining)
	{
		case 1: return FText::FromString(Prefix + TEXT("Finals"));
		case 2: return FText::FromString(Prefix + TEXT("Semi-Finals"));
		case 3: return FText::FromString(Prefix + TEXT("Quarter-Finals"));
		default: return FText::Format(FText::FromString(Prefix + TEXT("Round {0}")), CurrentRound);
	}
}

FTimespan UMGTournamentSubsystem::GetTimeUntilStart(const FString& TournamentID) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		return Tournament.Schedule.TournamentStart - FDateTime::Now();
	}
	return FTimespan::Zero();
}

FMGTournamentPrize UMGTournamentSubsystem::GetPrizeForPlacement(const FString& TournamentID, int32 Placement) const
{
	FMGTournamentData Tournament;
	if (GetTournament(TournamentID, Tournament))
	{
		for (const FMGTournamentPrize& Prize : Tournament.Prizes)
		{
			if (Prize.Placement == Placement)
			{
				return Prize;
			}
		}
	}
	return FMGTournamentPrize();
}

// ==========================================
// INTERNAL
// ==========================================

void UMGTournamentSubsystem::GenerateBracket(FMGTournamentData& Tournament)
{
	switch (Tournament.Format)
	{
		case EMGTournamentFormat::SingleElimination:
			GenerateSingleEliminationBracket(Tournament);
			break;
		case EMGTournamentFormat::DoubleElimination:
			GenerateDoubleEliminationBracket(Tournament);
			break;
		case EMGTournamentFormat::RoundRobin:
			GenerateRoundRobinSchedule(Tournament);
			break;
		case EMGTournamentFormat::GroupStage:
			GenerateGroupStage(Tournament);
			break;
		default:
			GenerateSingleEliminationBracket(Tournament);
			break;
	}
}

void UMGTournamentSubsystem::GenerateSingleEliminationBracket(FMGTournamentData& Tournament)
{
	int32 ParticipantCount = Tournament.Participants.Num();

	// Calculate bracket size (next power of 2)
	int32 BracketSize = 1;
	while (BracketSize < ParticipantCount)
	{
		BracketSize *= 2;
	}

	int32 TotalRounds = FMath::CeilToInt(FMath::Log2(float(BracketSize)));
	int32 ByeCount = BracketSize - ParticipantCount;

	// Get seeded matchups
	TArray<TPair<int32, int32>> Matchups = GetSeededMatchups(BracketSize);

	Tournament.Matches.Empty();
	Tournament.WinnersBracket.Empty();

	// Generate first round
	FMGBracketRound FirstRound;
	FirstRound.RoundNumber = 1;
	FirstRound.RoundName = GetRoundName(TotalRounds, 1, EMGBracketSide::Winners);
	FirstRound.BracketSide = EMGBracketSide::Winners;
	FirstRound.BestOf = Tournament.RoundBestOf.Contains(1) ? Tournament.RoundBestOf[1] : 1;

	int32 MatchNum = 1;
	for (const TPair<int32, int32>& Matchup : Matchups)
	{
		FMGTournamentMatch Match;
		Match.MatchID = FString::Printf(TEXT("M%d_R1_%d"), MatchNum, MatchNum);
		Match.Round = 1;
		Match.MatchNumber = MatchNum;
		Match.BracketSide = EMGBracketSide::Winners;
		Match.BestOf = FirstRound.BestOf;

		// Assign participants (seeds are 1-indexed)
		if (Matchup.Key <= ParticipantCount)
		{
			Match.Participant1ID = Tournament.Participants[Matchup.Key - 1].ParticipantID;
		}
		if (Matchup.Value <= ParticipantCount)
		{
			Match.Participant2ID = Tournament.Participants[Matchup.Value - 1].ParticipantID;
		}

		// Handle byes
		if (Match.Participant1ID.IsEmpty() || Match.Participant2ID.IsEmpty())
		{
			Match.State = EMGMatchState::Bye;
			Match.WinnerID = Match.Participant1ID.IsEmpty() ? Match.Participant2ID : Match.Participant1ID;
		}

		// Set track from pool
		if (Tournament.TrackPool.Num() > 0)
		{
			Match.TrackID = Tournament.TrackPool[MatchNum % Tournament.TrackPool.Num()];
		}

		Tournament.Matches.Add(Match);
		FirstRound.MatchIDs.Add(Match.MatchID);
		MatchNum++;
	}

	Tournament.WinnersBracket.Add(FirstRound);

	// Generate subsequent rounds
	int32 MatchesInRound = Matchups.Num() / 2;
	for (int32 Round = 2; Round <= TotalRounds; Round++)
	{
		FMGBracketRound NextRound;
		NextRound.RoundNumber = Round;
		NextRound.RoundName = GetRoundName(TotalRounds, Round, EMGBracketSide::Winners);
		NextRound.BracketSide = EMGBracketSide::Winners;
		NextRound.BestOf = Tournament.RoundBestOf.Contains(Round) ? Tournament.RoundBestOf[Round] : 1;

		for (int32 i = 0; i < MatchesInRound; i++)
		{
			FMGTournamentMatch Match;
			Match.MatchID = FString::Printf(TEXT("M%d_R%d_%d"), MatchNum, Round, i + 1);
			Match.Round = Round;
			Match.MatchNumber = i + 1;
			Match.BracketSide = EMGBracketSide::Winners;
			Match.BestOf = NextRound.BestOf;

			if (Tournament.TrackPool.Num() > 0)
			{
				Match.TrackID = Tournament.TrackPool[MatchNum % Tournament.TrackPool.Num()];
			}

			Tournament.Matches.Add(Match);
			NextRound.MatchIDs.Add(Match.MatchID);
			MatchNum++;
		}

		Tournament.WinnersBracket.Add(NextRound);
		MatchesInRound /= 2;
	}

	// Link matches (set NextMatchWinnerID)
	int32 PreviousRoundStart = 0;
	int32 CurrentRoundStart = 0;

	for (int32 Round = 0; Round < TotalRounds - 1; Round++)
	{
		int32 MatchesInThisRound = Tournament.WinnersBracket[Round].MatchIDs.Num();
		int32 NextRoundStart = CurrentRoundStart + MatchesInThisRound;

		for (int32 i = 0; i < MatchesInThisRound; i += 2)
		{
			int32 NextMatchIndex = NextRoundStart + (i / 2);
			if (NextMatchIndex < Tournament.Matches.Num())
			{
				Tournament.Matches[CurrentRoundStart + i].NextMatchWinnerID = Tournament.Matches[NextMatchIndex].MatchID;
				Tournament.Matches[CurrentRoundStart + i + 1].NextMatchWinnerID = Tournament.Matches[NextMatchIndex].MatchID;
			}
		}

		CurrentRoundStart = NextRoundStart;
	}

	// Advance any byes
	for (FMGTournamentMatch& Match : Tournament.Matches)
	{
		if (Match.State == EMGMatchState::Bye && !Match.NextMatchWinnerID.IsEmpty())
		{
			int32 NextIndex = GetMatchIndex(Tournament, Match.NextMatchWinnerID);
			if (NextIndex != INDEX_NONE)
			{
				if (Tournament.Matches[NextIndex].Participant1ID.IsEmpty())
				{
					Tournament.Matches[NextIndex].Participant1ID = Match.WinnerID;
				}
				else
				{
					Tournament.Matches[NextIndex].Participant2ID = Match.WinnerID;
				}
			}
		}
	}
}

void UMGTournamentSubsystem::GenerateDoubleEliminationBracket(FMGTournamentData& Tournament)
{
	// Generate winners bracket first
	GenerateSingleEliminationBracket(Tournament);

	// Generate losers bracket
	int32 WinnersRounds = Tournament.WinnersBracket.Num();
	int32 LosersRounds = (WinnersRounds - 1) * 2;

	int32 MatchNum = Tournament.Matches.Num() + 1;
	int32 MatchesInRound = Tournament.WinnersBracket[0].MatchIDs.Num() / 2;

	for (int32 Round = 1; Round <= LosersRounds; Round++)
	{
		FMGBracketRound LosersRound;
		LosersRound.RoundNumber = Round;
		LosersRound.RoundName = GetRoundName(LosersRounds, Round, EMGBracketSide::Losers);
		LosersRound.BracketSide = EMGBracketSide::Losers;
		LosersRound.BestOf = Tournament.RoundBestOf.Contains(Round) ? Tournament.RoundBestOf[Round] : 1;

		for (int32 i = 0; i < MatchesInRound; i++)
		{
			FMGTournamentMatch Match;
			Match.MatchID = FString::Printf(TEXT("L%d_R%d_%d"), MatchNum, Round, i + 1);
			Match.Round = Round;
			Match.MatchNumber = i + 1;
			Match.BracketSide = EMGBracketSide::Losers;
			Match.BestOf = LosersRound.BestOf;

			if (Tournament.TrackPool.Num() > 0)
			{
				Match.TrackID = Tournament.TrackPool[MatchNum % Tournament.TrackPool.Num()];
			}

			Tournament.Matches.Add(Match);
			LosersRound.MatchIDs.Add(Match.MatchID);
			MatchNum++;
		}

		Tournament.LosersBracket.Add(LosersRound);

		// Losers bracket halves every 2 rounds
		if (Round % 2 == 0)
		{
			MatchesInRound /= 2;
		}
	}

	// Grand Finals
	FMGTournamentMatch GrandFinals;
	GrandFinals.MatchID = FString::Printf(TEXT("GF_%d"), MatchNum);
	GrandFinals.Round = 1;
	GrandFinals.MatchNumber = 1;
	GrandFinals.BracketSide = EMGBracketSide::GrandFinals;
	GrandFinals.BestOf = Tournament.RoundBestOf.Contains(0) ? Tournament.RoundBestOf[0] : 3; // Finals usually best of 3
	Tournament.Matches.Add(GrandFinals);
}

void UMGTournamentSubsystem::GenerateRoundRobinSchedule(FMGTournamentData& Tournament)
{
	int32 ParticipantCount = Tournament.Participants.Num();
	int32 Rounds = ParticipantCount - 1;
	int32 MatchesPerRound = ParticipantCount / 2;

	Tournament.Matches.Empty();
	Tournament.WinnersBracket.Empty();

	int32 MatchNum = 1;

	// Circle method for round robin scheduling
	TArray<int32> Indices;
	for (int32 i = 0; i < ParticipantCount; i++)
	{
		Indices.Add(i);
	}

	for (int32 Round = 1; Round <= Rounds; Round++)
	{
		FMGBracketRound RoundData;
		RoundData.RoundNumber = Round;
		RoundData.RoundName = FText::Format(FText::FromString(TEXT("Round {0}")), Round);
		RoundData.BestOf = 1;

		for (int32 i = 0; i < MatchesPerRound; i++)
		{
			int32 P1 = Indices[i];
			int32 P2 = Indices[ParticipantCount - 1 - i];

			FMGTournamentMatch Match;
			Match.MatchID = FString::Printf(TEXT("RR_%d_R%d_%d"), MatchNum, Round, i + 1);
			Match.Round = Round;
			Match.MatchNumber = i + 1;
			Match.Participant1ID = Tournament.Participants[P1].ParticipantID;
			Match.Participant2ID = Tournament.Participants[P2].ParticipantID;

			if (Tournament.TrackPool.Num() > 0)
			{
				Match.TrackID = Tournament.TrackPool[MatchNum % Tournament.TrackPool.Num()];
			}

			Tournament.Matches.Add(Match);
			RoundData.MatchIDs.Add(Match.MatchID);
			MatchNum++;
		}

		Tournament.WinnersBracket.Add(RoundData);

		// Rotate indices (keep first element fixed)
		int32 Last = Indices.Last();
		for (int32 i = ParticipantCount - 1; i > 1; i--)
		{
			Indices[i] = Indices[i - 1];
		}
		Indices[1] = Last;
	}
}

void UMGTournamentSubsystem::GenerateGroupStage(FMGTournamentData& Tournament)
{
	int32 ParticipantCount = Tournament.Participants.Num();
	int32 GroupCount = FMath::Max(2, ParticipantCount / 4);
	int32 PlayersPerGroup = ParticipantCount / GroupCount;

	Tournament.Groups.Empty();
	Tournament.Matches.Empty();

	// Create groups
	for (int32 g = 0; g < GroupCount; g++)
	{
		FMGTournamentGroup Group;
		Group.GroupID = FString::Printf(TEXT("Group_%c"), 'A' + g);
		Group.GroupName = FText::FromString(FString::Printf(TEXT("Group %c"), 'A' + g));
		Group.AdvancingCount = 2;

		// Assign participants using snake seeding
		for (int32 i = 0; i < PlayersPerGroup; i++)
		{
			int32 ParticipantIndex;
			if (i % 2 == 0)
			{
				ParticipantIndex = (i * GroupCount) + g;
			}
			else
			{
				ParticipantIndex = ((i + 1) * GroupCount) - 1 - g;
			}

			if (ParticipantIndex < ParticipantCount)
			{
				Group.ParticipantIDs.Add(Tournament.Participants[ParticipantIndex].ParticipantID);
			}
		}

		Tournament.Groups.Add(Group);
	}

	// Generate round robin matches within each group
	int32 MatchNum = 1;
	for (FMGTournamentGroup& Group : Tournament.Groups)
	{
		int32 GroupSize = Group.ParticipantIDs.Num();

		for (int32 i = 0; i < GroupSize; i++)
		{
			for (int32 j = i + 1; j < GroupSize; j++)
			{
				FMGTournamentMatch Match;
				Match.MatchID = FString::Printf(TEXT("GS_%d_%s"), MatchNum, *Group.GroupID);
				Match.Round = 1;
				Match.MatchNumber = MatchNum;
				Match.GroupID = Group.GroupID;
				Match.Participant1ID = Group.ParticipantIDs[i];
				Match.Participant2ID = Group.ParticipantIDs[j];

				if (Tournament.TrackPool.Num() > 0)
				{
					Match.TrackID = Tournament.TrackPool[MatchNum % Tournament.TrackPool.Num()];
				}

				Tournament.Matches.Add(Match);
				Group.MatchIDs.Add(Match.MatchID);
				MatchNum++;
			}
		}
	}
}

void UMGTournamentSubsystem::AdvanceWinner(FMGTournamentData& Tournament, FMGTournamentMatch& CompletedMatch)
{
	if (CompletedMatch.NextMatchWinnerID.IsEmpty())
	{
		return;
	}

	int32 NextMatchIndex = GetMatchIndex(Tournament, CompletedMatch.NextMatchWinnerID);
	if (NextMatchIndex != INDEX_NONE)
	{
		FMGTournamentMatch& NextMatch = Tournament.Matches[NextMatchIndex];

		if (NextMatch.Participant1ID.IsEmpty())
		{
			NextMatch.Participant1ID = CompletedMatch.WinnerID;
		}
		else
		{
			NextMatch.Participant2ID = CompletedMatch.WinnerID;
		}

		// Check if next match is ready
		if (!NextMatch.Participant1ID.IsEmpty() && !NextMatch.Participant2ID.IsEmpty())
		{
			NextMatch.State = EMGMatchState::Pending;
		}
	}
}

void UMGTournamentSubsystem::MoveToLosersBracket(FMGTournamentData& Tournament, FMGTournamentMatch& CompletedMatch)
{
	if (CompletedMatch.NextMatchLoserID.IsEmpty())
	{
		return;
	}

	int32 LosersMatchIndex = GetMatchIndex(Tournament, CompletedMatch.NextMatchLoserID);
	if (LosersMatchIndex != INDEX_NONE)
	{
		FMGTournamentMatch& LosersMatch = Tournament.Matches[LosersMatchIndex];

		if (LosersMatch.Participant1ID.IsEmpty())
		{
			LosersMatch.Participant1ID = CompletedMatch.LoserID;
		}
		else
		{
			LosersMatch.Participant2ID = CompletedMatch.LoserID;
		}
	}
}

bool UMGTournamentSubsystem::IsRoundComplete(const FMGTournamentData& Tournament, int32 Round, EMGBracketSide Side) const
{
	for (const FMGTournamentMatch& Match : Tournament.Matches)
	{
		if (Match.Round == Round && Match.BracketSide == Side)
		{
			if (Match.State != EMGMatchState::Completed &&
				Match.State != EMGMatchState::Bye &&
				Match.State != EMGMatchState::Forfeited)
			{
				return false;
			}
		}
	}
	return true;
}

void UMGTournamentSubsystem::AdvanceToNextRound(FMGTournamentData& Tournament)
{
	Tournament.CurrentRound++;

	// Check if tournament is complete
	bool bAllMatchesComplete = true;
	for (const FMGTournamentMatch& Match : Tournament.Matches)
	{
		if (Match.State != EMGMatchState::Completed &&
			Match.State != EMGMatchState::Bye &&
			Match.State != EMGMatchState::Forfeited)
		{
			bAllMatchesComplete = false;
			break;
		}
	}

	if (bAllMatchesComplete)
	{
		CompleteTournament(Tournament);
	}
}

void UMGTournamentSubsystem::CompleteTournament(FMGTournamentData& Tournament)
{
	Tournament.State = EMGTournamentState::Completed;

	CalculateFinalPlacements(Tournament);
	DistributePrizes(Tournament);
	UpdatePlayerStats(Tournament);

	// Add to history
	TournamentHistory.Add(Tournament);

	// Get final standings
	TArray<FMGTournamentParticipant> FinalStandings = GetStandings(Tournament.TournamentID);

	OnTournamentCompleted.Broadcast(Tournament.TournamentID, FinalStandings);
	OnTournamentStateChanged.Broadcast(Tournament);
}

void UMGTournamentSubsystem::CalculateFinalPlacements(FMGTournamentData& Tournament)
{
	// Sort participants by wins (descending), then losses (ascending)
	Tournament.Participants.Sort([](const FMGTournamentParticipant& A, const FMGTournamentParticipant& B) {
		if (A.Wins != B.Wins) return A.Wins > B.Wins;
		return A.Losses < B.Losses;
	});

	// Assign placements
	for (int32 i = 0; i < Tournament.Participants.Num(); i++)
	{
		Tournament.Participants[i].FinalPlacement = i + 1;
	}
}

void UMGTournamentSubsystem::DistributePrizes(const FMGTournamentData& Tournament)
{
	for (const FMGTournamentParticipant& Participant : Tournament.Participants)
	{
		FMGTournamentPrize Prize = GetPrizeForPlacement(Tournament.TournamentID, Participant.FinalPlacement);

		// Would apply prizes to player through economy system
		// For now, just log
		if (Prize.CashReward > 0 || Prize.XPReward > 0)
		{
			// Apply rewards
		}
	}
}

void UMGTournamentSubsystem::SeedParticipants(FMGTournamentData& Tournament)
{
	// Sort by existing seed or rating
	Tournament.Participants.Sort([](const FMGTournamentParticipant& A, const FMGTournamentParticipant& B) {
		return A.Seed < B.Seed;
	});

	// Reassign seeds
	for (int32 i = 0; i < Tournament.Participants.Num(); i++)
	{
		Tournament.Participants[i].Seed = i + 1;
	}
}

TArray<TPair<int32, int32>> UMGTournamentSubsystem::GetSeededMatchups(int32 ParticipantCount) const
{
	TArray<TPair<int32, int32>> Matchups;

	// Standard seeding: 1v16, 8v9, 5v12, 4v13, etc.
	// This ensures top seeds meet in later rounds

	if (ParticipantCount <= 2)
	{
		Matchups.Add(TPair<int32, int32>(1, 2));
		return Matchups;
	}

	// Recursive seeding algorithm
	TArray<int32> Seeds;
	Seeds.Add(1);
	Seeds.Add(2);

	int32 CurrentSize = 2;
	while (CurrentSize < ParticipantCount)
	{
		TArray<int32> NewSeeds;
		for (int32 i = 0; i < CurrentSize; i++)
		{
			NewSeeds.Add(Seeds[i]);
			NewSeeds.Add(CurrentSize * 2 + 1 - Seeds[i]);
		}
		Seeds = NewSeeds;
		CurrentSize *= 2;
	}

	// Create matchups from seeded order
	for (int32 i = 0; i < Seeds.Num(); i += 2)
	{
		Matchups.Add(TPair<int32, int32>(Seeds[i], Seeds[i + 1]));
	}

	return Matchups;
}

void UMGTournamentSubsystem::UpdatePlayerStats(const FMGTournamentData& Tournament)
{
	// Find local player's results
	for (const FMGTournamentParticipant& Participant : Tournament.Participants)
	{
		if (Participant.ParticipantID == LocalPlayerID ||
			Participant.MemberIDs.Contains(LocalPlayerID))
		{
			PlayerStats.TournamentsEntered++;
			PlayerStats.TotalMatchWins += Participant.Wins;
			PlayerStats.TotalMatchLosses += Participant.Losses;

			if (Participant.FinalPlacement == 1)
			{
				PlayerStats.TournamentsWon++;
			}
			if (Participant.FinalPlacement <= 3)
			{
				PlayerStats.TopThreeFinishes++;
			}

			// Update best placement by tier
			int32 CurrentBest = PlayerStats.BestPlacementByTier.FindOrAdd(Tournament.Tier);
			if (CurrentBest == 0 || Participant.FinalPlacement < CurrentBest)
			{
				PlayerStats.BestPlacementByTier[Tournament.Tier] = Participant.FinalPlacement;
			}

			// Calculate earnings
			FMGTournamentPrize Prize = GetPrizeForPlacement(Tournament.TournamentID, Participant.FinalPlacement);
			PlayerStats.TotalEarnings += Prize.CashReward;
			PlayerStats.ChampionshipPoints += Prize.ChampionshipPoints;

			break;
		}
	}
}

void UMGTournamentSubsystem::LoadMockTournaments()
{
	// Weekend Tournament
	{
		FMGTournamentData Tournament;
		Tournament.TournamentID = TEXT("TOURN_WEEKEND_001");
		Tournament.TournamentName = FText::FromString(TEXT("Weekend Warrior Cup"));
		Tournament.Description = FText::FromString(TEXT("Weekly single elimination tournament for all skill levels."));
		Tournament.Tier = EMGTournamentTier::Weekly;
		Tournament.Format = EMGTournamentFormat::SingleElimination;
		Tournament.EntryType = EMGTournamentEntryType::Solo;
		Tournament.State = EMGTournamentState::Registration;
		Tournament.MaxParticipants = 32;
		Tournament.MinParticipants = 8;
		Tournament.TotalPrizePool = 50000;
		Tournament.bIsFeatured = true;

		// Schedule
		FDateTime Now = FDateTime::Now();
		Tournament.Schedule.RegistrationStart = Now - FTimespan::FromHours(24);
		Tournament.Schedule.RegistrationEnd = Now + FTimespan::FromHours(2);
		Tournament.Schedule.CheckInStart = Now + FTimespan::FromHours(2);
		Tournament.Schedule.CheckInEnd = Now + FTimespan::FromHours(2.5);
		Tournament.Schedule.TournamentStart = Now + FTimespan::FromHours(3);

		// Prizes
		FMGTournamentPrize First;
		First.Placement = 1;
		First.CashReward = 25000;
		First.XPReward = 5000;
		First.ReputationReward = 500;
		First.ChampionshipPoints = 100;
		Tournament.Prizes.Add(First);

		FMGTournamentPrize Second;
		Second.Placement = 2;
		Second.CashReward = 15000;
		Second.XPReward = 3000;
		Second.ReputationReward = 300;
		Second.ChampionshipPoints = 75;
		Tournament.Prizes.Add(Second);

		FMGTournamentPrize Third;
		Third.Placement = 3;
		Third.CashReward = 10000;
		Third.XPReward = 2000;
		Third.ReputationReward = 200;
		Third.ChampionshipPoints = 50;
		Tournament.Prizes.Add(Third);

		// Tracks
		Tournament.TrackPool.Add(FName(TEXT("DowntownDrift")));
		Tournament.TrackPool.Add(FName(TEXT("IndustrialZone")));
		Tournament.TrackPool.Add(FName(TEXT("HarborRun")));

		// Best of settings
		Tournament.RoundBestOf.Add(1, 1); // Round 1: Best of 1
		Tournament.RoundBestOf.Add(2, 1); // Semi-finals: Best of 1
		Tournament.RoundBestOf.Add(3, 3); // Finals: Best of 3

		// Mock participants
		for (int32 i = 0; i < 12; i++)
		{
			FMGTournamentParticipant Participant;
			Participant.ParticipantID = FString::Printf(TEXT("Player_%03d"), i + 1);
			Participant.DisplayName = FText::FromString(FString::Printf(TEXT("Racer%d"), i + 1));
			Participant.Seed = i + 1;
			Participant.bCheckedIn = (i < 8);
			Tournament.Participants.Add(Participant);
		}

		Tournaments.Add(Tournament);
	}

	// Monthly Championship
	{
		FMGTournamentData Tournament;
		Tournament.TournamentID = TEXT("TOURN_MONTHLY_001");
		Tournament.TournamentName = FText::FromString(TEXT("Monthly Championship"));
		Tournament.Description = FText::FromString(TEXT("Premier monthly competition with double elimination bracket."));
		Tournament.Tier = EMGTournamentTier::Monthly;
		Tournament.Format = EMGTournamentFormat::DoubleElimination;
		Tournament.EntryType = EMGTournamentEntryType::Solo;
		Tournament.State = EMGTournamentState::Announced;
		Tournament.MaxParticipants = 64;
		Tournament.MinParticipants = 16;
		Tournament.TotalPrizePool = 200000;
		Tournament.bIsFeatured = true;
		Tournament.bHasStream = true;

		FDateTime Now = FDateTime::Now();
		Tournament.Schedule.RegistrationStart = Now + FTimespan::FromDays(2);
		Tournament.Schedule.RegistrationEnd = Now + FTimespan::FromDays(7);
		Tournament.Schedule.TournamentStart = Now + FTimespan::FromDays(8);

		// Requirements
		Tournament.Requirements.MinLevel = 20;
		Tournament.Requirements.MinRankTier = 2; // At least Bronze

		// Prizes
		FMGTournamentPrize First;
		First.Placement = 1;
		First.CashReward = 100000;
		First.XPReward = 20000;
		First.ReputationReward = 2000;
		First.ChampionshipPoints = 500;
		First.TitleReward = FName(TEXT("MonthlyChampion"));
		Tournament.Prizes.Add(First);

		Tournaments.Add(Tournament);
	}

	// Crew Battle
	{
		FMGTournamentData Tournament;
		Tournament.TournamentID = TEXT("TOURN_CREW_001");
		Tournament.TournamentName = FText::FromString(TEXT("Crew Showdown"));
		Tournament.Description = FText::FromString(TEXT("4v4 crew battles! Represent your crew in this team tournament."));
		Tournament.Tier = EMGTournamentTier::Weekly;
		Tournament.Format = EMGTournamentFormat::SingleElimination;
		Tournament.EntryType = EMGTournamentEntryType::Crew;
		Tournament.State = EMGTournamentState::Registration;
		Tournament.MaxParticipants = 16;
		Tournament.MinParticipants = 4;
		Tournament.TeamSize = 4;
		Tournament.TotalPrizePool = 100000;

		Tournament.Requirements.bRequiresCrew = true;

		FDateTime Now = FDateTime::Now();
		Tournament.Schedule.RegistrationStart = Now - FTimespan::FromHours(12);
		Tournament.Schedule.RegistrationEnd = Now + FTimespan::FromHours(12);
		Tournament.Schedule.TournamentStart = Now + FTimespan::FromHours(14);

		Tournaments.Add(Tournament);
	}
}

void UMGTournamentSubsystem::UpdateTournamentStates()
{
	FDateTime Now = FDateTime::Now();

	for (FMGTournamentData& Tournament : Tournaments)
	{
		EMGTournamentState PreviousState = Tournament.State;

		switch (Tournament.State)
		{
			case EMGTournamentState::Announced:
				if (Now >= Tournament.Schedule.RegistrationStart)
				{
					Tournament.State = EMGTournamentState::Registration;
				}
				break;

			case EMGTournamentState::Registration:
				if (Now >= Tournament.Schedule.CheckInStart)
				{
					Tournament.State = EMGTournamentState::CheckIn;
				}
				break;

			case EMGTournamentState::CheckIn:
				if (Now >= Tournament.Schedule.TournamentStart)
				{
					// Auto-start if enough participants
					if (Tournament.Participants.Num() >= Tournament.MinParticipants)
					{
						StartTournament(Tournament.TournamentID);
					}
					else
					{
						Tournament.State = EMGTournamentState::Cancelled;
					}
				}
				break;

			default:
				break;
		}

		if (Tournament.State != PreviousState)
		{
			OnTournamentStateChanged.Broadcast(Tournament);
		}
	}
}

int32 UMGTournamentSubsystem::GetParticipantIndex(const FMGTournamentData& Tournament, const FString& ParticipantID) const
{
	for (int32 i = 0; i < Tournament.Participants.Num(); i++)
	{
		if (Tournament.Participants[i].ParticipantID == ParticipantID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UMGTournamentSubsystem::GetMatchIndex(const FMGTournamentData& Tournament, const FString& MatchID) const
{
	for (int32 i = 0; i < Tournament.Matches.Num(); i++)
	{
		if (Tournament.Matches[i].MatchID == MatchID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

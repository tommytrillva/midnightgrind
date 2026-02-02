// Copyright Midnight Grind. All Rights Reserved.

#include "CrewBattles/MGCrewBattlesSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGCrewBattlesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadBattleData();

	// Start battle tick timer for state updates
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			BattleTickHandle,
			this,
			&UMGCrewBattlesSubsystem::OnBattleTick,
			1.0f,
			true
		);
	}
}

void UMGCrewBattlesSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BattleTickHandle);
	}

	SaveBattleData();

	Super::Deinitialize();
}

bool UMGCrewBattlesSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Matchmaking
// ============================================================================

bool UMGCrewBattlesSubsystem::StartMatchmaking(EMGCrewBattleType BattleType, EMGCrewBattleFormat Format)
{
	if (bIsMatchmaking || IsInBattle())
	{
		return false;
	}

	if (LocalCrewID.IsNone())
	{
		return false;
	}

	bIsMatchmaking = true;

	if (const UWorld* World = GetWorld())
	{
		MatchmakingStartTime = World->GetTimeSeconds();
	}

	// Initialize battle for matchmaking
	ActiveBattle = FMGCrewBattle();
	ActiveBattle.BattleID = FGuid::NewGuid();
	ActiveBattle.State = EMGCrewBattleState::Matchmaking;
	ActiveBattle.BattleType = BattleType;
	ActiveBattle.Format = Format;

	// Set up local crew as Crew1
	ActiveBattle.Crew1.CrewID = LocalCrewID;
	ActiveBattle.Crew1.CrewName = LocalCrewName;
	ActiveBattle.Crew1.CrewTag = LocalCrewTag;
	ActiveBattle.Crew1.CrewRating = LocalCrewRating;

	// Set required wins based on format
	switch (Format)
	{
	case EMGCrewBattleFormat::BestOf1:
		ActiveBattle.RequiredWins = 1;
		break;
	case EMGCrewBattleFormat::BestOf3:
		ActiveBattle.RequiredWins = 2;
		break;
	case EMGCrewBattleFormat::BestOf5:
		ActiveBattle.RequiredWins = 3;
		break;
	default:
		ActiveBattle.RequiredWins = 1;
		break;
	}

	return true;
}

void UMGCrewBattlesSubsystem::CancelMatchmaking()
{
	if (!bIsMatchmaking)
	{
		return;
	}

	bIsMatchmaking = false;
	MatchmakingStartTime = 0.0f;
	ActiveBattle = FMGCrewBattle();
}

float UMGCrewBattlesSubsystem::GetMatchmakingTime() const
{
	if (!bIsMatchmaking)
	{
		return 0.0f;
	}

	if (const UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds() - MatchmakingStartTime;
	}

	return 0.0f;
}

int32 UMGCrewBattlesSubsystem::GetEstimatedWaitTime() const
{
	if (!bIsMatchmaking)
	{
		return 0;
	}

	// Estimate based on rating bracket (higher ratings may have longer waits)
	int32 BaseWait = 30;

	if (LocalCrewRating > 2000)
	{
		BaseWait = 60;
	}
	else if (LocalCrewRating > 1800)
	{
		BaseWait = 45;
	}

	// Factor in current matchmaking time
	float CurrentWait = GetMatchmakingTime();
	int32 RemainingEstimate = FMath::Max(0, BaseWait - static_cast<int32>(CurrentWait));

	return RemainingEstimate;
}

// ============================================================================
// Challenges
// ============================================================================

FGuid UMGCrewBattlesSubsystem::SendChallenge(FName TargetCrewID, EMGCrewBattleType BattleType, EMGCrewBattleFormat Format, FDateTime ProposedTime, int32 WagerAmount)
{
	if (LocalCrewID.IsNone() || TargetCrewID.IsNone() || TargetCrewID == LocalCrewID)
	{
		return FGuid();
	}

	FMGCrewBattleChallenge NewChallenge;
	NewChallenge.ChallengeID = FGuid::NewGuid();
	NewChallenge.ChallengerCrewID = LocalCrewID;
	NewChallenge.ChallengerCrewName = LocalCrewName;
	NewChallenge.DefenderCrewID = TargetCrewID;
	NewChallenge.BattleType = BattleType;
	NewChallenge.Format = Format;
	NewChallenge.ProposedTime = ProposedTime;
	NewChallenge.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(24);
	NewChallenge.WagerAmount = WagerAmount;
	NewChallenge.bAccepted = false;

	OutgoingChallenges.Add(NewChallenge);
	SaveBattleData();

	return NewChallenge.ChallengeID;
}

bool UMGCrewBattlesSubsystem::AcceptChallenge(FGuid ChallengeID)
{
	for (int32 i = 0; i < IncomingChallenges.Num(); ++i)
	{
		if (IncomingChallenges[i].ChallengeID == ChallengeID)
		{
			FMGCrewBattleChallenge& Challenge = IncomingChallenges[i];
			Challenge.bAccepted = true;

			// Create battle from accepted challenge
			ActiveBattle = FMGCrewBattle();
			ActiveBattle.BattleID = FGuid::NewGuid();
			ActiveBattle.State = EMGCrewBattleState::Scheduled;
			ActiveBattle.BattleType = Challenge.BattleType;
			ActiveBattle.Format = Challenge.Format;
			ActiveBattle.ScheduledTime = Challenge.ProposedTime;

			// Challenger is Crew1
			ActiveBattle.Crew1.CrewID = Challenge.ChallengerCrewID;
			ActiveBattle.Crew1.CrewName = Challenge.ChallengerCrewName;

			// Local crew (defender) is Crew2
			ActiveBattle.Crew2.CrewID = LocalCrewID;
			ActiveBattle.Crew2.CrewName = LocalCrewName;
			ActiveBattle.Crew2.CrewTag = LocalCrewTag;
			ActiveBattle.Crew2.CrewRating = LocalCrewRating;

			// Set required wins
			switch (Challenge.Format)
			{
			case EMGCrewBattleFormat::BestOf1:
				ActiveBattle.RequiredWins = 1;
				break;
			case EMGCrewBattleFormat::BestOf3:
				ActiveBattle.RequiredWins = 2;
				break;
			case EMGCrewBattleFormat::BestOf5:
				ActiveBattle.RequiredWins = 3;
				break;
			default:
				ActiveBattle.RequiredWins = 1;
				break;
			}

			OnCrewChallengeResponse.Broadcast(ChallengeID, true);
			IncomingChallenges.RemoveAt(i);
			SaveBattleData();

			return true;
		}
	}

	return false;
}

bool UMGCrewBattlesSubsystem::DeclineChallenge(FGuid ChallengeID)
{
	for (int32 i = 0; i < IncomingChallenges.Num(); ++i)
	{
		if (IncomingChallenges[i].ChallengeID == ChallengeID)
		{
			OnCrewChallengeResponse.Broadcast(ChallengeID, false);
			IncomingChallenges.RemoveAt(i);
			SaveBattleData();
			return true;
		}
	}

	return false;
}

bool UMGCrewBattlesSubsystem::CancelChallenge(FGuid ChallengeID)
{
	for (int32 i = 0; i < OutgoingChallenges.Num(); ++i)
	{
		if (OutgoingChallenges[i].ChallengeID == ChallengeID)
		{
			OutgoingChallenges.RemoveAt(i);
			SaveBattleData();
			return true;
		}
	}

	return false;
}

TArray<FMGCrewBattleChallenge> UMGCrewBattlesSubsystem::GetIncomingChallenges() const
{
	return IncomingChallenges;
}

TArray<FMGCrewBattleChallenge> UMGCrewBattlesSubsystem::GetOutgoingChallenges() const
{
	return OutgoingChallenges;
}

// ============================================================================
// Active Battle
// ============================================================================

bool UMGCrewBattlesSubsystem::IsInBattle() const
{
	return ActiveBattle.State == EMGCrewBattleState::Preparing ||
		   ActiveBattle.State == EMGCrewBattleState::InProgress ||
		   ActiveBattle.State == EMGCrewBattleState::Scheduled;
}

void UMGCrewBattlesSubsystem::SetRosterReady(bool bReady)
{
	if (!IsInBattle())
	{
		return;
	}

	// Determine which crew we are
	if (ActiveBattle.Crew1.CrewID == LocalCrewID)
	{
		ActiveBattle.Crew1.bIsReady = bReady;
	}
	else if (ActiveBattle.Crew2.CrewID == LocalCrewID)
	{
		ActiveBattle.Crew2.bIsReady = bReady;
	}

	// Check if both crews are ready to start
	if (ActiveBattle.Crew1.bIsReady && ActiveBattle.Crew2.bIsReady &&
		ActiveBattle.State == EMGCrewBattleState::Preparing)
	{
		ActiveBattle.State = EMGCrewBattleState::InProgress;
		ActiveBattle.StartedTime = FDateTime::UtcNow();
		OnCrewBattleStarted.Broadcast(ActiveBattle);
	}
}

bool UMGCrewBattlesSubsystem::SetRoster(const TArray<FName>& PlayerIDs)
{
	if (!IsInBattle())
	{
		return false;
	}

	// Determine which crew we are and set roster
	if (ActiveBattle.Crew1.CrewID == LocalCrewID)
	{
		ActiveBattle.Crew1.RosterPlayerIDs = PlayerIDs;
		return true;
	}
	else if (ActiveBattle.Crew2.CrewID == LocalCrewID)
	{
		ActiveBattle.Crew2.RosterPlayerIDs = PlayerIDs;
		return true;
	}

	return false;
}

void UMGCrewBattlesSubsystem::BanMap(FName TrackID)
{
	if (!IsInBattle() || TrackID.IsNone())
	{
		return;
	}

	if (!ActiveBattle.BannedMaps.Contains(TrackID))
	{
		ActiveBattle.BannedMaps.Add(TrackID);
	}
}

void UMGCrewBattlesSubsystem::SelectMap(FName TrackID)
{
	if (!IsInBattle() || TrackID.IsNone())
	{
		return;
	}

	// Can't select a banned map
	if (ActiveBattle.BannedMaps.Contains(TrackID))
	{
		return;
	}

	// Set the map for the current round
	if (ActiveBattle.Rounds.Num() > ActiveBattle.CurrentRound)
	{
		ActiveBattle.Rounds[ActiveBattle.CurrentRound].TrackID = TrackID;
	}
}

TArray<FName> UMGCrewBattlesSubsystem::GetAvailableMaps() const
{
	TArray<FName> AvailableMaps;

	for (const FName& Map : ActiveBattle.MapPool)
	{
		if (!ActiveBattle.BannedMaps.Contains(Map))
		{
			AvailableMaps.Add(Map);
		}
	}

	return AvailableMaps;
}

// ============================================================================
// Round Management
// ============================================================================

void UMGCrewBattlesSubsystem::StartNextRound()
{
	if (ActiveBattle.State != EMGCrewBattleState::InProgress)
	{
		return;
	}

	// Check if battle is already won
	if (ActiveBattle.Crew1.Wins >= ActiveBattle.RequiredWins ||
		ActiveBattle.Crew2.Wins >= ActiveBattle.RequiredWins)
	{
		return;
	}

	FMGCrewBattleRound NewRound;
	NewRound.RoundNumber = ActiveBattle.Rounds.Num() + 1;
	NewRound.bCompleted = false;

	ActiveBattle.Rounds.Add(NewRound);
	ActiveBattle.CurrentRound = ActiveBattle.Rounds.Num() - 1;
}

void UMGCrewBattlesSubsystem::ReportRoundResult(const FMGCrewBattleRound& RoundResult)
{
	if (ActiveBattle.State != EMGCrewBattleState::InProgress)
	{
		return;
	}

	if (ActiveBattle.CurrentRound < 0 || ActiveBattle.CurrentRound >= ActiveBattle.Rounds.Num())
	{
		return;
	}

	FMGCrewBattleRound& CurrentRound = ActiveBattle.Rounds[ActiveBattle.CurrentRound];
	CurrentRound = RoundResult;
	CurrentRound.bCompleted = true;

	// Calculate points for point-based format
	if (ActiveBattle.Format == EMGCrewBattleFormat::PointBased)
	{
		CurrentRound.Crew1Points = CalculateRoundPoints(CurrentRound.PlayerPositions, true);
		CurrentRound.Crew2Points = CalculateRoundPoints(CurrentRound.PlayerPositions, false);

		ActiveBattle.Crew1.TotalPoints += CurrentRound.Crew1Points;
		ActiveBattle.Crew2.TotalPoints += CurrentRound.Crew2Points;
	}

	// Determine round winner
	if (!CurrentRound.WinnerCrewID.IsNone())
	{
		if (CurrentRound.WinnerCrewID == ActiveBattle.Crew1.CrewID)
		{
			ActiveBattle.Crew1.Wins++;
		}
		else if (CurrentRound.WinnerCrewID == ActiveBattle.Crew2.CrewID)
		{
			ActiveBattle.Crew2.Wins++;
		}
	}

	OnCrewBattleRoundComplete.Broadcast(ActiveBattle, CurrentRound);

	// Check for battle completion
	bool bBattleComplete = false;
	bool bWeWon = false;

	if (ActiveBattle.Format == EMGCrewBattleFormat::PointBased ||
		ActiveBattle.Format == EMGCrewBattleFormat::Elimination)
	{
		// Point-based: after all rounds, most points wins
		// For now, check if we've played required rounds
		if (ActiveBattle.Rounds.Num() >= ActiveBattle.RequiredWins * 2 - 1)
		{
			bBattleComplete = true;
			if (ActiveBattle.Format == EMGCrewBattleFormat::PointBased)
			{
				if (ActiveBattle.Crew1.CrewID == LocalCrewID)
				{
					bWeWon = ActiveBattle.Crew1.TotalPoints > ActiveBattle.Crew2.TotalPoints;
				}
				else
				{
					bWeWon = ActiveBattle.Crew2.TotalPoints > ActiveBattle.Crew1.TotalPoints;
				}
			}
		}
	}
	else
	{
		// Best-of format: first to required wins
		if (ActiveBattle.Crew1.Wins >= ActiveBattle.RequiredWins)
		{
			bBattleComplete = true;
			bWeWon = (ActiveBattle.Crew1.CrewID == LocalCrewID);
			ActiveBattle.WinnerCrewID = ActiveBattle.Crew1.CrewID;
		}
		else if (ActiveBattle.Crew2.Wins >= ActiveBattle.RequiredWins)
		{
			bBattleComplete = true;
			bWeWon = (ActiveBattle.Crew2.CrewID == LocalCrewID);
			ActiveBattle.WinnerCrewID = ActiveBattle.Crew2.CrewID;
		}
	}

	if (bBattleComplete)
	{
		ProcessBattleCompletion(bWeWon);
	}
}

FMGCrewBattleRound UMGCrewBattlesSubsystem::GetCurrentRound() const
{
	if (ActiveBattle.CurrentRound >= 0 && ActiveBattle.CurrentRound < ActiveBattle.Rounds.Num())
	{
		return ActiveBattle.Rounds[ActiveBattle.CurrentRound];
	}

	return FMGCrewBattleRound();
}

int32 UMGCrewBattlesSubsystem::GetRoundsToWin() const
{
	return ActiveBattle.RequiredWins;
}

void UMGCrewBattlesSubsystem::Forfeit()
{
	if (!IsInBattle())
	{
		return;
	}

	// Mark our crew as forfeiting
	bool bWeAreCrewOne = (ActiveBattle.Crew1.CrewID == LocalCrewID);

	if (bWeAreCrewOne)
	{
		ActiveBattle.Crew1.bForfeit = true;
		ActiveBattle.WinnerCrewID = ActiveBattle.Crew2.CrewID;
	}
	else
	{
		ActiveBattle.Crew2.bForfeit = true;
		ActiveBattle.WinnerCrewID = ActiveBattle.Crew1.CrewID;
	}

	// We lost by forfeit
	ProcessBattleCompletion(false);
}

// ============================================================================
// History & Stats
// ============================================================================

TArray<FMGCrewBattleHistory> UMGCrewBattlesSubsystem::GetBattleHistory(int32 MaxEntries) const
{
	if (MaxEntries <= 0 || MaxEntries >= BattleHistory.Num())
	{
		return BattleHistory;
	}

	TArray<FMGCrewBattleHistory> RecentHistory;
	int32 StartIndex = FMath::Max(0, BattleHistory.Num() - MaxEntries);

	for (int32 i = StartIndex; i < BattleHistory.Num(); ++i)
	{
		RecentHistory.Add(BattleHistory[i]);
	}

	return RecentHistory;
}

TArray<FMGCrewBattleHistory> UMGCrewBattlesSubsystem::GetHistoryVsCrew(FName CrewID) const
{
	TArray<FMGCrewBattleHistory> FilteredHistory;

	for (const FMGCrewBattleHistory& Entry : BattleHistory)
	{
		if (Entry.OpponentCrewID == CrewID)
		{
			FilteredHistory.Add(Entry);
		}
	}

	return FilteredHistory;
}

int32 UMGCrewBattlesSubsystem::GetHeadToHeadRecord(FName CrewID, int32& OutWins, int32& OutLosses) const
{
	OutWins = 0;
	OutLosses = 0;

	for (const FMGCrewBattleHistory& Entry : BattleHistory)
	{
		if (Entry.OpponentCrewID == CrewID)
		{
			if (Entry.bWon)
			{
				OutWins++;
			}
			else
			{
				OutLosses++;
			}
		}
	}

	return OutWins + OutLosses;
}

// ============================================================================
// Leaderboard
// ============================================================================

TArray<FMGCrewLeaderboardEntry> UMGCrewBattlesSubsystem::GetTopCrews(int32 Count) const
{
	if (Count <= 0 || Count >= CachedLeaderboard.Num())
	{
		return CachedLeaderboard;
	}

	TArray<FMGCrewLeaderboardEntry> TopEntries;
	for (int32 i = 0; i < FMath::Min(Count, CachedLeaderboard.Num()); ++i)
	{
		TopEntries.Add(CachedLeaderboard[i]);
	}

	return TopEntries;
}

int32 UMGCrewBattlesSubsystem::GetCrewLeaderboardPosition() const
{
	return LeaderboardPosition;
}

FMGCrewLeaderboardEntry UMGCrewBattlesSubsystem::GetCrewLeaderboardEntry(FName CrewID) const
{
	for (const FMGCrewLeaderboardEntry& Entry : CachedLeaderboard)
	{
		if (Entry.CrewID == CrewID)
		{
			return Entry;
		}
	}

	return FMGCrewLeaderboardEntry();
}

// ============================================================================
// Rating
// ============================================================================

int32 UMGCrewBattlesSubsystem::GetCrewRating() const
{
	return LocalCrewRating;
}

int32 UMGCrewBattlesSubsystem::PredictRatingChange(int32 OpponentRating, bool bWin) const
{
	return CalculateRatingChange(LocalCrewRating, OpponentRating, bWin);
}

// ============================================================================
// Crew Info
// ============================================================================

void UMGCrewBattlesSubsystem::SetLocalCrewInfo(FName CrewID, const FString& CrewName, const FString& CrewTag, int32 Rating)
{
	LocalCrewID = CrewID;
	LocalCrewName = CrewName;
	LocalCrewTag = CrewTag;
	LocalCrewRating = Rating;
}

// ============================================================================
// Network
// ============================================================================

void UMGCrewBattlesSubsystem::ReceiveBattleUpdate(const FMGCrewBattle& Battle)
{
	// Handle match found during matchmaking
	if (bIsMatchmaking && Battle.State == EMGCrewBattleState::Preparing)
	{
		bIsMatchmaking = false;
		MatchmakingStartTime = 0.0f;
		ActiveBattle = Battle;
		OnCrewBattleMatchFound.Broadcast(Battle);
		return;
	}

	// Update active battle state
	if (Battle.BattleID == ActiveBattle.BattleID)
	{
		EMGCrewBattleState PreviousState = ActiveBattle.State;
		ActiveBattle = Battle;

		// Fire events based on state changes
		if (PreviousState != EMGCrewBattleState::InProgress && Battle.State == EMGCrewBattleState::InProgress)
		{
			OnCrewBattleStarted.Broadcast(Battle);
		}
	}
}

void UMGCrewBattlesSubsystem::ReceiveChallenge(const FMGCrewBattleChallenge& Challenge)
{
	// Verify this challenge is for our crew
	if (Challenge.DefenderCrewID != LocalCrewID)
	{
		return;
	}

	// Check if we already have this challenge
	for (const FMGCrewBattleChallenge& Existing : IncomingChallenges)
	{
		if (Existing.ChallengeID == Challenge.ChallengeID)
		{
			return;
		}
	}

	IncomingChallenges.Add(Challenge);
	OnCrewChallengeReceived.Broadcast(Challenge);
	SaveBattleData();
}

// ============================================================================
// Protected Helpers
// ============================================================================

void UMGCrewBattlesSubsystem::OnBattleTick()
{
	// Check for expired challenges
	CheckExpiredChallenges();

	// Update matchmaking (simulate match found after some time for now)
	// In production, this would communicate with a backend service
}

void UMGCrewBattlesSubsystem::ProcessBattleCompletion(bool bWon)
{
	ActiveBattle.State = EMGCrewBattleState::Completed;
	ActiveBattle.CompletedTime = FDateTime::UtcNow();

	// Calculate and apply rating change
	int32 OpponentRating = (ActiveBattle.Crew1.CrewID == LocalCrewID)
		? ActiveBattle.Crew2.CrewRating
		: ActiveBattle.Crew1.CrewRating;

	int32 RatingChange = CalculateRatingChange(LocalCrewRating, OpponentRating, bWon);
	ActiveBattle.RatingChange = RatingChange;

	int32 OldRating = LocalCrewRating;
	LocalCrewRating = FMath::Max(0, LocalCrewRating + RatingChange);

	// Update stats
	UpdateStats(ActiveBattle, bWon);

	// Add to history
	AddToHistory(ActiveBattle, bWon);

	// Fire completion event
	OnCrewBattleComplete.Broadcast(ActiveBattle, bWon);

	// Fire rating change event if it changed
	if (OldRating != LocalCrewRating)
	{
		OnCrewRatingChanged.Broadcast(LocalCrewID, LocalCrewRating);
	}

	// Clear active battle
	FMGCrewBattle CompletedBattle = ActiveBattle;
	ActiveBattle = FMGCrewBattle();

	SaveBattleData();
}

int32 UMGCrewBattlesSubsystem::CalculateRatingChange(int32 CrewRating, int32 OpponentRating, bool bWon) const
{
	// ELO-based rating calculation
	const float KFactor = 32.0f;

	float ExpectedScore = 1.0f / (1.0f + FMath::Pow(10.0f, (OpponentRating - CrewRating) / 400.0f));
	float ActualScore = bWon ? 1.0f : 0.0f;

	int32 Change = FMath::RoundToInt(KFactor * (ActualScore - ExpectedScore));

	// Minimum change to prevent stagnation
	if (bWon && Change < 1)
	{
		Change = 1;
	}
	else if (!bWon && Change > -1)
	{
		Change = -1;
	}

	return Change;
}

void UMGCrewBattlesSubsystem::UpdateStats(const FMGCrewBattle& Battle, bool bWon)
{
	Stats.TotalBattles++;

	if (bWon)
	{
		Stats.Wins++;
		Stats.CurrentStreak++;
		Stats.BestWinStreak = FMath::Max(Stats.BestWinStreak, Stats.CurrentStreak);
	}
	else
	{
		Stats.Losses++;
		Stats.CurrentStreak = 0;
	}

	// Calculate win rate
	if (Stats.TotalBattles > 0)
	{
		Stats.WinRate = static_cast<float>(Stats.Wins) / static_cast<float>(Stats.TotalBattles);
	}

	// Track rounds
	bool bWeAreCrew1 = (Battle.Crew1.CrewID == LocalCrewID);
	if (bWeAreCrew1)
	{
		Stats.RoundsWon += Battle.Crew1.Wins;
		Stats.RoundsLost += Battle.Crew2.Wins;
	}
	else
	{
		Stats.RoundsWon += Battle.Crew2.Wins;
		Stats.RoundsLost += Battle.Crew1.Wins;
	}

	// Update most beaten crew and rival tracking
	FName OpponentID = bWeAreCrew1 ? Battle.Crew2.CrewID : Battle.Crew1.CrewID;

	// Count wins/losses vs this opponent
	int32 WinsVsOpponent = 0;
	int32 LossesVsOpponent = 0;

	for (const FMGCrewBattleHistory& Entry : BattleHistory)
	{
		if (Entry.OpponentCrewID == OpponentID)
		{
			if (Entry.bWon)
			{
				WinsVsOpponent++;
			}
			else
			{
				LossesVsOpponent++;
			}
		}
	}

	// Include current battle
	if (bWon)
	{
		WinsVsOpponent++;
	}
	else
	{
		LossesVsOpponent++;
	}

	// Update most beaten crew
	if (WinsVsOpponent > Stats.BeatenCount)
	{
		Stats.MostBeatenCrew = OpponentID;
		Stats.BeatenCount = WinsVsOpponent;
	}

	// Update rival (most competitive matchup)
	int32 TotalGames = WinsVsOpponent + LossesVsOpponent;
	int32 RivalTotalGames = Stats.RivalWins + Stats.RivalLosses;

	if (TotalGames >= 3 && TotalGames > RivalTotalGames)
	{
		Stats.RivalCrew = OpponentID;
		Stats.RivalWins = WinsVsOpponent;
		Stats.RivalLosses = LossesVsOpponent;
	}
}

void UMGCrewBattlesSubsystem::AddToHistory(const FMGCrewBattle& Battle, bool bWon)
{
	FMGCrewBattleHistory HistoryEntry;
	HistoryEntry.BattleID = Battle.BattleID;
	HistoryEntry.BattleType = Battle.BattleType;
	HistoryEntry.bWon = bWon;
	HistoryEntry.RatingChange = Battle.RatingChange;
	HistoryEntry.CompletedAt = Battle.CompletedTime;

	bool bWeAreCrew1 = (Battle.Crew1.CrewID == LocalCrewID);

	if (bWeAreCrew1)
	{
		HistoryEntry.OpponentCrewID = Battle.Crew2.CrewID;
		HistoryEntry.OpponentCrewName = Battle.Crew2.CrewName;
		HistoryEntry.ScoreFor = Battle.Crew1.Wins;
		HistoryEntry.ScoreAgainst = Battle.Crew2.Wins;
	}
	else
	{
		HistoryEntry.OpponentCrewID = Battle.Crew1.CrewID;
		HistoryEntry.OpponentCrewName = Battle.Crew1.CrewName;
		HistoryEntry.ScoreFor = Battle.Crew2.Wins;
		HistoryEntry.ScoreAgainst = Battle.Crew1.Wins;
	}

	// Track which tracks were played
	for (const FMGCrewBattleRound& Round : Battle.Rounds)
	{
		if (!Round.TrackID.IsNone())
		{
			HistoryEntry.TracksPlayed.Add(Round.TrackID);
		}
	}

	BattleHistory.Add(HistoryEntry);

	// Limit history size
	const int32 MaxHistoryEntries = 100;
	while (BattleHistory.Num() > MaxHistoryEntries)
	{
		BattleHistory.RemoveAt(0);
	}
}

void UMGCrewBattlesSubsystem::SaveBattleData()
{
	// Persist battle data to save game
	// Implementation would use USaveGame or similar system
}

void UMGCrewBattlesSubsystem::LoadBattleData()
{
	// Load persisted battle data
	// Implementation would use USaveGame or similar system
}

void UMGCrewBattlesSubsystem::CheckExpiredChallenges()
{
	FDateTime Now = FDateTime::UtcNow();

	// Remove expired incoming challenges
	for (int32 i = IncomingChallenges.Num() - 1; i >= 0; --i)
	{
		if (IncomingChallenges[i].ExpiresAt < Now)
		{
			IncomingChallenges.RemoveAt(i);
		}
	}

	// Remove expired outgoing challenges
	for (int32 i = OutgoingChallenges.Num() - 1; i >= 0; --i)
	{
		if (OutgoingChallenges[i].ExpiresAt < Now)
		{
			OutgoingChallenges.RemoveAt(i);
		}
	}
}

int32 UMGCrewBattlesSubsystem::CalculateRoundPoints(const TMap<FName, int32>& Positions, bool bIsOurCrew) const
{
	int32 TotalPoints = 0;

	// Point values per position (1st = 25, 2nd = 18, 3rd = 15, etc.)
	const TArray<int32> PointValues = { 25, 18, 15, 12, 10, 8, 6, 4, 2, 1 };

	// Get roster for the crew we're calculating points for
	const TArray<FName>& CrewRoster = bIsOurCrew
		? (ActiveBattle.Crew1.CrewID == LocalCrewID ? ActiveBattle.Crew1.RosterPlayerIDs : ActiveBattle.Crew2.RosterPlayerIDs)
		: (ActiveBattle.Crew1.CrewID == LocalCrewID ? ActiveBattle.Crew2.RosterPlayerIDs : ActiveBattle.Crew1.RosterPlayerIDs);

	for (const auto& Pair : Positions)
	{
		// Check if this player is on the crew
		if (CrewRoster.Contains(Pair.Key))
		{
			int32 Position = Pair.Value - 1; // Convert to 0-based index
			if (Position >= 0 && Position < PointValues.Num())
			{
				TotalPoints += PointValues[Position];
			}
		}
	}

	return TotalPoints;
}

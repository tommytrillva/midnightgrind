// Copyright Midnight Grind. All Rights Reserved.

#include "Leaderboard/MGLeaderboardSubsystem.h"
#include "Misc/DateTime.h"

void UMGLeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadPersonalBests();
}

void UMGLeaderboardSubsystem::Deinitialize()
{
	SavePersonalBests();
	Super::Deinitialize();
}

// ==========================================
// QUERIES
// ==========================================

void UMGLeaderboardSubsystem::QueryLeaderboard(const FMGLeaderboardQuery& Query)
{
	if (bQueryInProgress)
	{
		return;
	}

	bQueryInProgress = true;
	PendingQuery = Query;

	// In production, this would make an async network request
	// For now, simulate with immediate response
	ProcessQueryResult(Query);
}

void UMGLeaderboardSubsystem::QueryTopEntries(EMGLeaderboardType Type, FName TrackID, int32 Count)
{
	FMGLeaderboardQuery Query;
	Query.Type = Type;
	Query.Scope = EMGLeaderboardScope::Global;
	Query.TrackID = TrackID;
	Query.StartRank = 1;
	Query.MaxEntries = Count;
	Query.bAroundPlayer = false;

	QueryLeaderboard(Query);
}

void UMGLeaderboardSubsystem::QueryAroundPlayer(EMGLeaderboardType Type, FName TrackID, int32 Range)
{
	FMGLeaderboardQuery Query;
	Query.Type = Type;
	Query.Scope = EMGLeaderboardScope::Global;
	Query.TrackID = TrackID;
	Query.MaxEntries = Range * 2 + 1;
	Query.bAroundPlayer = true;

	QueryLeaderboard(Query);
}

void UMGLeaderboardSubsystem::QueryFriendsLeaderboard(EMGLeaderboardType Type, FName TrackID)
{
	FMGLeaderboardQuery Query;
	Query.Type = Type;
	Query.Scope = EMGLeaderboardScope::Friends;
	Query.TrackID = TrackID;
	Query.StartRank = 1;
	Query.MaxEntries = 100;
	Query.bAroundPlayer = false;

	QueryLeaderboard(Query);
}

FMGLeaderboardResult UMGLeaderboardSubsystem::GetCachedResult(EMGLeaderboardType Type, FName TrackID) const
{
	FString Key = GetCacheKey(Type, EMGLeaderboardScope::Global, TrackID);
	if (const FMGLeaderboardResult* Result = CachedResults.Find(Key))
	{
		return *Result;
	}
	return FMGLeaderboardResult();
}

// ==========================================
// SUBMISSIONS
// ==========================================

void UMGLeaderboardSubsystem::SubmitLapTime(FName TrackID, float LapTime, FName VehicleID, const FString& GhostReplayID)
{
	ProcessSubmissionResult(EMGLeaderboardType::LapTime, LapTime, TrackID, VehicleID, GhostReplayID);
}

void UMGLeaderboardSubsystem::SubmitRaceTime(FName TrackID, float RaceTime, FName VehicleID, int32 Position)
{
	ProcessSubmissionResult(EMGLeaderboardType::RaceTime, RaceTime, TrackID, VehicleID, TEXT(""));

	// Also submit win if first place
	if (Position == 1)
	{
		SubmitScore(EMGLeaderboardType::Wins, 1.0f);
	}
}

void UMGLeaderboardSubsystem::SubmitDriftScore(FName TrackID, int32 DriftScore)
{
	ProcessSubmissionResult(EMGLeaderboardType::DriftScore, static_cast<float>(DriftScore), TrackID, NAME_None, TEXT(""));
}

void UMGLeaderboardSubsystem::SubmitScore(EMGLeaderboardType Type, float Score, FName TrackID, FName VehicleID)
{
	ProcessSubmissionResult(Type, Score, TrackID, VehicleID, TEXT(""));
}

// ==========================================
// PERSONAL BESTS
// ==========================================

FMGPersonalBest UMGLeaderboardSubsystem::GetPersonalBest(FName TrackID) const
{
	if (const FMGPersonalBest* PB = PersonalBests.Find(TrackID))
	{
		return *PB;
	}
	return FMGPersonalBest();
}

TArray<FMGPersonalBest> UMGLeaderboardSubsystem::GetAllPersonalBests() const
{
	TArray<FMGPersonalBest> Result;
	for (const auto& Pair : PersonalBests)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

int32 UMGLeaderboardSubsystem::GetTrackMedal(FName TrackID) const
{
	const FMGPersonalBest* PB = PersonalBests.Find(TrackID);
	if (!PB)
	{
		return 0; // No medal
	}

	// Medal based on rank (Gold = top 1%, Silver = top 5%, Bronze = top 10%)
	if (PB->LapTimeRank <= 100)
	{
		return 3; // Gold
	}
	else if (PB->LapTimeRank <= 500)
	{
		return 2; // Silver
	}
	else if (PB->LapTimeRank <= 1000)
	{
		return 1; // Bronze
	}

	return 0;
}

bool UMGLeaderboardSubsystem::HasPersonalBest(FName TrackID) const
{
	return PersonalBests.Contains(TrackID);
}

// ==========================================
// PLAYER STATS
// ==========================================

int32 UMGLeaderboardSubsystem::GetGlobalRank(EMGLeaderboardType Type) const
{
	// Would query from server - return mock data
	return 1234;
}

int32 UMGLeaderboardSubsystem::GetTotalEntriesCount(EMGLeaderboardType Type, FName TrackID) const
{
	FString Key = GetCacheKey(Type, EMGLeaderboardScope::Global, TrackID);
	if (const FMGLeaderboardResult* Result = CachedResults.Find(Key))
	{
		return Result->TotalEntries;
	}
	return 0;
}

float UMGLeaderboardSubsystem::GetPercentileRank(EMGLeaderboardType Type, FName TrackID) const
{
	int32 Rank = GetGlobalRank(Type);
	int32 Total = GetTotalEntriesCount(Type, TrackID);

	if (Total <= 0)
	{
		return 0.0f;
	}

	return (1.0f - (static_cast<float>(Rank) / static_cast<float>(Total))) * 100.0f;
}

// ==========================================
// UTILITY
// ==========================================

FString UMGLeaderboardSubsystem::FormatTime(float TimeSeconds)
{
	int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeSeconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(Seconds);
	int32 Milliseconds = FMath::FloorToInt((Seconds - WholeSeconds) * 1000.0f);

	return FString::Printf(TEXT("%d:%02d.%03d"), Minutes, WholeSeconds, Milliseconds);
}

FString UMGLeaderboardSubsystem::FormatRank(int32 Rank)
{
	if (Rank <= 0)
	{
		return TEXT("-");
	}

	FString Suffix;
	int32 LastDigit = Rank % 10;
	int32 LastTwoDigits = Rank % 100;

	if (LastTwoDigits >= 11 && LastTwoDigits <= 13)
	{
		Suffix = TEXT("th");
	}
	else if (LastDigit == 1)
	{
		Suffix = TEXT("st");
	}
	else if (LastDigit == 2)
	{
		Suffix = TEXT("nd");
	}
	else if (LastDigit == 3)
	{
		Suffix = TEXT("rd");
	}
	else
	{
		Suffix = TEXT("th");
	}

	return FString::Printf(TEXT("%d%s"), Rank, *Suffix);
}

FText UMGLeaderboardSubsystem::GetLeaderboardDisplayName(EMGLeaderboardType Type)
{
	switch (Type)
	{
		case EMGLeaderboardType::LapTime: return FText::FromString(TEXT("Best Lap Times"));
		case EMGLeaderboardType::RaceTime: return FText::FromString(TEXT("Race Times"));
		case EMGLeaderboardType::Reputation: return FText::FromString(TEXT("Reputation"));
		case EMGLeaderboardType::Wins: return FText::FromString(TEXT("Total Wins"));
		case EMGLeaderboardType::WinStreak: return FText::FromString(TEXT("Win Streak"));
		case EMGLeaderboardType::DriftScore: return FText::FromString(TEXT("Drift Masters"));
		case EMGLeaderboardType::PerfectStarts: return FText::FromString(TEXT("Perfect Starts"));
		case EMGLeaderboardType::WeeklyChallenge: return FText::FromString(TEXT("Weekly Challenge"));
		case EMGLeaderboardType::SeasonRanking: return FText::FromString(TEXT("Season Rankings"));
		default: return FText::FromString(TEXT("Leaderboard"));
	}
}

// ==========================================
// INTERNAL
// ==========================================

FString UMGLeaderboardSubsystem::GetCacheKey(EMGLeaderboardType Type, EMGLeaderboardScope Scope, FName TrackID) const
{
	return FString::Printf(TEXT("%d_%d_%s"), static_cast<int32>(Type), static_cast<int32>(Scope), *TrackID.ToString());
}

void UMGLeaderboardSubsystem::ProcessQueryResult(const FMGLeaderboardQuery& Query)
{
	FMGLeaderboardResult Result;
	Result.Query = Query;
	Result.bSuccess = true;
	Result.Entries = GenerateMockEntries(Query);
	Result.TotalEntries = 10000; // Mock total

	// Find local player rank
	for (int32 i = 0; i < Result.Entries.Num(); i++)
	{
		if (Result.Entries[i].bIsLocalPlayer)
		{
			Result.LocalPlayerRank = Result.Entries[i].Rank;
			Result.LocalPlayerScore = Result.Entries[i].Score;
			break;
		}
	}

	// Cache result
	FString Key = GetCacheKey(Query.Type, Query.Scope, Query.TrackID);
	CachedResults.Add(Key, Result);

	bQueryInProgress = false;

	OnLeaderboardQueryComplete.Broadcast(Result);
}

void UMGLeaderboardSubsystem::ProcessSubmissionResult(EMGLeaderboardType Type, float Score, FName TrackID, FName VehicleID, const FString& GhostID)
{
	FMGScoreSubmissionResult Result;
	Result.bSuccess = true;

	// Check personal best
	FMGPersonalBest* CurrentPB = PersonalBests.Find(TrackID);
	float CurrentBest = 0.0f;

	if (Type == EMGLeaderboardType::LapTime)
	{
		CurrentBest = CurrentPB ? CurrentPB->BestLapTime : FLT_MAX;
		Result.bIsPersonalBest = (Score < CurrentBest);
	}
	else if (Type == EMGLeaderboardType::RaceTime)
	{
		CurrentBest = CurrentPB ? CurrentPB->BestRaceTime : FLT_MAX;
		Result.bIsPersonalBest = (Score < CurrentBest);
	}
	else
	{
		// For score-based leaderboards, higher is better
		CurrentBest = CurrentPB ? CurrentPB->BestLapTime : 0.0f;
		Result.bIsPersonalBest = (Score > CurrentBest);
	}

	if (Result.bIsPersonalBest)
	{
		Result.OldPersonalBest = CurrentBest;
		Result.ScoreImprovement = FMath::Abs(Score - CurrentBest);

		UpdatePersonalBest(TrackID, Type, Score, VehicleID);
	}

	// Mock rank data
	Result.NewRank = FMath::RandRange(100, 5000);
	Result.OldRank = Result.NewRank + FMath::RandRange(0, 100);
	Result.RankImprovement = Result.OldRank - Result.NewRank;

	OnScoreSubmissionComplete.Broadcast(Result);
}

void UMGLeaderboardSubsystem::UpdatePersonalBest(FName TrackID, EMGLeaderboardType Type, float Score, FName VehicleID)
{
	FMGPersonalBest& PB = PersonalBests.FindOrAdd(TrackID);
	PB.TrackID = TrackID;

	if (Type == EMGLeaderboardType::LapTime)
	{
		PB.BestLapTime = Score;
		PB.LapRecordVehicle = VehicleID;
		PB.LapRecordDate = FDateTime::Now();
		PB.LapTimeRank = FMath::RandRange(100, 5000); // Mock
	}
	else if (Type == EMGLeaderboardType::RaceTime)
	{
		PB.BestRaceTime = Score;
		PB.RaceRecordVehicle = VehicleID;
		PB.RaceRecordDate = FDateTime::Now();
		PB.RaceTimeRank = FMath::RandRange(100, 5000); // Mock
	}

	SavePersonalBests();
	OnPersonalBestUpdated.Broadcast(TrackID, PB);
}

TArray<FMGLeaderboardEntry> UMGLeaderboardSubsystem::GenerateMockEntries(const FMGLeaderboardQuery& Query) const
{
	TArray<FMGLeaderboardEntry> Entries;

	// Mock racer names
	const TArray<FString> MockNames = {
		TEXT("NightRider_X"), TEXT("DriftKing99"), TEXT("SpeedDemon"),
		TEXT("MidnightRacer"), TEXT("NeonPhantom"), TEXT("TurboTony"),
		TEXT("StreetLegend"), TEXT("GhostRunner"), TEXT("RoadWarrior"),
		TEXT("AsphaltAssassin"), TEXT("NitroNinja"), TEXT("BurnoutBoss"),
		TEXT("SlipstreamSam"), TEXT("ApexPredator"), TEXT("TrackTerror")
	};

	const TArray<FString> MockCrews = {
		TEXT("Midnight Runners"), TEXT("Street Kings"), TEXT("Neon Racers"),
		TEXT("Urban Legends"), TEXT(""), TEXT(""), TEXT("Night Owls")
	};

	int32 StartRank = Query.bAroundPlayer ? FMath::Max(1, 1234 - Query.MaxEntries / 2) : Query.StartRank;
	int32 LocalPlayerRank = 1234; // Mock local player rank

	float BaseTime = 65.0f; // ~1:05 base lap time

	for (int32 i = 0; i < Query.MaxEntries; i++)
	{
		FMGLeaderboardEntry Entry;
		Entry.Rank = StartRank + i;
		Entry.PlayerID = FString::Printf(TEXT("player_%d"), Entry.Rank);

		// Check if this is the local player position
		if (Entry.Rank == LocalPlayerRank)
		{
			Entry.PlayerName = TEXT("You");
			Entry.bIsLocalPlayer = true;
		}
		else
		{
			Entry.PlayerName = MockNames[FMath::RandRange(0, MockNames.Num() - 1)];
		}

		// Time increases with rank (lower rank = faster time)
		if (Query.Type == EMGLeaderboardType::LapTime || Query.Type == EMGLeaderboardType::RaceTime)
		{
			float RankFactor = static_cast<float>(Entry.Rank) / 10000.0f;
			Entry.Score = BaseTime + (RankFactor * 30.0f) + FMath::FRandRange(-0.5f, 0.5f);

			if (Query.Type == EMGLeaderboardType::RaceTime)
			{
				Entry.Score *= 3.0f; // 3 laps
			}
		}
		else
		{
			// Score-based (higher is better at lower ranks)
			Entry.Score = 100000.0f - (Entry.Rank * 10.0f) + FMath::FRandRange(-50.0f, 50.0f);
		}

		Entry.VehicleID = FName(*FString::Printf(TEXT("Vehicle_%d"), FMath::RandRange(1, 10)));
		Entry.Timestamp = FDateTime::Now() - FTimespan::FromDays(FMath::RandRange(0, 30));
		Entry.CrewName = MockCrews[FMath::RandRange(0, MockCrews.Num() - 1)];
		Entry.Region = TEXT("NA");
		Entry.bIsFriend = FMath::RandBool() && !Entry.bIsLocalPlayer && Entry.Rank < 20;
		Entry.bHasGhost = Entry.Rank <= 100 || Entry.bIsLocalPlayer;

		if (Entry.bHasGhost)
		{
			Entry.GhostReplayID = FString::Printf(TEXT("ghost_%s_%d"), *Query.TrackID.ToString(), Entry.Rank);
		}

		Entries.Add(Entry);
	}

	return Entries;
}

void UMGLeaderboardSubsystem::LoadPersonalBests()
{
	// Would load from save file
	// For now, start with empty data
	PersonalBests.Empty();
}

void UMGLeaderboardSubsystem::SavePersonalBests()
{
	// Would save to persistent storage
}

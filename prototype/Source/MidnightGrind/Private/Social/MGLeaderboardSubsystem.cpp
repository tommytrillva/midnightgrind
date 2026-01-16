// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGLeaderboardSubsystem.h"

void UMGLeaderboardSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeData();
}

void UMGLeaderboardSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// LEADERBOARDS
// ==========================================

void UMGLeaderboardSubsystem::GetLeaderboard(EMGLeaderboardType Type, EMGLeaderboardScope Scope, EMGLeaderboardTimeFilter TimeFilter, int32 StartRank, int32 Count)
{
	// In production, this would fetch from server
	TArray<FMGLeaderboardEntry> Entries = CreateMockLeaderboard(Type, Count);

	// Adjust ranks based on start
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Entries[i].Rank = StartRank + i;
	}

	// Cache result
	FName CacheKey = *FString::Printf(TEXT("%d_%d_%d"), static_cast<int32>(Type), static_cast<int32>(Scope), static_cast<int32>(TimeFilter));
	LeaderboardCache.Add(CacheKey, Entries);

	OnLeaderboardLoaded.Broadcast(Type, Entries);
}

void UMGLeaderboardSubsystem::GetTrackLeaderboard(FName TrackID, EMGLeaderboardScope Scope, int32 StartRank, int32 Count)
{
	TArray<FMGLeaderboardEntry> Entries = CreateMockLeaderboard(EMGLeaderboardType::LapTime, Count);

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Entries[i].Rank = StartRank + i;
		// Generate realistic lap times (60-90 seconds)
		float LapTime = 60.0f + (i * 0.5f) + FMath::RandRange(0.0f, 0.3f);
		Entries[i].Score = static_cast<int64>(LapTime * 1000); // Store as milliseconds
		Entries[i].FormattedScore = FormatTime(LapTime);
	}

	OnLeaderboardLoaded.Broadcast(EMGLeaderboardType::LapTime, Entries);
}

int32 UMGLeaderboardSubsystem::GetPlayerLeaderboardPosition(EMGLeaderboardType Type, EMGLeaderboardScope Scope) const
{
	// Mock - return random position based on scope
	switch (Scope)
	{
	case EMGLeaderboardScope::Global:
		return FMath::RandRange(5000, 50000);
	case EMGLeaderboardScope::Regional:
		return FMath::RandRange(500, 5000);
	case EMGLeaderboardScope::Friends:
		return FMath::RandRange(1, 20);
	case EMGLeaderboardScope::Crew:
		return FMath::RandRange(1, 50);
	default:
		return 0;
	}
}

TArray<FMGLeaderboardEntry> UMGLeaderboardSubsystem::GetEntriesAroundPlayer(EMGLeaderboardType Type, int32 Range)
{
	int32 PlayerRank = GetPlayerLeaderboardPosition(Type, EMGLeaderboardScope::Global);
	int32 StartRank = FMath::Max(1, PlayerRank - Range);
	int32 Count = Range * 2 + 1;

	TArray<FMGLeaderboardEntry> Entries = CreateMockLeaderboard(Type, Count);

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Entries[i].Rank = StartRank + i;

		if (Entries[i].Rank == PlayerRank)
		{
			Entries[i].bIsLocalPlayer = true;
			Entries[i].DisplayName = NSLOCTEXT("Leaderboard", "You", "You");
		}
	}

	return Entries;
}

// ==========================================
// TRACK RECORDS
// ==========================================

FMGTrackRecord UMGLeaderboardSubsystem::GetTrackRecords(FName TrackID) const
{
	if (const FMGTrackRecord* Record = TrackRecordsCache.Find(TrackID))
	{
		return *Record;
	}

	return FMGTrackRecord();
}

TArray<FMGTrackRecord> UMGLeaderboardSubsystem::GetAllTrackRecords() const
{
	TArray<FMGTrackRecord> Records;
	TrackRecordsCache.GenerateValueArray(Records);
	return Records;
}

void UMGLeaderboardSubsystem::SubmitLapTime(FName TrackID, float LapTime, FName VehicleID, const TArray<uint8>& GhostData)
{
	FMGTrackRecord* Record = TrackRecordsCache.Find(TrackID);
	if (!Record)
	{
		return;
	}

	int64 LapTimeMS = static_cast<int64>(LapTime * 1000);

	// Check if personal best
	bool bIsPersonalBest = Record->PersonalBest.Score == 0 || LapTimeMS < Record->PersonalBest.Score;

	if (bIsPersonalBest)
	{
		int64 OldScore = Record->PersonalBest.Score;

		Record->PersonalBest.Score = LapTimeMS;
		Record->PersonalBest.FormattedScore = FormatTime(LapTime);
		Record->PersonalBest.VehicleID = VehicleID;
		Record->PersonalBest.DateAchieved = FDateTime::UtcNow();
		Record->PersonalBest.bIsLocalPlayer = true;
		Record->PersonalBest.DisplayName = NSLOCTEXT("Leaderboard", "You", "You");
		Record->bHasGhost = GhostData.Num() > 0;

		OnNewPersonalBest.Broadcast(EMGLeaderboardType::LapTime, OldScore, LapTimeMS);

		// Check if world/regional record
		if (LapTimeMS < Record->WorldRecord.Score)
		{
			Record->WorldRecord = Record->PersonalBest;
			OnTrackRecordBroken.Broadcast(*Record);
		}
	}
}

void UMGLeaderboardSubsystem::SubmitRaceTime(FName TrackID, float RaceTime, int32 LapCount, FName VehicleID)
{
	// Similar to lap time but for full race
	SubmitLapTime(TrackID, RaceTime, VehicleID, TArray<uint8>());
}

TArray<uint8> UMGLeaderboardSubsystem::GetRecordGhostData(FName TrackID, EMGLeaderboardScope Scope)
{
	// In production, would fetch from server
	return TArray<uint8>();
}

// ==========================================
// RANKED
// ==========================================

void UMGLeaderboardSubsystem::SubmitRankedResult(int32 Position, int32 TotalRacers)
{
	// Update placement matches
	if (PlayerRankedData.bIsInPlacement)
	{
		PlayerRankedData.PlacementMatchesCompleted++;

		if (Position == 1)
		{
			PlayerRankedData.SeasonWins++;
		}
		else
		{
			PlayerRankedData.SeasonLosses++;
		}

		if (PlayerRankedData.PlacementMatchesCompleted >= PlayerRankedData.PlacementMatchesRequired)
		{
			PlayerRankedData.bIsInPlacement = false;

			// Calculate initial rating based on placement performance
			float WinRate = static_cast<float>(PlayerRankedData.SeasonWins) / PlayerRankedData.PlacementMatchesCompleted;
			PlayerRankedData.RatingPoints = static_cast<int32>(1000 + (WinRate * 500));

			UpdateTierFromRating();
		}

		return;
	}

	// Calculate rating change
	int32 RatingChange = CalculateRatingChange(Position, TotalRacers, PlayerRankedData.RatingPoints);

	int32 OldRating = PlayerRankedData.RatingPoints;
	int32 OldRank = PlayerRankedData.CurrentRank;
	EMGRankTier OldTier = PlayerRankedData.CurrentTier;

	PlayerRankedData.RatingPoints = FMath::Max(0, PlayerRankedData.RatingPoints + RatingChange);

	// Update wins/losses
	if (Position == 1)
	{
		PlayerRankedData.SeasonWins++;
		PlayerRankedData.WinStreak++;
		PlayerRankedData.BestWinStreak = FMath::Max(PlayerRankedData.BestWinStreak, PlayerRankedData.WinStreak);
	}
	else if (Position > TotalRacers / 2)
	{
		PlayerRankedData.SeasonLosses++;
		PlayerRankedData.WinStreak = 0;
	}

	UpdateTierFromRating();

	// Check for rank change
	// Mock rank calculation
	int32 NewRank = FMath::Max(1, 10000 - (PlayerRankedData.RatingPoints / 2));
	if (NewRank != OldRank)
	{
		PlayerRankedData.CurrentRank = NewRank;
		PlayerRankedData.PeakRank = FMath::Min(PlayerRankedData.PeakRank, NewRank);

		OnRankChanged.Broadcast(OldRank, NewRank);
	}

	if (PlayerRankedData.CurrentTier != OldTier)
	{
		if (static_cast<int32>(PlayerRankedData.CurrentTier) > static_cast<int32>(PlayerRankedData.PeakTier))
		{
			PlayerRankedData.PeakTier = PlayerRankedData.CurrentTier;
		}

		OnTierChanged.Broadcast(OldTier, PlayerRankedData.CurrentTier);
	}
}

int32 UMGLeaderboardSubsystem::GetRatingChangePreview(int32 ExpectedPosition, int32 TotalRacers) const
{
	return CalculateRatingChange(ExpectedPosition, TotalRacers, PlayerRankedData.RatingPoints);
}

// ==========================================
// WEEKLY COMPETITIONS
// ==========================================

void UMGLeaderboardSubsystem::GetWeeklyCompetitionLeaderboard(const FString& CompetitionID, int32 StartRank, int32 Count)
{
	TArray<FMGLeaderboardEntry> Entries = CreateMockLeaderboard(EMGLeaderboardType::Weekly, Count);

	for (int32 i = 0; i < Entries.Num(); i++)
	{
		Entries[i].Rank = StartRank + i;
	}

	OnLeaderboardLoaded.Broadcast(EMGLeaderboardType::Weekly, Entries);
}

void UMGLeaderboardSubsystem::SubmitWeeklyScore(const FString& CompetitionID, int64 Score)
{
	FMGWeeklyCompetition* Competition = WeeklyCompetitions.FindByPredicate([CompetitionID](const FMGWeeklyCompetition& C)
	{
		return C.CompetitionID == CompetitionID;
	});

	if (Competition)
	{
		if (Score > Competition->PlayerBestScore)
		{
			int64 OldScore = Competition->PlayerBestScore;
			Competition->PlayerBestScore = Score;
			Competition->bHasParticipated = true;

			OnNewPersonalBest.Broadcast(EMGLeaderboardType::Weekly, OldScore, Score);
		}
	}
}

// ==========================================
// STATISTICS
// ==========================================

int64 UMGLeaderboardSubsystem::GetPlayerStatistic(EMGLeaderboardType Type) const
{
	if (const int64* Value = PlayerStatistics.Find(Type))
	{
		return *Value;
	}
	return 0;
}

float UMGLeaderboardSubsystem::GetWinRate() const
{
	if (TotalRacesCompleted == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(TotalWins) / TotalRacesCompleted * 100.0f;
}

float UMGLeaderboardSubsystem::GetAverageFinishPosition() const
{
	if (TotalRacesCompleted == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(TotalPositionSum) / TotalRacesCompleted;
}

// ==========================================
// UTILITY
// ==========================================

FText UMGLeaderboardSubsystem::GetTierDisplayName(EMGRankTier Tier)
{
	switch (Tier)
	{
	case EMGRankTier::Unranked:
		return NSLOCTEXT("Leaderboard", "Unranked", "Unranked");
	case EMGRankTier::Bronze:
		return NSLOCTEXT("Leaderboard", "Bronze", "Bronze");
	case EMGRankTier::Silver:
		return NSLOCTEXT("Leaderboard", "Silver", "Silver");
	case EMGRankTier::Gold:
		return NSLOCTEXT("Leaderboard", "Gold", "Gold");
	case EMGRankTier::Platinum:
		return NSLOCTEXT("Leaderboard", "Platinum", "Platinum");
	case EMGRankTier::Diamond:
		return NSLOCTEXT("Leaderboard", "Diamond", "Diamond");
	case EMGRankTier::Champion:
		return NSLOCTEXT("Leaderboard", "Champion", "Champion");
	case EMGRankTier::Legend:
		return NSLOCTEXT("Leaderboard", "Legend", "Legend");
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UMGLeaderboardSubsystem::GetTierColor(EMGRankTier Tier)
{
	switch (Tier)
	{
	case EMGRankTier::Unranked:
		return FLinearColor(0.5f, 0.5f, 0.5f);
	case EMGRankTier::Bronze:
		return FLinearColor(0.8f, 0.5f, 0.2f);
	case EMGRankTier::Silver:
		return FLinearColor(0.75f, 0.75f, 0.8f);
	case EMGRankTier::Gold:
		return FLinearColor(1.0f, 0.84f, 0.0f);
	case EMGRankTier::Platinum:
		return FLinearColor(0.4f, 0.85f, 0.9f);
	case EMGRankTier::Diamond:
		return FLinearColor(0.7f, 0.5f, 1.0f);
	case EMGRankTier::Champion:
		return FLinearColor(1.0f, 0.3f, 0.3f);
	case EMGRankTier::Legend:
		return FLinearColor(1.0f, 0.85f, 0.0f);
	default:
		return FLinearColor::White;
	}
}

FText UMGLeaderboardSubsystem::GetLeaderboardTypeDisplayName(EMGLeaderboardType Type)
{
	switch (Type)
	{
	case EMGLeaderboardType::LapTime:
		return NSLOCTEXT("Leaderboard", "LapTime", "Lap Time");
	case EMGLeaderboardType::RaceTime:
		return NSLOCTEXT("Leaderboard", "RaceTime", "Race Time");
	case EMGLeaderboardType::Wins:
		return NSLOCTEXT("Leaderboard", "Wins", "Wins");
	case EMGLeaderboardType::TotalXP:
		return NSLOCTEXT("Leaderboard", "TotalXP", "Total XP");
	case EMGLeaderboardType::Reputation:
		return NSLOCTEXT("Leaderboard", "Reputation", "Reputation");
	case EMGLeaderboardType::Distance:
		return NSLOCTEXT("Leaderboard", "Distance", "Distance");
	case EMGLeaderboardType::DriftScore:
		return NSLOCTEXT("Leaderboard", "DriftScore", "Drift Score");
	case EMGLeaderboardType::CleanLaps:
		return NSLOCTEXT("Leaderboard", "CleanLaps", "Clean Laps");
	case EMGLeaderboardType::Weekly:
		return NSLOCTEXT("Leaderboard", "Weekly", "Weekly");
	case EMGLeaderboardType::Season:
		return NSLOCTEXT("Leaderboard", "Season", "Season");
	case EMGLeaderboardType::Crew:
		return NSLOCTEXT("Leaderboard", "Crew", "Crew");
	default:
		return FText::GetEmpty();
	}
}

FText UMGLeaderboardSubsystem::FormatTime(float TimeSeconds)
{
	int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeSeconds, 60.0f);

	return FText::FromString(FString::Printf(TEXT("%d:%06.3f"), Minutes, Seconds));
}

FText UMGLeaderboardSubsystem::FormatLargeNumber(int64 Number)
{
	if (Number >= 1000000000)
	{
		return FText::Format(NSLOCTEXT("Leaderboard", "Billions", "{0}B"), FText::AsNumber(Number / 1000000000));
	}
	else if (Number >= 1000000)
	{
		return FText::Format(NSLOCTEXT("Leaderboard", "Millions", "{0}M"), FText::AsNumber(Number / 1000000));
	}
	else if (Number >= 1000)
	{
		return FText::Format(NSLOCTEXT("Leaderboard", "Thousands", "{0}K"), FText::AsNumber(Number / 1000));
	}
	else
	{
		return FText::AsNumber(Number);
	}
}

EMGRankTier UMGLeaderboardSubsystem::GetTierFromRating(int32 Rating)
{
	if (Rating >= 2500) return EMGRankTier::Legend;
	if (Rating >= 2200) return EMGRankTier::Champion;
	if (Rating >= 1900) return EMGRankTier::Diamond;
	if (Rating >= 1600) return EMGRankTier::Platinum;
	if (Rating >= 1300) return EMGRankTier::Gold;
	if (Rating >= 1000) return EMGRankTier::Silver;
	if (Rating >= 700) return EMGRankTier::Bronze;
	return EMGRankTier::Unranked;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGLeaderboardSubsystem::InitializeData()
{
	// Initialize current season
	CurrentSeason.SeasonID = TEXT("Season_001");
	CurrentSeason.SeasonName = NSLOCTEXT("Leaderboard", "Season1", "Season 1: Origins");
	CurrentSeason.StartDate = FDateTime::UtcNow() - FTimespan::FromDays(30);
	CurrentSeason.EndDate = FDateTime::UtcNow() + FTimespan::FromDays(60);
	CurrentSeason.bIsActive = true;

	// Initialize player ranked data
	PlayerRankedData.bIsInPlacement = true;
	PlayerRankedData.PlacementMatchesCompleted = 0;
	PlayerRankedData.PlacementMatchesRequired = 10;
	PlayerRankedData.PeakRank = INT32_MAX;

	// Initialize weekly competitions
	{
		FMGWeeklyCompetition Competition;
		Competition.CompetitionID = TEXT("Weekly_TimeAttack");
		Competition.DisplayName = NSLOCTEXT("Leaderboard", "WeeklyTimeAttack", "Weekly Time Attack");
		Competition.Description = NSLOCTEXT("Leaderboard", "WeeklyTimeAttackDesc", "Set the fastest lap on the featured track!");
		Competition.TrackID = TEXT("Downtown");
		Competition.StartTime = FDateTime::UtcNow() - FTimespan::FromDays(3);
		Competition.EndTime = FDateTime::UtcNow() + FTimespan::FromDays(4);
		Competition.TotalParticipants = 15432;
		Competition.RewardThresholds = { 1, 10, 100, 1000 };

		WeeklyCompetitions.Add(Competition);
	}

	// Initialize mock track records
	TrackRecordsCache.Add(TEXT("Downtown"), CreateMockTrackRecord(TEXT("Downtown"), NSLOCTEXT("Track", "Downtown", "Downtown")));
	TrackRecordsCache.Add(TEXT("Highway"), CreateMockTrackRecord(TEXT("Highway"), NSLOCTEXT("Track", "Highway", "Highway")));
	TrackRecordsCache.Add(TEXT("Industrial"), CreateMockTrackRecord(TEXT("Industrial"), NSLOCTEXT("Track", "Industrial", "Industrial")));

	// Initialize player statistics
	PlayerStatistics.Add(EMGLeaderboardType::Wins, 47);
	PlayerStatistics.Add(EMGLeaderboardType::TotalXP, 125000);
	PlayerStatistics.Add(EMGLeaderboardType::Reputation, 8500);
	PlayerStatistics.Add(EMGLeaderboardType::Distance, 1250000); // meters
	PlayerStatistics.Add(EMGLeaderboardType::DriftScore, 450000);
	PlayerStatistics.Add(EMGLeaderboardType::CleanLaps, 156);

	TotalRacesCompleted = 203;
	TotalWins = 47;
	TotalPositionSum = 612;
}

TArray<FMGLeaderboardEntry> UMGLeaderboardSubsystem::CreateMockLeaderboard(EMGLeaderboardType Type, int32 Count)
{
	TArray<FMGLeaderboardEntry> Entries;

	TArray<FString> MockNames = {
		TEXT("SpeedDemon99"), TEXT("NightRacer"), TEXT("DriftKing"), TEXT("TurboMax"),
		TEXT("StreetLegend"), TEXT("GhostRider"), TEXT("NeonNinja"), TEXT("RoadWarrior"),
		TEXT("MidnightRunner"), TEXT("AsphaltAce"), TEXT("NitroNova"), TEXT("VelocityViper")
	};

	TArray<FString> MockTags = { TEXT("SRT"), TEXT("NR"), TEXT("DK"), TEXT("TBM"), TEXT("SL"), TEXT("GHR") };

	for (int32 i = 0; i < Count; i++)
	{
		FMGLeaderboardEntry Entry;
		Entry.Rank = i + 1;
		Entry.PreviousRank = Entry.Rank + FMath::RandRange(-5, 5);
		Entry.PlayerID = FString::Printf(TEXT("Player_%d"), i);
		Entry.DisplayName = FText::FromString(MockNames[i % MockNames.Num()] + FString::FromInt(FMath::RandRange(1, 99)));
		Entry.CountryCode = i % 3 == 0 ? TEXT("US") : (i % 3 == 1 ? TEXT("JP") : TEXT("DE"));
		Entry.Level = FMath::RandRange(10, 100);
		Entry.RankTier = GetTierFromRating(2500 - (i * 50));
		Entry.CrewTag = MockTags[i % MockTags.Num()];
		Entry.bIsOnline = FMath::RandBool();

		// Score based on type
		switch (Type)
		{
		case EMGLeaderboardType::LapTime:
			{
				float LapTime = 60.0f + (i * 0.3f);
				Entry.Score = static_cast<int64>(LapTime * 1000);
				Entry.FormattedScore = FormatTime(LapTime);
			}
			break;

		case EMGLeaderboardType::Wins:
			Entry.Score = FMath::Max(1, 500 - (i * 5));
			Entry.FormattedScore = FText::AsNumber(Entry.Score);
			break;

		case EMGLeaderboardType::TotalXP:
			Entry.Score = FMath::Max(1000, 10000000 - (i * 100000));
			Entry.FormattedScore = FormatLargeNumber(Entry.Score);
			break;

		case EMGLeaderboardType::Reputation:
			Entry.Score = FMath::Max(100, 100000 - (i * 1000));
			Entry.FormattedScore = FormatLargeNumber(Entry.Score);
			break;

		default:
			Entry.Score = FMath::Max(1, 10000 - (i * 100));
			Entry.FormattedScore = FText::AsNumber(Entry.Score);
			break;
		}

		Entry.DateAchieved = FDateTime::UtcNow() - FTimespan::FromDays(FMath::RandRange(0, 30));

		Entries.Add(Entry);
	}

	return Entries;
}

FMGTrackRecord UMGLeaderboardSubsystem::CreateMockTrackRecord(FName TrackID, FText TrackName)
{
	FMGTrackRecord Record;
	Record.TrackID = TrackID;
	Record.TrackName = TrackName;

	// World record
	Record.WorldRecord.Rank = 1;
	Record.WorldRecord.DisplayName = NSLOCTEXT("Leaderboard", "WorldRecordHolder", "xX_SpeedKing_Xx");
	float WorldTime = 58.123f;
	Record.WorldRecord.Score = static_cast<int64>(WorldTime * 1000);
	Record.WorldRecord.FormattedScore = FormatTime(WorldTime);
	Record.WorldRecord.RankTier = EMGRankTier::Legend;
	Record.WorldRecord.VehicleID = TEXT("Supercar_01");
	Record.WorldRecord.DateAchieved = FDateTime::UtcNow() - FTimespan::FromDays(5);
	Record.WorldRecord.bHasGhost = true;

	// Regional record
	Record.RegionalRecord = Record.WorldRecord;
	Record.RegionalRecord.DisplayName = NSLOCTEXT("Leaderboard", "RegionalRecordHolder", "LocalHero");
	float RegionalTime = 59.456f;
	Record.RegionalRecord.Score = static_cast<int64>(RegionalTime * 1000);
	Record.RegionalRecord.FormattedScore = FormatTime(RegionalTime);

	// Friends record
	Record.FriendsRecord.Rank = 1;
	Record.FriendsRecord.DisplayName = NSLOCTEXT("Leaderboard", "FriendRecordHolder", "BestFriend");
	float FriendsTime = 62.789f;
	Record.FriendsRecord.Score = static_cast<int64>(FriendsTime * 1000);
	Record.FriendsRecord.FormattedScore = FormatTime(FriendsTime);
	Record.FriendsRecord.bIsFriend = true;
	Record.FriendsRecord.RankTier = EMGRankTier::Platinum;

	Record.bHasGhost = true;

	return Record;
}

void UMGLeaderboardSubsystem::UpdateTierFromRating()
{
	PlayerRankedData.CurrentTier = GetTierFromRating(PlayerRankedData.RatingPoints);

	// Calculate division (1-4 within tier)
	int32 TierBase = 0;
	int32 TierRange = 300;

	switch (PlayerRankedData.CurrentTier)
	{
	case EMGRankTier::Bronze: TierBase = 700; break;
	case EMGRankTier::Silver: TierBase = 1000; break;
	case EMGRankTier::Gold: TierBase = 1300; break;
	case EMGRankTier::Platinum: TierBase = 1600; break;
	case EMGRankTier::Diamond: TierBase = 1900; break;
	case EMGRankTier::Champion: TierBase = 2200; break;
	case EMGRankTier::Legend: TierBase = 2500; TierRange = 500; break;
	default: TierBase = 0; break;
	}

	int32 PointsInTier = PlayerRankedData.RatingPoints - TierBase;
	int32 DivisionSize = TierRange / 4;
	PlayerRankedData.Division = 4 - FMath::Clamp(PointsInTier / DivisionSize, 0, 3);
	PlayerRankedData.PointsToNextDivision = (DivisionSize * (4 - PlayerRankedData.Division + 1)) - PointsInTier;
}

int32 UMGLeaderboardSubsystem::CalculateRatingChange(int32 Position, int32 TotalRacers, int32 CurrentRating) const
{
	// Expected position based on rating (simplified)
	float ExpectedPosition = TotalRacers / 2.0f;

	// Performance factor (-1 to 1)
	float Performance = (ExpectedPosition - Position) / (TotalRacers / 2.0f);

	// Base change
	int32 BaseChange = 25;

	// K-factor adjustment based on rating
	float KFactor = 1.0f;
	if (CurrentRating < 1000) KFactor = 1.5f;
	else if (CurrentRating > 2000) KFactor = 0.75f;

	// Win streak bonus
	float StreakBonus = 1.0f + (PlayerRankedData.WinStreak * 0.1f);
	if (Position > 1) StreakBonus = 1.0f;

	int32 Change = static_cast<int32>(BaseChange * Performance * KFactor * StreakBonus);

	// Minimum change
	if (Position == 1) Change = FMath::Max(Change, 10);
	if (Position == TotalRacers) Change = FMath::Min(Change, -10);

	return Change;
}

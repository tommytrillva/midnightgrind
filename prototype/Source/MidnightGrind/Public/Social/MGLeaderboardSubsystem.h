// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLeaderboardSubsystem.generated.h"

class UTexture2D;

/**
 * Leaderboard type
 */
UENUM(BlueprintType)
enum class EMGLeaderboardType : uint8
{
	/** Lap time leaderboard */
	LapTime,
	/** Race time leaderboard */
	RaceTime,
	/** Win count */
	Wins,
	/** Total XP */
	TotalXP,
	/** Reputation */
	Reputation,
	/** Distance driven */
	Distance,
	/** Drift score */
	DriftScore,
	/** Clean laps */
	CleanLaps,
	/** Weekly score */
	Weekly,
	/** Season ranking */
	Season,
	/** Crew ranking */
	Crew
};

/**
 * Leaderboard scope
 */
UENUM(BlueprintType)
enum class EMGLeaderboardScope : uint8
{
	/** Global - all players */
	Global,
	/** Regional */
	Regional,
	/** Friends only */
	Friends,
	/** Crew only */
	Crew,
	/** Near me (surrounding ranks) */
	NearMe
};

/**
 * Time filter for leaderboards
 */
UENUM(BlueprintType)
enum class EMGLeaderboardTimeFilter : uint8
{
	/** All time */
	AllTime,
	/** This season */
	Season,
	/** This month */
	Monthly,
	/** This week */
	Weekly,
	/** Today */
	Daily
};

/**
 * Rank tier
 */
UENUM(BlueprintType)
enum class EMGRankTier : uint8
{
	/** Unranked */
	Unranked,
	/** Bronze */
	Bronze,
	/** Silver */
	Silver,
	/** Gold */
	Gold,
	/** Platinum */
	Platinum,
	/** Diamond */
	Diamond,
	/** Champion */
	Champion,
	/** Legend */
	Legend
};

/**
 * Leaderboard entry
 */
USTRUCT(BlueprintType)
struct FMGLeaderboardEntry
{
	GENERATED_BODY()

	/** Rank position */
	UPROPERTY(BlueprintReadOnly)
	int32 Rank = 0;

	/** Previous rank (for movement) */
	UPROPERTY(BlueprintReadOnly)
	int32 PreviousRank = 0;

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Avatar */
	UPROPERTY(BlueprintReadOnly)
	UTexture2D* Avatar = nullptr;

	/** Country code */
	UPROPERTY(BlueprintReadOnly)
	FString CountryCode;

	/** Player level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Rank tier */
	UPROPERTY(BlueprintReadOnly)
	EMGRankTier RankTier = EMGRankTier::Unranked;

	/** Score/value */
	UPROPERTY(BlueprintReadOnly)
	int64 Score = 0;

	/** Formatted score */
	UPROPERTY(BlueprintReadOnly)
	FText FormattedScore;

	/** Secondary value (e.g. total races for lap time) */
	UPROPERTY(BlueprintReadOnly)
	int32 SecondaryValue = 0;

	/** Crew tag */
	UPROPERTY(BlueprintReadOnly)
	FString CrewTag;

	/** Crew name */
	UPROPERTY(BlueprintReadOnly)
	FText CrewName;

	/** Is local player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalPlayer = false;

	/** Is friend */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFriend = false;

	/** Is online */
	UPROPERTY(BlueprintReadOnly)
	bool bIsOnline = false;

	/** Vehicle used (for time records) */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Date achieved */
	UPROPERTY(BlueprintReadOnly)
	FDateTime DateAchieved;
};

/**
 * Track record
 */
USTRUCT(BlueprintType)
struct FMGTrackRecord
{
	GENERATED_BODY()

	/** Track ID */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Track name */
	UPROPERTY(BlueprintReadOnly)
	FText TrackName;

	/** World record holder */
	UPROPERTY(BlueprintReadOnly)
	FMGLeaderboardEntry WorldRecord;

	/** Regional record holder */
	UPROPERTY(BlueprintReadOnly)
	FMGLeaderboardEntry RegionalRecord;

	/** Friends record holder */
	UPROPERTY(BlueprintReadOnly)
	FMGLeaderboardEntry FriendsRecord;

	/** Personal best */
	UPROPERTY(BlueprintReadOnly)
	FMGLeaderboardEntry PersonalBest;

	/** Ghost available */
	UPROPERTY(BlueprintReadOnly)
	bool bHasGhost = false;
};

/**
 * Ranked season info
 */
USTRUCT(BlueprintType)
struct FMGRankedSeason
{
	GENERATED_BODY()

	/** Season ID */
	UPROPERTY(BlueprintReadOnly)
	FString SeasonID;

	/** Season name */
	UPROPERTY(BlueprintReadOnly)
	FText SeasonName;

	/** Start date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime StartDate;

	/** End date */
	UPROPERTY(BlueprintReadOnly)
	FDateTime EndDate;

	/** Is active */
	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;
};

/**
 * Player ranked data
 */
USTRUCT(BlueprintType)
struct FMGPlayerRankedData
{
	GENERATED_BODY()

	/** Current tier */
	UPROPERTY(BlueprintReadOnly)
	EMGRankTier CurrentTier = EMGRankTier::Unranked;

	/** Division within tier (1-4, 1 is highest) */
	UPROPERTY(BlueprintReadOnly)
	int32 Division = 1;

	/** Current rating points */
	UPROPERTY(BlueprintReadOnly)
	int32 RatingPoints = 0;

	/** Points to next division */
	UPROPERTY(BlueprintReadOnly)
	int32 PointsToNextDivision = 100;

	/** Current rank */
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentRank = 0;

	/** Peak rank this season */
	UPROPERTY(BlueprintReadOnly)
	int32 PeakRank = 0;

	/** Peak tier this season */
	UPROPERTY(BlueprintReadOnly)
	EMGRankTier PeakTier = EMGRankTier::Unranked;

	/** Wins this season */
	UPROPERTY(BlueprintReadOnly)
	int32 SeasonWins = 0;

	/** Losses this season */
	UPROPERTY(BlueprintReadOnly)
	int32 SeasonLosses = 0;

	/** Win streak */
	UPROPERTY(BlueprintReadOnly)
	int32 WinStreak = 0;

	/** Best win streak */
	UPROPERTY(BlueprintReadOnly)
	int32 BestWinStreak = 0;

	/** Placement matches completed */
	UPROPERTY(BlueprintReadOnly)
	int32 PlacementMatchesCompleted = 0;

	/** Placement matches required */
	UPROPERTY(BlueprintReadOnly)
	int32 PlacementMatchesRequired = 10;

	/** Is in placement */
	UPROPERTY(BlueprintReadOnly)
	bool bIsInPlacement = true;
};

/**
 * Weekly competition
 */
USTRUCT(BlueprintType)
struct FMGWeeklyCompetition
{
	GENERATED_BODY()

	/** Competition ID */
	UPROPERTY(BlueprintReadOnly)
	FString CompetitionID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Description */
	UPROPERTY(BlueprintReadOnly)
	FText Description;

	/** Track ID */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Weather requirement */
	UPROPERTY(BlueprintReadOnly)
	FName WeatherRequirement;

	/** Vehicle restriction */
	UPROPERTY(BlueprintReadOnly)
	TArray<FName> AllowedVehicles;

	/** Start time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime StartTime;

	/** End time */
	UPROPERTY(BlueprintReadOnly)
	FDateTime EndTime;

	/** Total participants */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalParticipants = 0;

	/** Player's best score */
	UPROPERTY(BlueprintReadOnly)
	int64 PlayerBestScore = 0;

	/** Player's current rank */
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerRank = 0;

	/** Reward tiers */
	UPROPERTY(BlueprintReadOnly)
	TArray<int32> RewardThresholds;

	/** Has participated */
	UPROPERTY(BlueprintReadOnly)
	bool bHasParticipated = false;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLeaderboardLoaded, EMGLeaderboardType, Type, const TArray<FMGLeaderboardEntry>&, Entries);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewPersonalBest, EMGLeaderboardType, Type, int64, OldScore, int64, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRankChanged, int32, OldRank, int32, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierChanged, EMGRankTier, OldTier, EMGRankTier, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTrackRecordBroken, const FMGTrackRecord&, Record);

/**
 * Leaderboard Subsystem
 * Manages global, regional, and friend leaderboards
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLeaderboardSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLeaderboardLoaded OnLeaderboardLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRankChanged OnRankChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTierChanged OnTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTrackRecordBroken OnTrackRecordBroken;

	// ==========================================
	// LEADERBOARDS
	// ==========================================

	/** Get leaderboard entries */
	UFUNCTION(BlueprintCallable, Category = "Leaderboards")
	void GetLeaderboard(EMGLeaderboardType Type, EMGLeaderboardScope Scope, EMGLeaderboardTimeFilter TimeFilter, int32 StartRank = 1, int32 Count = 100);

	/** Get track leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Leaderboards")
	void GetTrackLeaderboard(FName TrackID, EMGLeaderboardScope Scope, int32 StartRank = 1, int32 Count = 100);

	/** Get player's position on leaderboard */
	UFUNCTION(BlueprintPure, Category = "Leaderboards")
	int32 GetPlayerLeaderboardPosition(EMGLeaderboardType Type, EMGLeaderboardScope Scope) const;

	/** Get entries around player */
	UFUNCTION(BlueprintCallable, Category = "Leaderboards")
	TArray<FMGLeaderboardEntry> GetEntriesAroundPlayer(EMGLeaderboardType Type, int32 Range = 5);

	// ==========================================
	// TRACK RECORDS
	// ==========================================

	/** Get track records */
	UFUNCTION(BlueprintPure, Category = "Records")
	FMGTrackRecord GetTrackRecords(FName TrackID) const;

	/** Get all track records */
	UFUNCTION(BlueprintPure, Category = "Records")
	TArray<FMGTrackRecord> GetAllTrackRecords() const;

	/** Submit lap time */
	UFUNCTION(BlueprintCallable, Category = "Records")
	void SubmitLapTime(FName TrackID, float LapTime, FName VehicleID, const TArray<uint8>& GhostData);

	/** Submit race time */
	UFUNCTION(BlueprintCallable, Category = "Records")
	void SubmitRaceTime(FName TrackID, float RaceTime, int32 LapCount, FName VehicleID);

	/** Get ghost data for record */
	UFUNCTION(BlueprintCallable, Category = "Records")
	TArray<uint8> GetRecordGhostData(FName TrackID, EMGLeaderboardScope Scope);

	// ==========================================
	// RANKED
	// ==========================================

	/** Get player ranked data */
	UFUNCTION(BlueprintPure, Category = "Ranked")
	FMGPlayerRankedData GetPlayerRankedData() const { return PlayerRankedData; }

	/** Get current season */
	UFUNCTION(BlueprintPure, Category = "Ranked")
	FMGRankedSeason GetCurrentSeason() const { return CurrentSeason; }

	/** Submit ranked match result */
	UFUNCTION(BlueprintCallable, Category = "Ranked")
	void SubmitRankedResult(int32 Position, int32 TotalRacers);

	/** Get rating change preview */
	UFUNCTION(BlueprintPure, Category = "Ranked")
	int32 GetRatingChangePreview(int32 ExpectedPosition, int32 TotalRacers) const;

	// ==========================================
	// WEEKLY COMPETITIONS
	// ==========================================

	/** Get active weekly competitions */
	UFUNCTION(BlueprintPure, Category = "Weekly")
	TArray<FMGWeeklyCompetition> GetActiveWeeklyCompetitions() const { return WeeklyCompetitions; }

	/** Get weekly competition leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Weekly")
	void GetWeeklyCompetitionLeaderboard(const FString& CompetitionID, int32 StartRank = 1, int32 Count = 100);

	/** Submit weekly competition score */
	UFUNCTION(BlueprintCallable, Category = "Weekly")
	void SubmitWeeklyScore(const FString& CompetitionID, int64 Score);

	// ==========================================
	// STATISTICS
	// ==========================================

	/** Get player statistics */
	UFUNCTION(BlueprintPure, Category = "Statistics")
	int64 GetPlayerStatistic(EMGLeaderboardType Type) const;

	/** Get win rate */
	UFUNCTION(BlueprintPure, Category = "Statistics")
	float GetWinRate() const;

	/** Get average finish position */
	UFUNCTION(BlueprintPure, Category = "Statistics")
	float GetAverageFinishPosition() const;

	/** Get total races completed */
	UFUNCTION(BlueprintPure, Category = "Statistics")
	int32 GetTotalRacesCompleted() const { return TotalRacesCompleted; }

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get tier display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetTierDisplayName(EMGRankTier Tier);

	/** Get tier color */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FLinearColor GetTierColor(EMGRankTier Tier);

	/** Get leaderboard type display name */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText GetLeaderboardTypeDisplayName(EMGLeaderboardType Type);

	/** Format time for display */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatTime(float TimeSeconds);

	/** Format large number */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static FText FormatLargeNumber(int64 Number);

	/** Get tier from rating points */
	UFUNCTION(BlueprintPure, Category = "Utility")
	static EMGRankTier GetTierFromRating(int32 Rating);

protected:
	/** Player ranked data */
	UPROPERTY()
	FMGPlayerRankedData PlayerRankedData;

	/** Current season */
	UPROPERTY()
	FMGRankedSeason CurrentSeason;

	/** Weekly competitions */
	UPROPERTY()
	TArray<FMGWeeklyCompetition> WeeklyCompetitions;

	/** Track records cache */
	UPROPERTY()
	TMap<FName, FMGTrackRecord> TrackRecordsCache;

	/** Cached leaderboards */
	UPROPERTY()
	TMap<FName, TArray<FMGLeaderboardEntry>> LeaderboardCache;

	/** Player statistics */
	TMap<EMGLeaderboardType, int64> PlayerStatistics;

	/** Total races */
	int32 TotalRacesCompleted = 0;

	/** Total wins */
	int32 TotalWins = 0;

	/** Total finish position sum (for average) */
	int32 TotalPositionSum = 0;

	/** Initialize data */
	void InitializeData();

	/** Create mock leaderboard data */
	TArray<FMGLeaderboardEntry> CreateMockLeaderboard(EMGLeaderboardType Type, int32 Count);

	/** Create mock track record */
	FMGTrackRecord CreateMockTrackRecord(FName TrackID, FText TrackName);

	/** Update tier from rating */
	void UpdateTierFromRating();

	/** Calculate rating change */
	int32 CalculateRatingChange(int32 Position, int32 TotalRacers, int32 CurrentRating) const;
};

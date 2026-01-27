// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLeaderboardSubsystem.generated.h"

/**
 * Leaderboard type
 */
UENUM(BlueprintType)
enum class EMGLeaderboardType : uint8
{
	/** Best lap time on track */
	LapTime,
	/** Best race time on track */
	RaceTime,
	/** Overall reputation */
	Reputation,
	/** Total wins */
	Wins,
	/** Win streak */
	WinStreak,
	/** Drift score */
	DriftScore,
	/** Perfect starts */
	PerfectStarts,
	/** Weekly challenge */
	WeeklyChallenge,
	/** Season ranking */
	SeasonRanking
};

/**
 * Leaderboard scope
 */
UENUM(BlueprintType)
enum class EMGLeaderboardScope : uint8
{
	/** Global rankings */
	Global,
	/** Friends only */
	Friends,
	/** Crew members */
	Crew,
	/** Regional */
	Regional,
	/** Weekly */
	Weekly,
	/** Daily */
	Daily
};

/**
 * Leaderboard entry
 */
USTRUCT(BlueprintType)
struct FMGLeaderboardEntry
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Player display name */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	/** Rank position */
	UPROPERTY(BlueprintReadOnly)
	int32 Rank = 0;

	/** Score/time value */
	UPROPERTY(BlueprintReadOnly)
	float Score = 0.0f;

	/** Vehicle used (for time-based) */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleID;

	/** Timestamp of entry */
	UPROPERTY(BlueprintReadOnly)
	FDateTime Timestamp;

	/** Player's crew name */
	UPROPERTY(BlueprintReadOnly)
	FString CrewName;

	/** Player's region */
	UPROPERTY(BlueprintReadOnly)
	FString Region;

	/** Is this the local player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocalPlayer = false;

	/** Is friend */
	UPROPERTY(BlueprintReadOnly)
	bool bIsFriend = false;

	/** Has ghost data available */
	UPROPERTY(BlueprintReadOnly)
	bool bHasGhost = false;

	/** Ghost replay ID */
	UPROPERTY(BlueprintReadOnly)
	FString GhostReplayID;
};

/**
 * Leaderboard query request
 */
USTRUCT(BlueprintType)
struct FMGLeaderboardQuery
{
	GENERATED_BODY()

	/** Leaderboard type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLeaderboardType Type = EMGLeaderboardType::LapTime;

	/** Scope */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLeaderboardScope Scope = EMGLeaderboardScope::Global;

	/** Track ID (for track-specific boards) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Start rank (for pagination) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartRank = 1;

	/** Max entries to fetch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxEntries = 50;

	/** Include entries around player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAroundPlayer = false;
};

/**
 * Leaderboard result
 */
USTRUCT(BlueprintType)
struct FMGLeaderboardResult
{
	GENERATED_BODY()

	/** Query that produced this result */
	UPROPERTY(BlueprintReadOnly)
	FMGLeaderboardQuery Query;

	/** Entries */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGLeaderboardEntry> Entries;

	/** Total entries in leaderboard */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalEntries = 0;

	/** Local player's rank */
	UPROPERTY(BlueprintReadOnly)
	int32 LocalPlayerRank = 0;

	/** Local player's score */
	UPROPERTY(BlueprintReadOnly)
	float LocalPlayerScore = 0.0f;

	/** Was query successful */
	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	/** Error message (if failed) */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;
};

/**
 * Personal best record
 */
USTRUCT(BlueprintType)
struct FMGPersonalBest
{
	GENERATED_BODY()

	/** Track ID */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Best lap time */
	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	/** Best race time */
	UPROPERTY(BlueprintReadOnly)
	float BestRaceTime = 0.0f;

	/** Vehicle used for lap record */
	UPROPERTY(BlueprintReadOnly)
	FName LapRecordVehicle;

	/** Vehicle used for race record */
	UPROPERTY(BlueprintReadOnly)
	FName RaceRecordVehicle;

	/** Global rank for lap time */
	UPROPERTY(BlueprintReadOnly)
	int32 LapTimeRank = 0;

	/** Global rank for race time */
	UPROPERTY(BlueprintReadOnly)
	int32 RaceTimeRank = 0;

	/** Date of lap record */
	UPROPERTY(BlueprintReadOnly)
	FDateTime LapRecordDate;

	/** Date of race record */
	UPROPERTY(BlueprintReadOnly)
	FDateTime RaceRecordDate;

	/** Has associated ghost */
	UPROPERTY(BlueprintReadOnly)
	bool bHasGhost = false;
};

/**
 * Score submission result
 */
USTRUCT(BlueprintType)
struct FMGScoreSubmissionResult
{
	GENERATED_BODY()

	/** Was submission successful */
	UPROPERTY(BlueprintReadOnly)
	bool bSuccess = false;

	/** Is new personal best */
	UPROPERTY(BlueprintReadOnly)
	bool bIsPersonalBest = false;

	/** Old personal best */
	UPROPERTY(BlueprintReadOnly)
	float OldPersonalBest = 0.0f;

	/** New rank achieved */
	UPROPERTY(BlueprintReadOnly)
	int32 NewRank = 0;

	/** Old rank */
	UPROPERTY(BlueprintReadOnly)
	int32 OldRank = 0;

	/** Rank improvement */
	UPROPERTY(BlueprintReadOnly)
	int32 RankImprovement = 0;

	/** Time/score improvement */
	UPROPERTY(BlueprintReadOnly)
	float ScoreImprovement = 0.0f;

	/** Error message */
	UPROPERTY(BlueprintReadOnly)
	FString ErrorMessage;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeaderboardQueryComplete, const FMGLeaderboardResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreSubmissionComplete, const FMGScoreSubmissionResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPersonalBestUpdated, FName, TrackID, const FMGPersonalBest&, NewRecord);

/**
 * Leaderboard Subsystem
 * Manages leaderboard queries and submissions
 *
 * Features:
 * - Multiple leaderboard types
 * - Scope filtering (global, friends, crew)
 * - Personal best tracking
 * - Ghost data integration
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
	FOnLeaderboardQueryComplete OnLeaderboardQueryComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnScoreSubmissionComplete OnScoreSubmissionComplete;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPersonalBestUpdated OnPersonalBestUpdated;

	// ==========================================
	// QUERIES
	// ==========================================

	/** Query leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void QueryLeaderboard(const FMGLeaderboardQuery& Query);

	/** Query top entries */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void QueryTopEntries(EMGLeaderboardType Type, FName TrackID, int32 Count = 10);

	/** Query around player */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void QueryAroundPlayer(EMGLeaderboardType Type, FName TrackID, int32 Range = 5);

	/** Query friends leaderboard */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard")
	void QueryFriendsLeaderboard(EMGLeaderboardType Type, FName TrackID);

	/** Get cached result */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	FMGLeaderboardResult GetCachedResult(EMGLeaderboardType Type, FName TrackID) const;

	/** Is query in progress */
	UFUNCTION(BlueprintPure, Category = "Leaderboard")
	bool IsQueryInProgress() const { return bQueryInProgress; }

	// ==========================================
	// SUBMISSIONS
	// ==========================================

	/** Submit lap time */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard|Submit")
	void SubmitLapTime(FName TrackID, float LapTime, FName VehicleID, const FString& GhostReplayID = TEXT(""));

	/** Submit race time */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard|Submit")
	void SubmitRaceTime(FName TrackID, float RaceTime, FName VehicleID, int32 Position);

	/** Submit drift score */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard|Submit")
	void SubmitDriftScore(FName TrackID, int32 DriftScore);

	/** Submit general score */
	UFUNCTION(BlueprintCallable, Category = "Leaderboard|Submit")
	void SubmitScore(EMGLeaderboardType Type, float Score, FName TrackID = NAME_None, FName VehicleID = NAME_None);

	// ==========================================
	// PERSONAL BESTS
	// ==========================================

	/** Get personal best for track */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Personal")
	FMGPersonalBest GetPersonalBest(FName TrackID) const;

	/** Get all personal bests */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Personal")
	TArray<FMGPersonalBest> GetAllPersonalBests() const;

	/** Get track medal (based on rank) */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Personal")
	int32 GetTrackMedal(FName TrackID) const;

	/** Has personal best for track */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Personal")
	bool HasPersonalBest(FName TrackID) const;

	// ==========================================
	// PLAYER STATS
	// ==========================================

	/** Get player's global rank for type */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Stats")
	int32 GetGlobalRank(EMGLeaderboardType Type) const;

	/** Get total leaderboard entries count */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Stats")
	int32 GetTotalEntriesCount(EMGLeaderboardType Type, FName TrackID) const;

	/** Get percentile rank */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Stats")
	float GetPercentileRank(EMGLeaderboardType Type, FName TrackID) const;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Format time for display */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Utility")
	static FString FormatTime(float TimeSeconds);

	/** Format rank for display */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Utility")
	static FString FormatRank(int32 Rank);

	/** Get leaderboard display name */
	UFUNCTION(BlueprintPure, Category = "Leaderboard|Utility")
	static FText GetLeaderboardDisplayName(EMGLeaderboardType Type);

protected:
	// ==========================================
	// CACHE
	// ==========================================

	/** Cached query results */
	UPROPERTY()
	TMap<FString, FMGLeaderboardResult> CachedResults;

	/** Personal bests */
	UPROPERTY()
	TMap<FName, FMGPersonalBest> PersonalBests;

	/** Cache timeout (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	float CacheTimeout = 60.0f;

	/** Query in progress flag */
	bool bQueryInProgress = false;

	/** Pending query */
	FMGLeaderboardQuery PendingQuery;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Get cache key for query */
	FString GetCacheKey(EMGLeaderboardType Type, EMGLeaderboardScope Scope, FName TrackID) const;

	/** Process query result (simulated) */
	void ProcessQueryResult(const FMGLeaderboardQuery& Query);

	/** Process submission result (simulated) */
	void ProcessSubmissionResult(EMGLeaderboardType Type, float Score, FName TrackID, FName VehicleID, const FString& GhostID);

	/** Update personal best */
	void UpdatePersonalBest(FName TrackID, EMGLeaderboardType Type, float Score, FName VehicleID);

	/** Generate mock leaderboard data */
	TArray<FMGLeaderboardEntry> GenerateMockEntries(const FMGLeaderboardQuery& Query) const;

	/** Load personal bests */
	void LoadPersonalBests();

	/** Save personal bests */
	void SavePersonalBests();
};

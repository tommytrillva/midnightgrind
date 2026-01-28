// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGRaceHistorySubsystem.h
 * @brief Race History Subsystem - Tracks and persists all race results and player statistics
 *
 * This subsystem maintains a comprehensive history of all races completed by the player,
 * providing detailed statistics, personal bests, and performance analytics. It persists
 * across game sessions and serves as the foundation for leaderboards, achievements, and
 * progression tracking.
 *
 * ============================================================================
 * KEY CONCEPTS FOR BEGINNERS
 * ============================================================================
 *
 * WHAT DOES THIS SUBSYSTEM DO?
 * Every time you finish a race, the Race History Subsystem:
 * 1. Records all the details (position, time, vehicle, etc.)
 * 2. Updates your lifetime statistics
 * 3. Checks for new personal bests
 * 4. Updates win/podium streaks
 * 5. Saves everything to disk
 *
 * WHY IS THIS IMPORTANT?
 * - Shows your progress over time
 * - Tracks personal bests for each track
 * - Maintains statistics for achievements
 * - Provides data for in-game "career" screens
 * - Enables comparing performance across vehicles
 *
 * DATA STRUCTURES:
 * @see FMGRaceResult - Individual race record (one entry per race)
 * @see FMGTrackStats - Aggregate stats for a specific track
 * @see FMGVehicleRaceStats - Aggregate stats for a specific vehicle
 * @see FMGLifetimeStats - Overall career statistics
 *
 * ============================================================================
 * RACE RESULT DATA (FMGRaceResult)
 * ============================================================================
 *
 * Each race result captures:
 *
 * IDENTIFICATION:
 * - RaceId: Unique identifier for this race
 * - TrackId/TrackName: Which track was raced
 * - RaceType: Sprint, Circuit, Drift, etc.
 * - Timestamp: When the race occurred
 *
 * RESULTS:
 * - Position: Final finishing position (1 = first)
 * - TotalRacers: How many competitors
 * - RaceTime: Total completion time
 * - BestLapTime: Fastest lap (circuit races)
 * - LapTimes: Array of all lap times
 * - bWasCleanRace: No collisions/penalties
 * - bDNF: Did Not Finish
 *
 * PERFORMANCE:
 * - AverageSpeedKPH: Average speed maintained
 * - TopSpeedKPH: Maximum speed reached
 * - DistanceM: Total distance driven
 * - DefeatedRivals: List of beaten opponents
 *
 * VEHICLE:
 * - VehicleId/VehicleName: What car was used
 * - PerformanceIndex: PI at time of race
 *
 * REWARDS:
 * - CashEarned: GrindCash won
 * - ReputationEarned: Rep points gained
 * - XPEarned: Experience earned
 *
 * ============================================================================
 * LIFETIME STATISTICS (FMGLifetimeStats)
 * ============================================================================
 *
 * Aggregate career statistics include:
 *
 * RACE COUNTS:
 * - TotalRaces, TotalWins, TotalPodiums, TotalDNFs
 * - CleanRaces: Races without collisions
 *
 * STREAKS:
 * - CurrentWinStreak, BestWinStreak
 * - CurrentPodiumStreak, BestPodiumStreak
 *
 * TOTALS:
 * - TotalDistanceKM: Kilometers driven
 * - TotalRaceTimeHours: Time spent racing
 * - HighestTopSpeedKPH: Speed record
 *
 * EARNINGS:
 * - TotalCashEarned, TotalReputationEarned, TotalXPEarned
 *
 * ONLINE:
 * - OnlineRaces, OnlineWins
 *
 * CALCULATED RATIOS:
 * - GetWinRate(): Wins / Total races
 * - GetPodiumRate(): Podiums / Total races
 * - GetCleanRaceRate(): Clean races / Total races
 *
 * ============================================================================
 * USAGE EXAMPLE
 * ============================================================================
 *
 * @code
 * // Get the race history subsystem
 * UMGRaceHistorySubsystem* History = GetGameInstance()->GetSubsystem<UMGRaceHistorySubsystem>();
 *
 * // Record a race result (usually done automatically by RaceFlowSubsystem)
 * FMGRaceResult Result;
 * Result.TrackId = "Tokyo_Highway";
 * Result.TrackName = FText::FromString("Tokyo Highway");
 * Result.Position = 1;
 * Result.TotalRacers = 8;
 * Result.RaceTime = 245.5f;
 * Result.BestLapTime = 78.2f;
 * Result.VehicleId = FName("Nissan_GTR");
 * Result.CashEarned = 15000;
 * History->RecordRaceResult(Result);
 *
 * // Query statistics
 * FMGLifetimeStats Stats = History->GetLifetimeStats();
 * UE_LOG(LogTemp, Log, TEXT("Win Rate: %.1f%%"), Stats.GetWinRate() * 100.0f);
 *
 * // Get personal best for a track
 * float BestTime = History->GetPersonalBestTime("Tokyo_Highway");
 *
 * // Get recent results
 * TArray<FMGRaceResult> RecentRaces = History->GetRecentResults(10);
 *
 * // Get stats for a specific vehicle
 * FMGVehicleRaceStats GTRStats = History->GetVehicleStats(FName("Nissan_GTR"));
 * UE_LOG(LogTemp, Log, TEXT("GTR Wins: %d"), GTRStats.Wins);
 * @endcode
 *
 * ============================================================================
 * EVENTS
 * ============================================================================
 *
 * Subscribe to events for real-time updates:
 *
 * @code
 * // React to new race results
 * History->OnRaceResultRecorded.AddDynamic(this, &UMyWidget::HandleNewResult);
 *
 * // Show celebration for new personal bests
 * History->OnNewPersonalBest.AddDynamic(this, &UMyWidget::ShowPersonalBestAnimation);
 *
 * // Update win streak display
 * History->OnWinStreakUpdated.AddDynamic(this, &UMyWidget::UpdateStreakUI);
 * @endcode
 *
 * ============================================================================
 * PERSISTENCE
 * ============================================================================
 *
 * Race history is automatically saved to disk:
 * - SaveHistory(): Manually trigger a save
 * - LoadHistory(): Load from disk (called automatically on startup)
 * - ClearHistory(): Delete all history (with confirmation!)
 *
 * History is stored in the game's save directory as a JSON file.
 * Maximum entries are limited (default 500) to prevent unbounded growth.
 * Oldest entries are removed when the limit is exceeded.
 *
 * @note For online profiles, history is synced with the server via MGOnlineProfile
 *
 * @see UMGRaceFlowSubsystem for automatic result recording
 * @see UMGOnlineProfileSubsystem for cloud synchronization
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceHistorySubsystem.generated.h"

/**
 * Individual race result
 */
USTRUCT(BlueprintType)
struct FMGRaceResult
{
	GENERATED_BODY()

	/** Unique race ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FGuid RaceId;

	/** Track/Layout ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString TrackId;

	/** Track display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FText TrackName;

	/** Race type (Sprint, Circuit, Drift, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName RaceType;

	/** Finishing position (1 = first) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	int32 Position = 0;

	/** Total racers in event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	int32 TotalRacers = 0;

	/** Total race time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	float RaceTime = 0.0f;

	/** Best lap time (for circuit races) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	float BestLapTime = 0.0f;

	/** Average speed in KPH */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	float AverageSpeedKPH = 0.0f;

	/** Top speed reached in KPH */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	float TopSpeedKPH = 0.0f;

	/** Number of laps completed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	int32 LapsCompleted = 0;

	/** Total distance driven in meters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	float DistanceM = 0.0f;

	/** Was this a clean race (no collisions/penalties) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	bool bWasCleanRace = false;

	/** Did player DNF */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	bool bDNF = false;

	/** Vehicle used */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName VehicleId;

	/** Vehicle display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FText VehicleName;

	/** Performance index at time of race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	int32 PerformanceIndex = 0;

	/** GrindCash earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 CashEarned = 0;

	/** Reputation earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 ReputationEarned = 0;

	/** XP earned */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 XPEarned = 0;

	/** Rivals defeated (player IDs or AI names) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Results")
	TArray<FString> DefeatedRivals;

	/** When the race occurred */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FDateTime Timestamp;

	/** Individual lap times */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> LapTimes;

	/** Sector times for best lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> BestLapSectorTimes;

	/** Was this an online race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	bool bWasOnlineRace = false;

	/** Check if this is a win */
	bool IsWin() const { return Position == 1 && !bDNF; }

	/** Check if this is a podium */
	bool IsPodium() const { return Position <= 3 && !bDNF; }
};

/**
 * Track statistics aggregate
 */
USTRUCT(BlueprintType)
struct FMGTrackStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Podiums = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AveragePosition = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRaced;
};

/**
 * Vehicle statistics aggregate
 */
USTRUCT(BlueprintType)
struct FMGVehicleRaceStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Podiums = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDistanceKM = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TopSpeedRecordKPH = 0.0f;
};

/**
 * Global lifetime statistics
 */
USTRUCT(BlueprintType)
struct FMGLifetimeStats
{
	GENERATED_BODY()

	// Race counts
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPodiums = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalDNFs = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanRaces = 0;

	// Streaks
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentWinStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestWinStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPodiumStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestPodiumStreak = 0;

	// Distance and time
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TotalDistanceKM = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TotalRaceTimeHours = 0.0;

	// Speed records
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestTopSpeedKPH = 0.0f;

	// Earnings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalCashEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalReputationEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalXPEarned = 0;

	// Online
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OnlineRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OnlineWins = 0;

	// Ratios (calculated)
	float GetWinRate() const { return TotalRaces > 0 ? (float)TotalWins / TotalRaces : 0.0f; }
	float GetPodiumRate() const { return TotalRaces > 0 ? (float)TotalPodiums / TotalRaces : 0.0f; }
	float GetCleanRaceRate() const { return TotalRaces > 0 ? (float)CleanRaces / TotalRaces : 0.0f; }
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceResultRecorded, const FMGRaceResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewPersonalBest, const FString&, TrackId, float, NewBestTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWinStreakUpdated, int32, NewStreak);

/**
 * Race History Subsystem
 * Tracks and persists all race results and statistics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceHistorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// RECORDING RESULTS
	// ==========================================

	/** Record a completed race result */
	UFUNCTION(BlueprintCallable, Category = "Race History")
	void RecordRaceResult(const FMGRaceResult& Result);

	/** Create and record a result from basic parameters */
	UFUNCTION(BlueprintCallable, Category = "Race History")
	FMGRaceResult CreateAndRecordResult(
		const FString& TrackId,
		const FText& TrackName,
		FName RaceType,
		int32 Position,
		int32 TotalRacers,
		float RaceTime,
		float BestLapTime,
		FName VehicleId,
		const FText& VehicleName,
		bool bWasCleanRace
	);

	// ==========================================
	// QUERYING HISTORY
	// ==========================================

	/** Get all race results */
	UFUNCTION(BlueprintPure, Category = "Race History|Query")
	TArray<FMGRaceResult> GetAllResults() const { return RaceHistory; }

	/** Get recent results (limited count) */
	UFUNCTION(BlueprintPure, Category = "Race History|Query")
	TArray<FMGRaceResult> GetRecentResults(int32 Count = 10) const;

	/** Get results for a specific track */
	UFUNCTION(BlueprintPure, Category = "Race History|Query")
	TArray<FMGRaceResult> GetResultsForTrack(const FString& TrackId) const;

	/** Get results for a specific vehicle */
	UFUNCTION(BlueprintPure, Category = "Race History|Query")
	TArray<FMGRaceResult> GetResultsForVehicle(FName VehicleId) const;

	/** Get wins only */
	UFUNCTION(BlueprintPure, Category = "Race History|Query")
	TArray<FMGRaceResult> GetWins() const;

	/** Get a specific result by ID */
	UFUNCTION(BlueprintPure, Category = "Race History|Query")
	bool GetResultById(const FGuid& RaceId, FMGRaceResult& OutResult) const;

	// ==========================================
	// STATISTICS
	// ==========================================

	/** Get lifetime statistics */
	UFUNCTION(BlueprintPure, Category = "Race History|Stats")
	FMGLifetimeStats GetLifetimeStats() const { return LifetimeStats; }

	/** Get statistics for a specific track */
	UFUNCTION(BlueprintPure, Category = "Race History|Stats")
	FMGTrackStats GetTrackStats(const FString& TrackId) const;

	/** Get statistics for a specific vehicle */
	UFUNCTION(BlueprintPure, Category = "Race History|Stats")
	FMGVehicleRaceStats GetVehicleStats(FName VehicleId) const;

	/** Get most raced tracks */
	UFUNCTION(BlueprintPure, Category = "Race History|Stats")
	TArray<FMGTrackStats> GetMostRacedTracks(int32 Count = 5) const;

	/** Get most successful vehicles */
	UFUNCTION(BlueprintPure, Category = "Race History|Stats")
	TArray<FMGVehicleRaceStats> GetMostSuccessfulVehicles(int32 Count = 5) const;

	// ==========================================
	// PERSONAL BESTS
	// ==========================================

	/** Get personal best time for a track */
	UFUNCTION(BlueprintPure, Category = "Race History|Records")
	float GetPersonalBestTime(const FString& TrackId) const;

	/** Get personal best lap for a track */
	UFUNCTION(BlueprintPure, Category = "Race History|Records")
	float GetPersonalBestLap(const FString& TrackId) const;

	/** Check if a time beats the personal best */
	UFUNCTION(BlueprintPure, Category = "Race History|Records")
	bool IsNewPersonalBest(const FString& TrackId, float Time) const;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Race History|Events")
	FOnRaceResultRecorded OnRaceResultRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Race History|Events")
	FOnNewPersonalBest OnNewPersonalBest;

	UPROPERTY(BlueprintAssignable, Category = "Race History|Events")
	FOnWinStreakUpdated OnWinStreakUpdated;

	// ==========================================
	// PERSISTENCE
	// ==========================================

	/** Save history to disk */
	UFUNCTION(BlueprintCallable, Category = "Race History|Persistence")
	void SaveHistory();

	/** Load history from disk */
	UFUNCTION(BlueprintCallable, Category = "Race History|Persistence")
	void LoadHistory();

	/** Clear all history (with confirmation) */
	UFUNCTION(BlueprintCallable, Category = "Race History|Persistence")
	void ClearHistory();

protected:
	void UpdateLifetimeStats(const FMGRaceResult& Result);
	void UpdateTrackStats(const FMGRaceResult& Result);
	void UpdateVehicleStats(const FMGRaceResult& Result);
	void UpdateStreaks(const FMGRaceResult& Result);
	void CheckPersonalBests(const FMGRaceResult& Result);

private:
	/** All recorded race results */
	UPROPERTY()
	TArray<FMGRaceResult> RaceHistory;

	/** Lifetime aggregate statistics */
	UPROPERTY()
	FMGLifetimeStats LifetimeStats;

	/** Per-track statistics */
	UPROPERTY()
	TMap<FString, FMGTrackStats> TrackStatsMap;

	/** Per-vehicle statistics */
	UPROPERTY()
	TMap<FName, FMGVehicleRaceStats> VehicleStatsMap;

	/** Personal best times per track */
	UPROPERTY()
	TMap<FString, float> PersonalBestTimes;

	/** Personal best laps per track */
	UPROPERTY()
	TMap<FString, float> PersonalBestLaps;

	/** Maximum history entries to keep */
	UPROPERTY()
	int32 MaxHistoryEntries = 500;
};

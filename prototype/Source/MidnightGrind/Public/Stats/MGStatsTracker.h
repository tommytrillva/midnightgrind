// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGStatsTracker.h - Player Statistics Tracking System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the statistics tracking system for Midnight Grind. Stats
 * tracking is fundamental to any modern game - it records everything the player
 * does so the game can:
 *
 * - Show players their progress and achievements
 * - Unlock rewards based on accomplishments
 * - Feed data to other systems (matchmaking, difficulty, analytics)
 * - Provide "career stats" screens that players love to browse
 *
 * WHAT DOES THIS SYSTEM TRACK?
 * ----------------------------
 *
 * RACE STATS (FMGRaceStats):
 * - Total races entered, wins, podiums (top 3), and DNFs (did not finish)
 * - Pink slip races (betting your car) won and lost
 * - Win streaks - current and best ever
 * - Best lap times across all tracks
 * - Special accomplishments like photo finishes and comeback wins
 *
 * DRIVING STATS (FMGDrivingStats):
 * - Total distance driven (used for odometer-style displays)
 * - Top speed ever achieved
 * - Drift statistics - total score, longest drift, best combo
 * - Near misses, overtakes, and times being overtaken
 * - NOS (nitrous) usage
 * - Collision data - walls, traffic, total damage taken
 *
 * ECONOMY STATS (FMGEconomyStats):
 * - Credits earned and spent (total and by category)
 * - Where credits came from (races, challenges, vehicle sales)
 * - Where credits went (vehicles, parts, customization, repairs)
 * - Vehicle and parts purchase counts
 *
 * SOCIAL STATS (FMGSocialStats):
 * - Rival and nemesis system interactions
 * - Multiplayer race counts and wins
 * - Friend invitations sent and accepted
 *
 * PROGRESSION STATS (FMGProgressionStats):
 * - Current level and total XP earned
 * - Reputation points
 * - Challenges and achievements completed
 * - Career mode progress
 *
 * TIME STATS (FMGTimeStats):
 * - Total play time (a key engagement metric!)
 * - Time spent in different activities (racing, garage, menus)
 * - Session data - count, longest session, first/last play dates
 *
 * VEHICLE-SPECIFIC STATS (FMGVehicleStats):
 * - Per-vehicle: races, wins, distance, best times, drift scores
 * - Helps players see which cars they perform best with
 *
 * TRACK-SPECIFIC STATS (FMGTrackStats):
 * - Per-track: times raced, wins, best times, best vehicle used
 * - Leaderboard position tracking
 *
 * WHY IS STATS TRACKING IMPORTANT?
 * --------------------------------
 *
 * 1. PLAYER MOTIVATION:
 *    Stats give players goals to work toward. "I want to get 100 wins" or
 *    "I want to drift 1 million points" keeps them playing.
 *
 * 2. ACHIEVEMENT SYSTEMS:
 *    Most achievements are based on stats. "Win 50 races" requires tracking wins.
 *
 * 3. MATCHMAKING:
 *    Fair online matchmaking needs to know player skill levels. Win rates,
 *    average positions, and skill ratings help match similar players.
 *
 * 4. ADAPTIVE DIFFICULTY:
 *    The DynamicDifficulty system reads stats to know if the player is
 *    struggling or dominating, then adjusts accordingly.
 *
 * 5. ANALYTICS:
 *    Game developers analyze stats to improve the game. "Players are DNF'ing
 *    30% of races on Track X" suggests that track needs balancing.
 *
 * 6. SOCIAL FEATURES:
 *    "Compare stats with friends" is a popular feature that drives engagement.
 *
 * 7. GAME ECONOMY BALANCE:
 *    Economy stats help designers ensure players earn and spend credits at
 *    appropriate rates.
 *
 * KEY CONCEPTS:
 * -------------
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    Like other subsystems in this codebase, UMGStatsTracker inherits from
 *    UGameInstanceSubsystem. This means it persists across level changes and
 *    is accessible from anywhere in the game.
 *
 * 2. DELEGATES/EVENTS:
 *    - OnStatUpdated: Fired when any stat changes (useful for UI updates)
 *    - OnMilestoneReached: Fired when a milestone is achieved (100 wins, etc.)
 *
 * 3. BLUEPRINT ACCESSIBILITY:
 *    All structs are BlueprintType and most functions are BlueprintCallable,
 *    allowing designers to work with stats without C++ knowledge.
 *
 * 4. HELPER FUNCTIONS:
 *    Structs include helpful functions like GetWinRate(), GetDistanceInMiles(),
 *    and GetPlayTimeHours() to make common calculations easy.
 *
 * USAGE EXAMPLE:
 * --------------
 * @code
 * UMGStatsTracker* Stats = GetGameInstance()->GetSubsystem<UMGStatsTracker>();
 *
 * // Record a race result
 * Stats->RecordRaceResult(
 *     TrackID,
 *     VehicleID,
 *     Position,      // 1 = first place
 *     TotalRacers,   // how many racers in the race
 *     RaceTime,      // total race time in seconds
 *     BestLap,       // best lap time in seconds
 *     DriftScore,    // total drift points earned
 *     bPinkSlip      // was this a pink slip race?
 * );
 *
 * // Record driving events during gameplay
 * Stats->RecordDrift(Score, Duration, ComboCount);
 * Stats->RecordOvertake();
 * Stats->RecordNearMiss();
 * Stats->RecordCollision(bWall, bTraffic, Damage);
 *
 * // Get player stats for display
 * const FMGPlayerStats& PlayerStats = Stats->GetPlayerStats();
 * float WinRate = PlayerStats.Race.GetWinRate();
 * float TotalHours = PlayerStats.Time.GetPlayTimeHours();
 *
 * // Get vehicle-specific stats
 * FMGVehicleStats VehicleStats = Stats->GetVehicleStats(VehicleID);
 * @endcode
 *
 * BEST PRACTICES:
 * ---------------
 * 1. Call recording functions IMMEDIATELY when events happen to ensure accuracy
 * 2. Use the appropriate recording function - don't try to modify stats directly
 * 3. Subscribe to OnStatUpdated for UI elements that display stats
 * 4. Use OnMilestoneReached to trigger achievement notifications
 * 5. Remember to call RecordSessionStart() and RecordSessionEnd() for time tracking
 *
 * DATA PERSISTENCE:
 * -----------------
 * This subsystem tracks stats in memory. Stats should be saved to persistent
 * storage (save game, cloud save) by the save game system. The Initialize() and
 * Deinitialize() methods handle loading and saving respectively.
 *
 * @see UMGDynamicDifficultySubsystem - consumes stats for difficulty adjustment
 * @see UMGBalancingSubsystem - uses metrics for balance decisions
 * @see Save game system - handles persistence of stats data
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStatsTracker.generated.h"

/**
 * Race-related statistics - wins, losses, times, and special achievements.
 *
 * These stats form the core of a player's racing career record. They're
 * displayed on profile screens and used for achievements/leaderboards.
 */
USTRUCT(BlueprintType)
struct FMGRaceStats
{
	GENERATED_BODY()

	/** Total number of races entered (includes DNFs) */
	UPROPERTY(BlueprintReadOnly) int32 TotalRaces = 0;

	/** Number of 1st place finishes */
	UPROPERTY(BlueprintReadOnly) int32 Wins = 0;

	/** Number of top 3 finishes (1st, 2nd, or 3rd place) */
	UPROPERTY(BlueprintReadOnly) int32 Podiums = 0;

	/** Did Not Finish - races quit, crashed out, or timed out */
	UPROPERTY(BlueprintReadOnly) int32 DNFs = 0;

	/** Pink slip races won (took opponent's car as prize) */
	UPROPERTY(BlueprintReadOnly) int32 PinkSlipsWon = 0;

	/** Pink slip races lost (lost your car to opponent) */
	UPROPERTY(BlueprintReadOnly) int32 PinkSlipsLost = 0;

	/** Current consecutive win count (resets on loss) */
	UPROPERTY(BlueprintReadOnly) int32 CurrentWinStreak = 0;

	/** Highest win streak ever achieved */
	UPROPERTY(BlueprintReadOnly) int32 BestWinStreak = 0;

	/** Total time spent actively racing in seconds */
	UPROPERTY(BlueprintReadOnly) float TotalRaceTime = 0.0f;

	/** Best single lap time achieved across all tracks (seconds) */
	UPROPERTY(BlueprintReadOnly) float BestLapTime = 0.0f;

	/** Which track the best lap time was set on */
	UPROPERTY(BlueprintReadOnly) FName BestLapTrack;

	/** Total number of laps completed across all races */
	UPROPERTY(BlueprintReadOnly) int32 LapsCompleted = 0;

	/** Number of checkpoints missed (checkpoint races) */
	UPROPERTY(BlueprintReadOnly) int32 CheckpointsMissed = 0;

	/** Laps completed without any collisions or going off-track */
	UPROPERTY(BlueprintReadOnly) int32 PerfectLaps = 0;

	/** Wins decided by less than 0.5 seconds */
	UPROPERTY(BlueprintReadOnly) int32 PhotoFinishes = 0;

	/** Wins where player was in last place at some point */
	UPROPERTY(BlueprintReadOnly) int32 ComebackWins = 0;

	/** Calculate win rate as a decimal (0.0 to 1.0) */
	float GetWinRate() const { return TotalRaces > 0 ? static_cast<float>(Wins) / TotalRaces : 0.0f; }

	/** Calculate podium rate as a decimal (0.0 to 1.0) */
	float GetPodiumRate() const { return TotalRaces > 0 ? static_cast<float>(Podiums) / TotalRaces : 0.0f; }
};

/**
 * Driving behavior statistics - how the player actually handles their car.
 *
 * These stats track driving style and skill indicators. Useful for achievements
 * like "Drift 1 million points total" and for analytics about player behavior.
 */
USTRUCT(BlueprintType)
struct FMGDrivingStats
{
	GENERATED_BODY()

	/** Total distance driven in centimeters (Unreal default unit) */
	UPROPERTY(BlueprintReadOnly) float TotalDistance = 0.0f;

	/** Highest speed ever reached (cm/s in Unreal units) */
	UPROPERTY(BlueprintReadOnly) float TopSpeed = 0.0f;

	/** Cumulative drift score earned across all races */
	UPROPERTY(BlueprintReadOnly) float TotalDriftScore = 0.0f;

	/** Total number of successful drifts initiated */
	UPROPERTY(BlueprintReadOnly) int32 DriftCount = 0;

	/** Duration of the longest single drift in seconds */
	UPROPERTY(BlueprintReadOnly) float LongestDrift = 0.0f;

	/** Highest drift combo achieved (consecutive drifts without breaking) */
	UPROPERTY(BlueprintReadOnly) int32 BestDriftCombo = 0;

	/** Total time spent airborne from jumps in seconds */
	UPROPERTY(BlueprintReadOnly) float TotalAirTime = 0.0f;

	/** Close calls with traffic/obstacles without hitting them */
	UPROPERTY(BlueprintReadOnly) int32 NearMisses = 0;

	/** Number of successful overtakes (passing an opponent) */
	UPROPERTY(BlueprintReadOnly) int32 Overtakes = 0;

	/** Number of times opponents passed the player */
	UPROPERTY(BlueprintReadOnly) int32 TimesOvertaken = 0;

	/** Total NOS/nitrous used in seconds of boost time */
	UPROPERTY(BlueprintReadOnly) float TotalNOSUsed = 0.0f;

	/** Perfect race starts (optimal launch timing) */
	UPROPERTY(BlueprintReadOnly) int32 PerfectStarts = 0;

	/** Total collisions with anything */
	UPROPERTY(BlueprintReadOnly) int32 Collisions = 0;

	/** Collisions specifically with walls/barriers */
	UPROPERTY(BlueprintReadOnly) int32 WallHits = 0;

	/** Collisions specifically with traffic vehicles */
	UPROPERTY(BlueprintReadOnly) int32 TrafficHits = 0;

	/** Cumulative damage taken (if damage system is used) */
	UPROPERTY(BlueprintReadOnly) float TotalDamage = 0.0f;

	/** Convert distance to miles for display */
	float GetDistanceInMiles() const { return TotalDistance / 160934.0f; }

	/** Convert distance to kilometers for display */
	float GetDistanceInKm() const { return TotalDistance / 100000.0f; }
};

/**
 * Economy statistics - tracking money flow in and out.
 *
 * Important for:
 * - Showing players their financial history
 * - Balancing the game economy (are players earning/spending at intended rates?)
 * - Achievements ("Earn 1 million credits", "Spend 100k on customization")
 *
 * Uses int64 for credit values to support large amounts without overflow.
 */
USTRUCT(BlueprintType)
struct FMGEconomyStats
{
	GENERATED_BODY()

	/** Total credits earned from all sources ever */
	UPROPERTY(BlueprintReadOnly) int64 TotalCreditsEarned = 0;

	/** Total credits spent on all purchases ever */
	UPROPERTY(BlueprintReadOnly) int64 TotalCreditsSpent = 0;

	/** Credits earned specifically from race prizes */
	UPROPERTY(BlueprintReadOnly) int64 CreditsFromRaces = 0;

	/** Credits earned from completing challenges/missions */
	UPROPERTY(BlueprintReadOnly) int64 CreditsFromChallenges = 0;

	/** Credits earned from selling vehicles/parts */
	UPROPERTY(BlueprintReadOnly) int64 CreditsFromSales = 0;

	/** Credits spent purchasing vehicles */
	UPROPERTY(BlueprintReadOnly) int64 SpentOnVehicles = 0;

	/** Credits spent on performance parts */
	UPROPERTY(BlueprintReadOnly) int64 SpentOnParts = 0;

	/** Credits spent on visual customization (paint, decals, etc.) */
	UPROPERTY(BlueprintReadOnly) int64 SpentOnCustomization = 0;

	/** Credits spent on vehicle repairs (if damage system exists) */
	UPROPERTY(BlueprintReadOnly) int64 SpentOnRepairs = 0;

	/** Total number of vehicles bought */
	UPROPERTY(BlueprintReadOnly) int32 VehiclesPurchased = 0;

	/** Total number of vehicles sold */
	UPROPERTY(BlueprintReadOnly) int32 VehiclesSold = 0;

	/** Total number of parts purchased */
	UPROPERTY(BlueprintReadOnly) int32 PartsPurchased = 0;
};

/**
 * Social/multiplayer statistics - interactions with other players and NPCs.
 *
 * The rival/nemesis system creates persistent AI opponents that remember
 * the player and have a relationship history with them.
 */
USTRUCT(BlueprintType)
struct FMGSocialStats
{
	GENERATED_BODY()

	/** Number of AI rivals established through gameplay */
	UPROPERTY(BlueprintReadOnly) int32 RivalsCreated = 0;

	/** Number of nemesis-level rivals defeated (highest rivalry tier) */
	UPROPERTY(BlueprintReadOnly) int32 NemesesDefeated = 0;

	/** Number of taunts/challenges received from rival NPCs */
	UPROPERTY(BlueprintReadOnly) int32 RivalTauntsReceived = 0;

	/** Total online races against other players */
	UPROPERTY(BlueprintReadOnly) int32 MultiplayerRaces = 0;

	/** Online race wins against other players */
	UPROPERTY(BlueprintReadOnly) int32 MultiplayerWins = 0;

	/** Races played specifically against friends */
	UPROPERTY(BlueprintReadOnly) int32 FriendRaces = 0;

	/** Game invites sent to other players */
	UPROPERTY(BlueprintReadOnly) int32 InvitesSent = 0;

	/** Game invites accepted by other players */
	UPROPERTY(BlueprintReadOnly) int32 InvitesAccepted = 0;
};

/**
 * Progression statistics - player advancement through the game.
 *
 * Tracks the player's journey from beginner to endgame. These stats
 * drive unlock systems and show players how far they've come.
 */
USTRUCT(BlueprintType)
struct FMGProgressionStats
{
	GENERATED_BODY()

	/** Player's current level (typically starts at 1) */
	UPROPERTY(BlueprintReadOnly) int32 CurrentLevel = 1;

	/** Total experience points earned across all activities */
	UPROPERTY(BlueprintReadOnly) int32 TotalXPEarned = 0;

	/** Reputation points - separate from XP, used for specific unlocks */
	UPROPERTY(BlueprintReadOnly) int32 TotalReputation = 0;

	/** Number of challenges/missions completed */
	UPROPERTY(BlueprintReadOnly) int32 ChallengesCompleted = 0;

	/** Number of achievements earned */
	UPROPERTY(BlueprintReadOnly) int32 AchievementsUnlocked = 0;

	/** Number of vehicles made available (through unlocks, not purchases) */
	UPROPERTY(BlueprintReadOnly) int32 VehiclesUnlocked = 0;

	/** Number of tracks/circuits unlocked */
	UPROPERTY(BlueprintReadOnly) int32 TracksUnlocked = 0;

	/** Number of career mode story events completed */
	UPROPERTY(BlueprintReadOnly) int32 CareerEventsCompleted = 0;

	/** Overall career completion percentage (0.0 to 1.0) */
	UPROPERTY(BlueprintReadOnly) float CareerProgress = 0.0f;
};

/**
 * Time-based statistics - how much and when the player plays.
 *
 * Time stats are crucial for:
 * - Engagement metrics (is the game sticky?)
 * - Balancing (how long does it take to progress?)
 * - Achievements ("Play for 100 hours")
 * - Analytics (when do players tend to stop playing?)
 */
USTRUCT(BlueprintType)
struct FMGTimeStats
{
	GENERATED_BODY()

	/** Total time spent in-game in seconds */
	UPROPERTY(BlueprintReadOnly) float TotalPlayTime = 0.0f;

	/** Time spent actively racing (in race sessions) */
	UPROPERTY(BlueprintReadOnly) float TimeInRaces = 0.0f;

	/** Time spent in the garage customizing vehicles */
	UPROPERTY(BlueprintReadOnly) float TimeInGarage = 0.0f;

	/** Time spent in menus, shops, and other non-gameplay screens */
	UPROPERTY(BlueprintReadOnly) float TimeInMenus = 0.0f;

	/** Duration of the longest single play session in seconds */
	UPROPERTY(BlueprintReadOnly) float LongestSession = 0.0f;

	/** Total number of play sessions (game launches) */
	UPROPERTY(BlueprintReadOnly) int32 SessionCount = 0;

	/** Date/time of the very first time the player launched the game */
	UPROPERTY(BlueprintReadOnly) FDateTime FirstPlayDate;

	/** Date/time of the most recent play session */
	UPROPERTY(BlueprintReadOnly) FDateTime LastPlayDate;

	/** Number of unique calendar days the player has played */
	UPROPERTY(BlueprintReadOnly) int32 DaysPlayed = 0;

	/** Convert total play time to hours for easy display */
	float GetPlayTimeHours() const { return TotalPlayTime / 3600.0f; }
};

/**
 * Statistics for a specific vehicle.
 *
 * Tracks performance metrics per vehicle, so players can see which cars
 * they perform best with. Useful for:
 * - "My favorite car" displays
 * - Vehicle-specific leaderboards
 * - Analytics on which cars are most popular/effective
 */
USTRUCT(BlueprintType)
struct FMGVehicleStats
{
	GENERATED_BODY()

	/** Unique identifier for this vehicle */
	UPROPERTY(BlueprintReadOnly) FName VehicleID;

	/** Number of races entered with this vehicle */
	UPROPERTY(BlueprintReadOnly) int32 RacesEntered = 0;

	/** Number of wins achieved with this vehicle */
	UPROPERTY(BlueprintReadOnly) int32 Wins = 0;

	/** Total distance driven in this vehicle (centimeters) */
	UPROPERTY(BlueprintReadOnly) float DistanceDriven = 0.0f;

	/** Best lap time achieved in this vehicle */
	UPROPERTY(BlueprintReadOnly) float BestLapTime = 0.0f;

	/** Track where the best lap time was set */
	UPROPERTY(BlueprintReadOnly) FName BestLapTrack;

	/** Total drift score earned in this vehicle */
	UPROPERTY(BlueprintReadOnly) float TotalDriftScore = 0.0f;

	/** Highest speed achieved in this vehicle */
	UPROPERTY(BlueprintReadOnly) float TopSpeed = 0.0f;

	/** Cumulative damage taken by this vehicle */
	UPROPERTY(BlueprintReadOnly) int32 TotalDamage = 0;

	/** Total credits spent repairing this vehicle */
	UPROPERTY(BlueprintReadOnly) int64 RepairCosts = 0;
};

/**
 * Statistics for a specific track/circuit.
 *
 * Tracks performance on each track, allowing players to see where they
 * excel and where they need practice. Feeds into track leaderboards.
 */
USTRUCT(BlueprintType)
struct FMGTrackStats
{
	GENERATED_BODY()

	/** Unique identifier for this track */
	UPROPERTY(BlueprintReadOnly) FName TrackID;

	/** Number of times this track has been raced */
	UPROPERTY(BlueprintReadOnly) int32 TimesRaced = 0;

	/** Number of wins on this track */
	UPROPERTY(BlueprintReadOnly) int32 Wins = 0;

	/** Best single lap time on this track */
	UPROPERTY(BlueprintReadOnly) float BestLapTime = 0.0f;

	/** Best total race time on this track */
	UPROPERTY(BlueprintReadOnly) float BestRaceTime = 0.0f;

	/** Which vehicle achieved the best time */
	UPROPERTY(BlueprintReadOnly) FName BestVehicle;

	/** Total drift score earned on this track */
	UPROPERTY(BlueprintReadOnly) float TotalDriftScore = 0.0f;

	/** Player's position on the global leaderboard for this track */
	UPROPERTY(BlueprintReadOnly) int32 LeaderboardPosition = 0;
};

/**
 * Aggregate container for all player statistics.
 *
 * This is the main struct returned by GetPlayerStats(). It contains all
 * the sub-categories of stats organized for easy access.
 *
 * Example usage:
 * @code
 * const FMGPlayerStats& Stats = StatsTracker->GetPlayerStats();
 * float WinRate = Stats.Race.GetWinRate();
 * float PlayTimeHours = Stats.Time.GetPlayTimeHours();
 * int64 TotalEarnings = Stats.Economy.TotalCreditsEarned;
 * @endcode
 */
USTRUCT(BlueprintType)
struct FMGPlayerStats
{
	GENERATED_BODY()

	/** Race results and records */
	UPROPERTY(BlueprintReadOnly) FMGRaceStats Race;

	/** Driving behavior and skill metrics */
	UPROPERTY(BlueprintReadOnly) FMGDrivingStats Driving;

	/** Credit earning and spending history */
	UPROPERTY(BlueprintReadOnly) FMGEconomyStats Economy;

	/** Multiplayer and rival interactions */
	UPROPERTY(BlueprintReadOnly) FMGSocialStats Social;

	/** Level, XP, and unlock progress */
	UPROPERTY(BlueprintReadOnly) FMGProgressionStats Progression;

	/** Play time and session data */
	UPROPERTY(BlueprintReadOnly) FMGTimeStats Time;
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================

/** Fired when any stat value changes. UI can bind to this for live updates. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatUpdated, FName, StatName, float, NewValue);

/** Fired when a milestone is reached (e.g., "100_wins", "1000_km_driven"). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneReached, FName, MilestoneID);

/**
 * Stats Tracker Subsystem - Central hub for all player statistics.
 *
 * This subsystem:
 * - Collects gameplay events and updates stats accordingly
 * - Stores aggregate stats and per-vehicle/per-track breakdowns
 * - Fires events when stats change or milestones are reached
 * - Provides easy access to all stats for UI and other systems
 *
 * LIFECYCLE:
 * - Initialize(): Loads saved stats from persistent storage
 * - Deinitialize(): Saves stats before shutting down
 *
 * RECORDING METHODS:
 * Call these during gameplay to record events:
 * - RecordRaceResult(): After every race completes
 * - RecordDrift(): When a drift ends (with final score)
 * - RecordOvertake(): Each time player passes an opponent
 * - RecordCollision(): On vehicle collision
 * - etc.
 *
 * ACCESS METHODS:
 * - GetPlayerStats(): Returns all aggregate stats
 * - GetVehicleStats(): Returns stats for a specific vehicle
 * - GetTrackStats(): Returns stats for a specific track
 */
UCLASS()
class MIDNIGHTGRIND_API UMGStatsTracker : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// =========================================================================
	// LIFECYCLE
	// =========================================================================

	/** Called when subsystem is created - loads saved stats */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called when subsystem is destroyed - saves stats */
	virtual void Deinitialize() override;

	// =========================================================================
	// STAT ACCESS
	// =========================================================================

	/** Get all player statistics */
	UFUNCTION(BlueprintPure, Category = "Stats")
	const FMGPlayerStats& GetPlayerStats() const { return PlayerStats; }

	/** Get statistics for a specific vehicle */
	UFUNCTION(BlueprintPure, Category = "Stats")
	FMGVehicleStats GetVehicleStats(FName VehicleID) const;

	/** Get statistics for a specific track */
	UFUNCTION(BlueprintPure, Category = "Stats")
	FMGTrackStats GetTrackStats(FName TrackID) const;

	// =========================================================================
	// RACE RECORDING
	// =========================================================================

	/**
	 * Record the results of a completed race.
	 * Call this at the end of every race, even DNFs.
	 *
	 * @param TrackID Identifier of the track raced on
	 * @param VehicleID Identifier of the vehicle used
	 * @param Position Final position (1 = first place)
	 * @param TotalRacers Number of racers in the race
	 * @param RaceTime Total race time in seconds
	 * @param BestLap Best lap time achieved in seconds
	 * @param DriftScore Total drift points earned in the race
	 * @param bPinkSlip Whether this was a pink slip (bet-your-car) race
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordRaceResult(FName TrackID, FName VehicleID, int32 Position, int32 TotalRacers,
		float RaceTime, float BestLap, float DriftScore, bool bPinkSlip);

	// =========================================================================
	// DRIVING EVENT RECORDING
	// =========================================================================

	/**
	 * Record a completed drift.
	 * Call when a drift ends (player stops sliding).
	 *
	 * @param Score Points earned from this drift
	 * @param Duration How long the drift lasted in seconds
	 * @param ComboCount Number of drifts chained together
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordDrift(float Score, float Duration, int32 ComboCount);

	/** Record an overtake (player passing an opponent) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordOvertake();

	/** Record a near miss (close call without collision) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordNearMiss();

	/**
	 * Record a collision event.
	 *
	 * @param bWall True if collision was with wall/barrier
	 * @param bTraffic True if collision was with traffic vehicle
	 * @param Damage Amount of damage taken (if damage system exists)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordCollision(bool bWall, bool bTraffic, float Damage);

	/**
	 * Record distance traveled. Call periodically during gameplay.
	 * @param Distance Distance in centimeters (Unreal units)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordDistance(float Distance);

	/**
	 * Record a new top speed if it exceeds current record.
	 * @param Speed Speed in cm/s (Unreal units)
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordTopSpeed(float Speed);

	/**
	 * Record NOS/nitrous usage.
	 * @param Amount Time in seconds of boost used
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordNOSUsage(float Amount);

	// =========================================================================
	// ECONOMY RECORDING
	// =========================================================================

	/**
	 * Record credits earned.
	 *
	 * @param Amount Credits earned
	 * @param Source Where credits came from (e.g., "Race", "Challenge", "Sale")
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordCreditsEarned(int64 Amount, FName Source);

	/**
	 * Record credits spent.
	 *
	 * @param Amount Credits spent
	 * @param Category What credits were spent on (e.g., "Vehicle", "Parts", "Customization")
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordCreditsSpent(int64 Amount, FName Category);

	// =========================================================================
	// SESSION TRACKING
	// =========================================================================

	/** Call when a play session begins (game launch, resume from background) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordSessionStart();

	/** Call when a play session ends (quit, suspend, background) */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void RecordSessionEnd();

	/**
	 * Update play time counters. Call every tick or on a timer.
	 *
	 * @param DeltaSeconds Time elapsed since last call
	 * @param Activity Current activity ("Race", "Garage", "Menu")
	 */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpdatePlayTime(float DeltaSeconds, FName Activity);

	// =========================================================================
	// EVENTS
	// =========================================================================

	/** Fired when any stat is updated. Useful for live UI updates. */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnStatUpdated OnStatUpdated;

	/** Fired when a milestone is reached. Trigger achievement notifications. */
	UPROPERTY(BlueprintAssignable, Category = "Stats|Events")
	FOnMilestoneReached OnMilestoneReached;

protected:
	/** Check if any milestones have been reached and fire events */
	void CheckMilestones();

	/** Helper to update vehicle-specific stats using a lambda */
	void UpdateVehicleStats(FName VehicleID, const TFunction<void(FMGVehicleStats&)>& UpdateFunc);

	/** Helper to update track-specific stats using a lambda */
	void UpdateTrackStats(FName TrackID, const TFunction<void(FMGTrackStats&)>& UpdateFunc);

private:
	/** All aggregate player stats */
	FMGPlayerStats PlayerStats;

	/** Stats broken down by vehicle */
	TMap<FName, FMGVehicleStats> VehicleStatsMap;

	/** Stats broken down by track */
	TMap<FName, FMGTrackStats> TrackStatsMap;

	/** Set of milestone IDs that have already been triggered (prevents duplicates) */
	TSet<FName> ReachedMilestones;

	/** Time elapsed in the current session */
	float CurrentSessionTime = 0.0f;

	/** When the current session started */
	FDateTime SessionStartTime;

	/** What the player is currently doing ("Race", "Garage", etc.) */
	FName CurrentActivity;

	/** Which vehicle the player is currently using */
	FName CurrentVehicle;
};

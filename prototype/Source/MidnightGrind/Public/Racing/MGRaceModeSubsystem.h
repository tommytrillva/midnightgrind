/**
 * @file MGRaceModeSubsystem.h
 * @brief Race Mode Subsystem - Core race logic, scoring, and timing management
 *
 * This subsystem handles all the fundamental mechanics of racing in Midnight Grind.
 * It manages different race types (circuit, sprint, drift, drag, etc.), tracks racer
 * progress through checkpoints and laps, calculates positions, and records timing data.
 *
 * ## Key Responsibilities
 * - Managing different race types with their unique rules and scoring
 * - Tracking all racer progress (position, lap, checkpoint, distance)
 * - Precision timing for laps, sectors, and total race time
 * - Position calculation and live standings
 * - Drift scoring for drift races
 * - Drag racing timing (reaction times, splits)
 * - Track records and personal bests
 *
 * ## Race Types Supported
 * - Circuit: Multi-lap closed circuit races
 * - Sprint: Point-to-point single run races
 * - Drift: Score-based drifting competitions
 * - Drag: Quarter-mile straight line races
 * - Time Attack: Solo time trial against the clock
 * - Elimination: Last place eliminated at intervals
 * - Pink Slip: Winner takes the loser's vehicle
 * - Touge: Mountain pass races (like Initial D)
 * - Knockout: Tournament-style elimination
 * - Checkpoint: Race against time hitting checkpoints
 *
 * ## Usage Example
 * @code
 * UMGRaceModeSubsystem* RaceMode = GetGameInstance()->GetSubsystem<UMGRaceModeSubsystem>();
 * FMGRaceConfig Config;
 * Config.RaceType = EMGRaceType::Circuit;
 * Config.NumLaps = 3;
 * RaceMode->SetupRace(Config);
 * RaceMode->StartCountdown();
 * @endcode
 *
 * @note This is a GameInstanceSubsystem that persists across level loads
 * @see UMGRaceFlowSubsystem for high-level race orchestration
 * @see UMGRaceDirectorSubsystem for AI pacing and drama
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceModeSubsystem.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class AMGVehiclePawn;
class AMGCheckpoint;
class UMGVehicleDefinition;

// ============================================================================
// RACE TYPE ENUMERATION
// ============================================================================

/**
 * @enum EMGRaceType
 * @brief Defines all available race modes in Midnight Grind
 *
 * Each race type has unique rules, scoring systems, and win conditions.
 * The Race Mode Subsystem uses this to configure appropriate behaviors.
 */
UENUM(BlueprintType)
enum class EMGRaceType : uint8
{
	/// Standard multi-lap race on a closed circuit track
	Circuit UMETA(DisplayName = "Circuit"),

	/// Point-to-point race from start to finish with no laps
	Sprint UMETA(DisplayName = "Sprint (Point-to-Point)"),

	/// Quarter-mile straight line acceleration race
	Drag UMETA(DisplayName = "Drag Race"),

	/// Score-based race where drifting earns points
	Drift UMETA(DisplayName = "Drift"),

	/// Solo time trial - race against the clock for best lap
	TimeAttack UMETA(DisplayName = "Time Attack"),

	/// Last place racer eliminated at regular intervals
	Elimination UMETA(DisplayName = "Elimination"),

	/// High-stakes race where winner takes loser's vehicle
	PinkSlip UMETA(DisplayName = "Pink Slip"),

	/// Japanese mountain pass style racing (Initial D inspired)
	Touge UMETA(DisplayName = "Touge (Mountain Pass)"),

	/// Tournament bracket elimination racing
	Knockout UMETA(DisplayName = "Knockout"),

	/// Race to hit checkpoints before time expires
	Checkpoint UMETA(DisplayName = "Checkpoint Rush"),

	/// Open world exploration without race objectives
	FreeRoam UMETA(DisplayName = "Free Roam")
};

// ============================================================================
// RACE DIFFICULTY ENUMERATION
// ============================================================================

/**
 * @enum EMGRaceDifficulty
 * @brief AI difficulty levels affecting opponent behavior and rubber-banding
 *
 * Higher difficulties increase AI speed, racing line precision, and reduce
 * the player's rubber-banding assistance.
 */
UENUM(BlueprintType)
enum class EMGRaceDifficulty : uint8
{
	/// Forgiving AI, strong player assistance
	Easy UMETA(DisplayName = "Easy"),

	/// Balanced challenge for average players
	Medium UMETA(DisplayName = "Medium"),

	/// Competitive AI, reduced player assistance
	Hard UMETA(DisplayName = "Hard"),

	/// Aggressive AI, minimal assistance
	Expert UMETA(DisplayName = "Expert"),

	/// Maximum challenge, no assistance
	Legendary UMETA(DisplayName = "Legendary")
};

// ============================================================================
// RACE STATE ENUMERATION
// ============================================================================

/**
 * @enum EMGRaceState
 * @brief Current state of the active race
 *
 * Represents the lifecycle of a single race from setup through completion.
 */
UENUM(BlueprintType)
enum class EMGRaceState : uint8
{
	/// No race configured
	None UMETA(DisplayName = "None"),

	/// Race is being configured, racers added
	Setup UMETA(DisplayName = "Setup"),

	/// Countdown in progress (3, 2, 1, GO!)
	Countdown UMETA(DisplayName = "Countdown"),

	/// Active racing - vehicles moving
	Racing UMETA(DisplayName = "Racing"),

	/// All racers finished or race ended normally
	Finished UMETA(DisplayName = "Finished"),

	/// Race ended due to Did Not Finish (time limit, crash, etc.)
	DNF UMETA(DisplayName = "DNF"),

	/// Race was cancelled before completion
	Cancelled UMETA(DisplayName = "Cancelled")
};

// ============================================================================
// TRAFFIC DENSITY ENUMERATION
// ============================================================================

/**
 * @enum EMGTrafficDensity
 * @brief Traffic level setting for street racing environments
 *
 * Controls the number of civilian vehicles on the road during races.
 * Higher density adds challenge but risks collisions.
 */
UENUM(BlueprintType)
enum class EMGTrafficDensity : uint8
{
	/// Empty streets - no traffic
	None UMETA(DisplayName = "No Traffic"),

	/// Occasional vehicles - minimal obstacle
	Light UMETA(DisplayName = "Light Traffic"),

	/// Moderate traffic - requires attention
	Medium UMETA(DisplayName = "Medium Traffic"),

	/// Dense traffic - significant challenge
	Heavy UMETA(DisplayName = "Heavy Traffic"),

	/// Maximum traffic - extreme difficulty
	Rush UMETA(DisplayName = "Rush Hour")
};

// ============================================================================
// PERFORMANCE CLASS ENUMERATION
// ============================================================================

/**
 * @enum EMGPerformanceClass
 * @brief Vehicle performance classification based on Performance Index (PI)
 *
 * Used for balanced matchmaking and class-restricted races.
 * PI is calculated from vehicle stats (power, weight, handling, etc.)
 */
UENUM(BlueprintType)
enum class EMGPerformanceClass : uint8
{
	/// Entry level: 0-299 PI (economy cars, project starters)
	D UMETA(DisplayName = "D Class (0-299 PI)"),

	/// Street: 300-399 PI (stock sports cars)
	C UMETA(DisplayName = "C Class (300-399 PI)"),

	/// Sport: 400-499 PI (tuned sports cars)
	B UMETA(DisplayName = "B Class (400-499 PI)"),

	/// Super: 500-599 PI (high-end sports cars)
	A UMETA(DisplayName = "A Class (500-599 PI)"),

	/// Supercar: 600-699 PI (exotic supercars)
	S UMETA(DisplayName = "S Class (600-699 PI)"),

	/// Hypercar: 700-799 PI (track-focused machines)
	S1 UMETA(DisplayName = "S1 Class (700-799 PI)"),

	/// Ultimate: 800-899 PI (extreme builds)
	S2 UMETA(DisplayName = "S2 Class (800-899 PI)"),

	/// Unclassified: 900+ PI (experimental, no limits)
	X UMETA(DisplayName = "X Class (900+ PI)"),

	/// No restrictions on vehicle performance
	Open UMETA(DisplayName = "Open Class (No Limit)")
};

// ============================================================================
// RACER ENTRY STRUCTURE
// ============================================================================

/**
 * @struct FMGRacerEntry
 * @brief Complete data for a single racer (player or AI) in a race
 *
 * Contains identity, vehicle reference, race progress, timing data, and
 * performance statistics. Updated continuously during the race.
 */
USTRUCT(BlueprintType)
struct FMGRacerEntry
{
	GENERATED_BODY()

	// ---- Identity ----

	/// Unique identifier for this racer in the current race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FGuid RacerID;

	/// Display name (player name or AI character name)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FString RacerName;

	/// True if this is the human player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	bool bIsPlayer = false;

	/// True if this is an AI-controlled racer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	bool bIsAI = true;

	// ---- Vehicle ----

	/// Reference to the vehicle pawn in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	TWeakObjectPtr<AMGVehiclePawn> Vehicle;

	/// Vehicle definition ID (used for loading/spawning)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	FName VehicleID;

	/// Vehicle's Performance Index for class determination
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
	int32 PerformanceIndex = 400;

	// ---- Race Progress ----

	/// Current race position (1 = first place)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentPosition = 0;

	/// Current lap number (1-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentLap = 0;

	/// Index of last checkpoint passed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentCheckpoint = 0;

	/// Total distance traveled in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float TotalDistance = 0.0f;

	/// Overall race progress from 0.0 (start) to 1.0 (finish)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float RaceProgress = 0.0f;

	/// True when racer has crossed the finish line
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bFinished = false;

	/// True if racer Did Not Finish (crashed, quit, time limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bDNF = false;

	/// True if racer was eliminated (elimination race mode)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bEliminated = false;

	// ---- Timing ----

	/// Total elapsed race time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TotalTime = 0.0f;

	/// Array of completed lap times
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> LapTimes;

	/// Array of sector times within current lap
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> SectorTimes;

	/// Best lap time achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BestLapTime = 0.0f;

	/// Time when racer crossed finish line
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float FinishTime = 0.0f;

	// ---- Performance Statistics ----

	/// Maximum speed achieved during race (km/h)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TopSpeed = 0.0f;

	/// Count of perfect gear shifts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 PerfectShifts = 0;

	/// Accumulated drift score points
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DriftScore = 0.0f;

	/// Count of near misses with traffic/obstacles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 NearMisses = 0;

	/// Count of vehicle collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 Collisions = 0;
};

// ============================================================================
// RACE CONFIGURATION STRUCTURE
// ============================================================================

/**
 * @struct FMGRaceConfig
 * @brief Complete configuration defining all parameters for a race
 *
 * This struct is used to configure the Race Mode Subsystem before starting
 * a race. It defines race type, track, opponents, conditions, and rewards.
 */
USTRUCT(BlueprintType)
struct FMGRaceConfig
{
	GENERATED_BODY()

	// ---- Basic Configuration ----

	/// Unique identifier for this race instance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName RaceID;

	/// Display name shown in UI
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FString RaceName;

	/// Type of race (circuit, sprint, drift, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	/// AI difficulty level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGRaceDifficulty Difficulty = EMGRaceDifficulty::Medium;

	/// Vehicle performance class restriction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	EMGPerformanceClass PerformanceClass = EMGPerformanceClass::Open;

	// ---- Track Configuration ----

	/// Track identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FName TrackID;

	/// Soft reference to track level asset (for async loading)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TSoftObjectPtr<UWorld> TrackLevel;

	/// Number of laps for circuit races
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	int32 NumLaps = 3;

	/// Total track length in meters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float TrackLength = 5000.0f;

	/// Run track in reverse direction
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	bool bIsReversed = false;

	/// Race during nighttime (Midnight Grind aesthetic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	bool bIsNightRace = true;

	// ---- Opponent Configuration ----

	/// Number of AI opponents
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Opponents")
	int32 NumOpponents = 7;

	/// AI skill level from 0.0 (novice) to 1.0 (expert)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Opponents")
	float AISkillLevel = 0.5f;

	// ---- Race Conditions ----

	/// Traffic density on the streets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	EMGTrafficDensity TrafficDensity = EMGTrafficDensity::Light;

	/// Enable police pursuit during race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bPoliceEnabled = true;

	/// Enable catch-up/rubber-banding mechanics
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bCatchupEnabled = true;

	/// Enable vehicle damage from collisions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bCollisionDamageEnabled = true;

	// ---- Reward Configuration ----

	/// Cash reward for 1st place
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 PrizeMoney1st = 5000;

	/// Cash reward for 2nd place
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 PrizeMoney2nd = 2500;

	/// Cash reward for 3rd place
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 PrizeMoney3rd = 1000;

	/// Reputation points reward
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 REPReward = 100;

	/// Entry fee required to participate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 BuyIn = 0;

	// ---- Pink Slip Configuration ----

	/// Is this a pink slip (vehicle wager) race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	bool bIsPinkSlipRace = false;

	/// Vehicle being wagered by the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	FName PinkSlipVehicleID;

	// ---- Time Limits ----

	/// Time limit in seconds (0 = no limit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TimeLimit = 0.0f;

	/// Interval between eliminations in elimination race mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float EliminationInterval = 30.0f;
};

// ============================================================================
// RACE RESULT STRUCTURE
// ============================================================================

/**
 * @struct FMGRaceResult
 * @brief Complete results from a finished race
 *
 * Contains final standings, timing data, rewards, and statistics
 * generated when a race concludes.
 */
USTRUCT(BlueprintType)
struct FMGRaceResult
{
	GENERATED_BODY()

	// ---- Race Info ----

	/// Copy of the race configuration used
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	FMGRaceConfig RaceConfig;

	/// Final standings of all racers
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	TArray<FMGRacerEntry> FinalStandings;

	// ---- Player Results ----

	/// Player's finishing position (1-based)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	int32 PlayerFinishPosition = 0;

	/// True if player won (finished 1st)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPlayerWon = false;

	/// True if player Did Not Finish
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Result")
	bool bPlayerDNF = false;

	// ---- Rewards ----

	/// Cash earned by player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 CashEarned = 0;

	/// Reputation earned by player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 REPEarned = 0;

	/// True if player won opponent's vehicle (pink slip)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	bool bWonPinkSlip = false;

	/// Vehicle ID won in pink slip race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName PinkSlipVehicleWon;

	// ---- Timing ----

	/// Player's best lap time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PlayerBestLap = 0.0f;

	/// Player's total race time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PlayerTotalTime = 0.0f;

	/// Current track record time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TrackRecordTime = 0.0f;

	/// True if player set new track record
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	bool bNewTrackRecord = false;

	// ---- Statistics ----

	/// Player's maximum speed during race
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PlayerTopSpeed = 0.0f;

	/// Total drift score (drift races)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TotalDriftScore = 0.0f;

	/// Total near misses achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalNearMisses = 0;

	/// Total perfect shifts achieved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalPerfectShifts = 0;
};

// ============================================================================
// CHECKPOINT DATA STRUCTURE
// ============================================================================

/**
 * @struct FMGCheckpointData
 * @brief Position and state data for a track checkpoint
 *
 * Used by the race mode subsystem to track checkpoint positions
 * and validate racer passage through checkpoints in order.
 */
USTRUCT(BlueprintType)
struct FMGCheckpointData
{
	GENERATED_BODY()

	/// Sequential index of this checkpoint (0 = start/finish)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	/// World position of checkpoint center
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FVector Location = FVector::ZeroVector;

	/// Facing direction (for directional validation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FRotator Rotation = FRotator::ZeroRotator;

	/// Distance in meters from start line
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	float DistanceFromStart = 0.0f;

	/// True if this is the start/finish line
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsStartFinish = false;

	/// True if this checkpoint marks a sector boundary
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	bool bIsSectorMarker = false;

	/// Which sector this checkpoint ends (if sector marker)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 SectorIndex = 0;

	/// Reference to the checkpoint actor in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	TWeakObjectPtr<AMGCheckpoint> CheckpointActor;
};

// ============================================================================
// TRACK RECORD STRUCTURE
// ============================================================================

/**
 * @struct FMGTrackRecord
 * @brief Persistent best time record for a track/race type combination
 *
 * Stored in save data and used for ghost cars, personal bests,
 * and leaderboard display.
 */
USTRUCT(BlueprintType)
struct FMGTrackRecord
{
	GENERATED_BODY()

	/// Track this record is for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FName TrackID;

	/// Race type (records are separate per type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	/// Best lap time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float BestLapTime = 0.0f;

	/// Best total race time in seconds
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	float BestTotalTime = 0.0f;

	/// Name of player who holds the record
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FString RecordHolderName;

	/// Vehicle used to set the record
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FName VehicleUsed;

	/// When the record was set
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Record")
	FDateTime RecordDate;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Broadcast when race state changes (setup, countdown, racing, finished, etc.)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceStateChanged, EMGRaceState, NewState);

/// Broadcast each second during countdown (3, 2, 1...)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTick, int32, SecondsRemaining);

/// Broadcast when countdown ends and race begins
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);

/// Broadcast when race ends with final results
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFinished, const FMGRaceResult&, Result);

/// Broadcast when any racer completes a lap
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLapCompleted, const FGuid&, RacerID, int32, LapNumber);

/// Broadcast when any racer passes a checkpoint
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointPassed, const FGuid&, RacerID, int32, CheckpointIndex);

/// Broadcast when any racer's position changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPositionChanged, const FGuid&, RacerID, int32, NewPosition);

/// Broadcast when a racer is eliminated (elimination mode)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacerEliminated, const FGuid&, RacerID);

/// Broadcast when a racer crosses the finish line
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacerFinished, const FMGRacerEntry&, Racer);

/// Broadcast when a new track record is set
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewTrackRecord, const FMGTrackRecord&, Record);

// ============================================================================
// RACE MODE SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGRaceModeSubsystem
 * @brief Core race logic, timing, and scoring management
 *
 * ## Overview
 * This subsystem is the heart of the racing mechanics. It handles all the
 * moment-to-moment logic of an active race including position tracking,
 * timing, checkpoint validation, and scoring.
 *
 * ## Key Features
 * - Multi-race-type support with unique rules per type
 * - Real-time position calculation based on progress
 * - Precision timing for laps and sectors
 * - Drift scoring system for drift races
 * - Drag racing reaction time and splits
 * - Track record persistence
 *
 * ## Integration Points
 * - Receives setup from UMGRaceFlowSubsystem
 * - Reports progress to UMGRaceDirectorSubsystem for pacing
 * - Checkpoints trigger RacerPassedCheckpoint()
 * - Vehicle systems call UpdateRacerTime() each frame
 *
 * ## For New Developers
 * 1. Call SetupRace() with a FMGRaceConfig
 * 2. Call AddRacer() for each participant
 * 3. Call StartCountdown() to begin
 * 4. Call RacerPassedCheckpoint() as racers cross checkpoints
 * 5. Race ends automatically or call FinishRace()
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRaceModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// DELEGATES
	// Subscribe to these for race state notifications
	// ==========================================

	/// Fires when race state changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStateChanged OnRaceStateChanged;

	/// Fires each second during countdown
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCountdownTick OnCountdownTick;

	/// Fires when race starts (after countdown)
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	/// Fires when race ends with results
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFinished OnRaceFinished;

	/// Fires when any racer completes a lap
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLapCompleted OnLapCompleted;

	/// Fires when any racer passes a checkpoint
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCheckpointPassed OnCheckpointPassed;

	/// Fires when positions change
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPositionChanged OnPositionChanged;

	/// Fires when racer is eliminated
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerEliminated OnRacerEliminated;

	/// Fires when racer finishes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerFinished OnRacerFinished;

	/// Fires when new track record is set
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewTrackRecord OnNewTrackRecord;

	// ==========================================
	// RACE SETUP
	// Configure the race before starting
	// ==========================================

	/// Configure race with given settings. Returns false if configuration invalid.
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool SetupRace(const FMGRaceConfig& Config);

	/// Add a racer to the race. Returns false if race already started.
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool AddRacer(const FMGRacerEntry& Racer);

	/// Remove racer by ID. Returns false if not found.
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool RemoveRacer(const FGuid& RacerID);

	/// Remove all racers
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	void ClearRacers();

	/// Register a checkpoint for this race
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	void RegisterCheckpoint(const FMGCheckpointData& Checkpoint);

	/// Clear all registered checkpoints
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	void ClearCheckpoints();

	/// Validate current setup is ready to race
	UFUNCTION(BlueprintCallable, Category = "Race Setup")
	bool ValidateRaceSetup() const;

	// ==========================================
	// RACE CONTROL
	// Start, pause, and stop the race
	// ==========================================

	/// Begin the countdown sequence (3, 2, 1, GO!)
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	bool StartCountdown();

	/// Start the race (called automatically after countdown)
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void StartRace();

	/// Pause race timing and AI
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void PauseRace();

	/// Resume paused race
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void ResumeRace();

	/// End the race and generate results
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void FinishRace();

	/// Cancel race without generating results
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void CancelRace();

	/// Restart with same configuration
	UFUNCTION(BlueprintCallable, Category = "Race Control")
	void RestartRace();

	// ==========================================
	// RACE STATE
	// Query current race status
	// ==========================================

	/// Get current race state
	UFUNCTION(BlueprintPure, Category = "Race State")
	EMGRaceState GetRaceState() const { return CurrentRaceState; }

	/// Is race actively running
	UFUNCTION(BlueprintPure, Category = "Race State")
	bool IsRaceInProgress() const { return CurrentRaceState == EMGRaceState::Racing; }

	/// Get current race configuration
	UFUNCTION(BlueprintPure, Category = "Race State")
	const FMGRaceConfig& GetCurrentRaceConfig() const { return CurrentRaceConfig; }

	/// Get elapsed race time
	UFUNCTION(BlueprintPure, Category = "Race State")
	float GetRaceTime() const { return RaceTime; }

	/// Get countdown seconds remaining
	UFUNCTION(BlueprintPure, Category = "Race State")
	int32 GetCountdownSeconds() const { return CountdownSeconds; }

	// ==========================================
	// RACER PROGRESS
	// Update and query racer positions
	// ==========================================

	/// Called when racer crosses checkpoint (by checkpoint trigger)
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerPassedCheckpoint(const FGuid& RacerID, int32 CheckpointIndex);

	/// Called when racer completes a lap
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerCompletedLap(const FGuid& RacerID);

	/// Mark racer as finished
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerFinished(const FGuid& RacerID);

	/// Mark racer as Did Not Finish
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void RacerDNF(const FGuid& RacerID);

	/// Eliminate racer (for elimination mode)
	UFUNCTION(BlueprintCallable, Category = "Progress")
	void EliminateRacer(const FGuid& RacerID);

	/// Get racer data by ID
	UFUNCTION(BlueprintPure, Category = "Progress")
	FMGRacerEntry GetRacerData(const FGuid& RacerID) const;

	/// Get all racers sorted by position
	UFUNCTION(BlueprintPure, Category = "Progress")
	TArray<FMGRacerEntry> GetCurrentStandings() const;

	/// Get racer's current position
	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetRacerPosition(const FGuid& RacerID) const;

	// ==========================================
	// PLAYER SPECIFIC
	// Convenience methods for the human player
	// ==========================================

	/// Set which racer is the player
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetPlayerRacer(const FGuid& RacerID);

	/// Get player's racer data
	UFUNCTION(BlueprintPure, Category = "Player")
	FMGRacerEntry GetPlayerRacerData() const;

	/// Get player's current position
	UFUNCTION(BlueprintPure, Category = "Player")
	int32 GetPlayerPosition() const;

	/// Get player's current lap
	UFUNCTION(BlueprintPure, Category = "Player")
	int32 GetPlayerCurrentLap() const;

	/// Get player's best lap time
	UFUNCTION(BlueprintPure, Category = "Player")
	float GetPlayerBestLap() const;

	/// Get time gap to race leader (negative if leading)
	UFUNCTION(BlueprintPure, Category = "Player")
	float GetGapToLeader() const;

	/// Get time gap to racer ahead (negative if leading)
	UFUNCTION(BlueprintPure, Category = "Player")
	float GetGapToRacerAhead() const;

	/// Get time gap to racer behind
	UFUNCTION(BlueprintPure, Category = "Player")
	float GetGapToRacerBehind() const;

	// ==========================================
	// TIMING
	// Lap and race timing functions
	// ==========================================

	/// Update racer's elapsed time (called each frame)
	UFUNCTION(BlueprintCallable, Category = "Timing")
	void UpdateRacerTime(const FGuid& RacerID, float DeltaTime);

	/// Get racer's current lap time
	UFUNCTION(BlueprintPure, Category = "Timing")
	float GetRacerLapTime(const FGuid& RacerID) const;

	/// Get racer's best lap time
	UFUNCTION(BlueprintPure, Category = "Timing")
	float GetRacerBestLap(const FGuid& RacerID) const;

	/// Get track record lap time
	UFUNCTION(BlueprintPure, Category = "Timing")
	float GetTrackRecordLap(FName TrackID, EMGRaceType RaceType) const;

	// ==========================================
	// DRIFT SCORING
	// Drift race specific functionality
	// ==========================================

	/// Add drift score points to racer
	UFUNCTION(BlueprintCallable, Category = "Drift")
	void AddDriftScore(const FGuid& RacerID, float Score);

	/// Get racer's total drift score
	UFUNCTION(BlueprintPure, Category = "Drift")
	float GetRacerDriftScore(const FGuid& RacerID) const;

	/// Get standings sorted by drift score
	UFUNCTION(BlueprintPure, Category = "Drift")
	TArray<FMGRacerEntry> GetDriftStandings() const;

	// ==========================================
	// DRAG RACING
	// Drag race specific functionality
	// ==========================================

	/// Record racer's reaction time at start
	UFUNCTION(BlueprintCallable, Category = "Drag")
	void RecordReactionTime(const FGuid& RacerID, float ReactionTime);

	/// Record racer's split time at distance marker
	UFUNCTION(BlueprintCallable, Category = "Drag")
	void RecordDragSplit(const FGuid& RacerID, float Distance, float Time);

	/// Get racer's reaction time
	UFUNCTION(BlueprintPure, Category = "Drag")
	float GetRacerReactionTime(const FGuid& RacerID) const;

	// ==========================================
	// UTILITIES
	// Helper functions for common operations
	// ==========================================

	/// Get performance class for a PI value
	UFUNCTION(BlueprintPure, Category = "Utilities")
	EMGPerformanceClass GetClassForPI(int32 PerformanceIndex) const;

	/// Get display name for performance class
	UFUNCTION(BlueprintPure, Category = "Utilities")
	FString GetClassDisplayName(EMGPerformanceClass Class) const;

	/// Calculate cash reward for position
	UFUNCTION(BlueprintPure, Category = "Utilities")
	int64 CalculateRewards(int32 Position, const FMGRaceConfig& Config) const;

	/// Calculate REP reward for position
	UFUNCTION(BlueprintPure, Category = "Utilities")
	int32 CalculateREPReward(int32 Position, const FMGRaceConfig& Config) const;

	// ==========================================
	// RECORDS
	// Track record management
	// ==========================================

	/// Save a new track record
	UFUNCTION(BlueprintCallable, Category = "Records")
	void SaveTrackRecord(const FMGTrackRecord& Record);

	/// Get track record for specific track/type
	UFUNCTION(BlueprintPure, Category = "Records")
	FMGTrackRecord GetTrackRecord(FName TrackID, EMGRaceType RaceType) const;

	/// Get all records for a track
	UFUNCTION(BlueprintPure, Category = "Records")
	TArray<FMGTrackRecord> GetAllRecordsForTrack(FName TrackID) const;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/// Recalculate all racer positions based on progress
	void UpdatePositions();

	/// Update elimination timer and eliminate last place if needed
	void UpdateEliminationTimer(float DeltaTime);

	/// Check if race completion conditions are met
	void CheckRaceCompletion();

	/// Generate race result struct from current state
	FMGRaceResult GenerateRaceResult();

	/// Transition to new race state
	void SetRaceState(EMGRaceState NewState);

private:
	// ---- Timer Handles ----
	FTimerHandle UpdateTimerHandle;          ///< General update tick
	FTimerHandle CountdownTimerHandle;       ///< Countdown tick
	FTimerHandle EliminationTimerHandle;     ///< Elimination mode tick

	// ---- Race State ----
	UPROPERTY()
	EMGRaceState CurrentRaceState = EMGRaceState::None;

	UPROPERTY()
	FMGRaceConfig CurrentRaceConfig;

	UPROPERTY()
	TArray<FMGRacerEntry> Racers;

	UPROPERTY()
	TArray<FMGCheckpointData> Checkpoints;

	UPROPERTY()
	FGuid PlayerRacerID;

	// ---- Timing State ----
	UPROPERTY()
	float RaceTime = 0.0f;

	UPROPERTY()
	int32 CountdownSeconds = 3;

	UPROPERTY()
	bool bRacePaused = false;

	// ---- Elimination Mode State ----
	UPROPERTY()
	float EliminationTimer = 0.0f;

	UPROPERTY()
	int32 EliminationCount = 0;

	// ---- Drag Racing State ----
	UPROPERTY()
	TMap<FGuid, float> ReactionTimes;

	UPROPERTY()
	TMap<FGuid, TArray<float>> DragSplits;

	// ---- Record Storage ----
	UPROPERTY()
	TArray<FMGTrackRecord> TrackRecords;

	// ---- Result Cache ----
	UPROPERTY()
	FMGRaceResult LastRaceResult;

	// ---- Position Tracking ----
	UPROPERTY()
	int32 NextFinishPosition = 1;
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * =============================================================================
 * @file MGCheckpointSubsystem.h
 * @brief Checkpoint System - Race Timing, Laps, Sectors, and Wrong Way Detection
 * =============================================================================
 *
 * @section Overview
 * This subsystem manages the checkpoint system for racing in Midnight Grind.
 * Checkpoints are invisible (or visible) triggers placed around tracks that
 * validate race progress, track timing, and detect wrong-way driving. This is
 * the core system that makes racing "work" - without it, the game wouldn't know
 * when you've completed a lap or crossed the finish line.
 *
 * @section WhyCheckpoints Why Checkpoints Matter
 *
 * Checkpoints serve multiple purposes:
 * - LAP VALIDATION: Ensures player drove the whole track, not shortcuts
 * - TIMING: Split times, sector times, lap times for competitive racing
 * - PROGRESSION: Know when to count a lap, when to end the race
 * - ANTI-CHEAT: Prevents skipping sections of the track
 * - NAVIGATION: Wrong-way detection, next checkpoint guidance
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * 1. GAME INSTANCE SUBSYSTEM
 *    Inherits from UGameInstanceSubsystem:
 *    - One instance for entire game session
 *    - Persists across level loads (keeps best times)
 *    - Access via: GetGameInstance()->GetSubsystem<UMGCheckpointSubsystem>()
 *
 * 2. CHECKPOINT TYPES (EMGCheckpointType)
 *    Different checkpoint functions:
 *    - Standard: Regular checkpoint for progress tracking
 *    - Start: Race starting line
 *    - Finish: Race ending line (may be same as Start for circuits)
 *    - Lap: Marks lap completion (usually same as Start/Finish)
 *    - Split: Timing checkpoint for split times
 *    - Secret: Hidden checkpoint for shortcuts
 *    - Bonus: Optional checkpoint for extra points
 *    - Mandatory: MUST be passed or lap is invalid
 *    - Optional: Can be skipped without penalty
 *    - TimeExtension: Adds time in time-attack modes
 *
 * 3. CHECKPOINT STATES (EMGCheckpointState)
 *    Current status of each checkpoint:
 *    - Inactive: Not part of current race
 *    - Active: The next checkpoint to pass
 *    - Upcoming: Soon to be active (for UI preview)
 *    - Passed: Successfully crossed
 *    - Missed: Skipped (may invalidate lap)
 *    - Invalid: Crossed wrong way or in wrong order
 *
 * 4. CHECKPOINT SHAPES (EMGCheckpointShape)
 *    Trigger volume geometry:
 *    - Box: Rectangular trigger (most common)
 *    - Sphere: Spherical trigger
 *    - Plane: Thin plane to cross (precise timing)
 *    - Cylinder: Cylindrical trigger
 *    - Custom: Complex geometry
 *
 * 5. CHECKPOINT LAYOUT (FMGCheckpointLayout)
 *    Complete checkpoint configuration for a track:
 *    - Array of checkpoints in order
 *    - Sector definitions (groups of checkpoints)
 *    - Total laps for circuit races
 *    - bIsCircuit: True for loops, false for point-to-point
 *
 * 6. SECTORS (FMGSectorDefinition)
 *    Track divided into timed sections:
 *    - Typically 3 sectors per lap (S1, S2, S3)
 *    - Each sector has best/target times
 *    - Sector colors for UI (green=personal best, purple=all-time best)
 *
 * 7. LAP DATA (FMGLapData)
 *    Information about a completed lap:
 *    - LapTime: Total time for the lap
 *    - SectorTimes: Time for each sector
 *    - Passages: Every checkpoint crossed
 *    - bIsValid: True if all checkpoints hit
 *    - bIsBestLap: True if new personal best
 *
 * 8. SPLIT TIMES & DELTA
 *    Comparison timing:
 *    - Split time: Time at each checkpoint
 *    - Delta: Difference from best/target (+/- seconds)
 *    - Green delta: Ahead of target
 *    - Red delta: Behind target
 *
 * 9. DIRECTION VALIDATION
 *    Checkpoints verify approach direction:
 *    - RequiredDirection: Which way to cross
 *    - DirectionTolerance: Allowed angle deviation
 *    - Prevents driving backward through checkpoints
 *
 * 10. WRONG WAY DETECTION
 *     System detects when player drives backward:
 *     - Compares velocity to expected direction
 *     - OnWrongWay delegate fires
 *     - UI shows "WRONG WAY" warning
 *
 * 11. TIME EXTENSION MODE
 *     For arcade-style time attack:
 *     - Checkpoints add time when crossed
 *     - TimeExtensionSeconds per checkpoint
 *     - OnTimeExpired when clock runs out
 *
 * @section Usage Common Usage Patterns
 *
 * @code
 * // Get the checkpoint subsystem
 * UMGCheckpointSubsystem* Checkpoints =
 *     GetGameInstance()->GetSubsystem<UMGCheckpointSubsystem>();
 *
 * // Load a track layout before starting race
 * if (Checkpoints->LoadLayout("Downtown_Circuit_Main"))
 * {
 *     // Layout loaded, ready to race
 * }
 *
 * // Start a race (3 laps, no time limit)
 * Checkpoints->StartRace(3, 0.0f);
 *
 * // Start a time attack (1 lap, 90 second limit)
 * Checkpoints->StartRace(1, 90.0f);
 *
 * // In vehicle Tick, update checkpoint detection
 * Checkpoints->UpdateCheckpointDetection(
 *     GetActorLocation(),
 *     GetVelocity(),
 *     DeltaTime
 * );
 *
 * // Also update wrong way detection
 * Checkpoints->UpdateWrongWayDetection(GetVelocity());
 *
 * // Get current race state for UI
 * FMGActiveCheckpointState State = Checkpoints->GetActiveState();
 * // Display: Lap State.CurrentLap, Time State.CurrentLapTime
 *
 * // Get timing delta (how far ahead/behind)
 * float Delta = Checkpoints->GetCurrentDelta();
 * FLinearColor DeltaColor = Checkpoints->GetDeltaColor(Delta);
 * FText DeltaText = Checkpoints->FormatDelta(Delta);
 * // Show "+0.352" in green or "-1.204" in red
 *
 * // Get next checkpoint for navigation arrow
 * FVector NextLocation = Checkpoints->GetNextCheckpointLocation();
 * float Distance = Checkpoints->GetDistanceToNextCheckpoint(PlayerLocation);
 *
 * // Listen for race events
 * Checkpoints->OnCheckpointPassed.AddDynamic(this, &AMyRace::HandleCheckpoint);
 * Checkpoints->OnLapCompleted.AddDynamic(this, &AMyRace::HandleLapComplete);
 * Checkpoints->OnRaceFinished.AddDynamic(this, &AMyRace::HandleRaceFinish);
 * Checkpoints->OnWrongWay.AddDynamic(this, &AMyRace::HandleWrongWay);
 *
 * void AMyRace::HandleCheckpoint(const FMGCheckpointPassage& Passage,
 *                                 int32 Remaining, float Delta)
 * {
 *     // Show split time popup
 *     ShowSplitTime(Passage.SplitTime, Delta);
 *
 *     // Award points if speed bonus earned
 *     if (Passage.bWasSpeedBonus)
 *     {
 *         ShowSpeedBonus(Passage.PointsEarned);
 *     }
 * }
 *
 * void AMyRace::HandleLapComplete(const FMGLapData& LapData,
 *                                  int32 LapsRemaining, bool bIsBest)
 * {
 *     if (bIsBest)
 *     {
 *         ShowNewBestLap(LapData.LapTime);
 *     }
 *     ShowLapTime(LapData.LapTime, LapsRemaining);
 * }
 *
 * void AMyRace::HandleWrongWay(bool bIsWrongWay)
 * {
 *     if (bIsWrongWay)
 *     {
 *         ShowWrongWayWarning();
 *     }
 *     else
 *     {
 *         HideWrongWayWarning();
 *     }
 * }
 *
 * // Set target times for ghost comparison
 * Checkpoints->SetTargetTimes(GhostSplitTimes, GhostLapTime);
 *
 * // End of session, save best times
 * Checkpoints->SaveBestTimes("Downtown_Circuit_Main");
 * @endcode
 *
 * @section Architecture Architecture Notes
 *
 * LAYOUT MANAGEMENT:
 * - RegisterLayout() adds layouts to database
 * - LoadLayout() activates a layout for racing
 * - Layouts can be defined in data assets or code
 * - Multiple layouts per track (full circuit, short circuit, etc.)
 *
 * DETECTION ALGORITHM:
 * - UpdateCheckpointDetection() runs each frame
 * - Checks if player is inside next checkpoint trigger
 * - Validates direction if bRequiresDirection is true
 * - Fires OnCheckpointPassed or OnCheckpointInvalid
 *
 * TIMING PRECISION:
 * - Uses high-resolution game time
 * - TickRace() updates all timing counters
 * - Split times recorded at checkpoint passage
 *
 * STATE PERSISTENCE:
 * - BestTimesRecords stores personal bests
 * - SaveCheckpointData() / LoadCheckpointData()
 * - Per-layout, per-vehicle times possible
 *
 * EVENTS/DELEGATES:
 * - OnCheckpointPassed: Valid checkpoint crossed
 * - OnCheckpointMissed: Required checkpoint skipped
 * - OnCheckpointInvalid: Wrong direction or order
 * - OnLapCompleted: Full lap finished
 * - OnSectorCompleted: Sector time recorded
 * - OnNewBestLap: New personal best lap
 * - OnNewBestSector: New personal best sector
 * - OnTimeExtension: Time added (arcade mode)
 * - OnTimeExpired: Clock ran out
 * - OnRaceFinished: All laps complete
 * - OnWrongWay: Direction changed
 * - OnApproachingCheckpoint: Near next checkpoint
 *
 * @see UMGRaceSubsystem - Overall race management
 * @see UMGGhostSubsystem - Ghost replay for time comparison
 * @see UMGLeaderboardSubsystem - Best times storage
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/MGSharedTypes.h"
#include "MGCheckpointSubsystem.generated.h"

// MOVED TO MGSharedTypes.h
// /**
//  * Checkpoint type
//  */
// UENUM(BlueprintType)
// enum class EMGCheckpointType : uint8
// {
// 	Standard		UMETA(DisplayName = "Standard"),
// 	Start			UMETA(DisplayName = "Start"),
// 	Finish			UMETA(DisplayName = "Finish"),
// 	Lap				UMETA(DisplayName = "Lap/Sector"),
// 	Split			UMETA(DisplayName = "Split Time"),
// 	Secret			UMETA(DisplayName = "Secret"),
// 	Bonus			UMETA(DisplayName = "Bonus"),
// 	Mandatory		UMETA(DisplayName = "Mandatory"),
// 	Optional		UMETA(DisplayName = "Optional"),
// 	TimeExtension	UMETA(DisplayName = "Time Extension")
// };

/**
 * Checkpoint state during race.
 * Tracks the current status of each checkpoint in the active layout.
 */
UENUM(BlueprintType)
enum class EMGCheckpointState : uint8
{
	/** Checkpoint is not part of current race or not yet relevant */
	Inactive		UMETA(DisplayName = "Inactive"),
	/** The next checkpoint the player must pass */
	Active			UMETA(DisplayName = "Active"),
	/** Soon to be active (used for UI preview/minimap) */
	Upcoming		UMETA(DisplayName = "Upcoming"),
	/** Successfully crossed in correct direction */
	Passed			UMETA(DisplayName = "Passed"),
	/** Skipped without passing (may invalidate lap) */
	Missed			UMETA(DisplayName = "Missed"),
	/** Crossed in wrong direction or wrong order */
	Invalid			UMETA(DisplayName = "Invalid")
};

/**
 * Checkpoint trigger volume shape.
 * Determines the geometry used for checkpoint collision detection.
 */
UENUM(BlueprintType)
enum class EMGCheckpointShape : uint8
{
	/** Rectangular box trigger (most common for wide checkpoints) */
	Box				UMETA(DisplayName = "Box"),
	/** Spherical trigger volume */
	Sphere			UMETA(DisplayName = "Sphere"),
	/** Thin plane for precise timing measurements */
	Plane			UMETA(DisplayName = "Plane"),
	/** Cylindrical trigger volume */
	Cylinder		UMETA(DisplayName = "Cylinder"),
	/** Custom geometry defined by mesh collision */
	Custom			UMETA(DisplayName = "Custom")
};

/**
 * Checkpoint definition data.
 * Defines a single checkpoint's properties, location, trigger shape, and behavior rules.
 */
USTRUCT(BlueprintType)
struct FMGCheckpointDefinition
{
	GENERATED_BODY()

	/** Unique identifier for this checkpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString CheckpointId;

	/** Sequential index in the track layout (0-based) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", meta = (ClampMin = "0"))
	int32 Index = 0;

	/** Checkpoint function type (Standard, Start, Finish, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	EMGCheckpointType Type = EMGCheckpointType::Standard;

	/** Trigger volume geometry shape */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	EMGCheckpointShape Shape = EMGCheckpointShape::Plane;

	/** World location of checkpoint center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FVector Location = FVector::ZeroVector;

	/** World rotation of checkpoint (affects direction validation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transform")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Box/Plane extents (X=depth, Y=width, Z=height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (EditCondition = "Shape == EMGCheckpointShape::Box || Shape == EMGCheckpointShape::Plane"))
	FVector Extents = FVector(10.0f, 50.0f, 50.0f);

	/** Sphere/Cylinder radius in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger", meta = (ClampMin = "1.0", EditCondition = "Shape == EMGCheckpointShape::Sphere || Shape == EMGCheckpointShape::Cylinder"))
	float Radius = 25.0f;

	/** If true, checkpoints must be passed in sequential order */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bMustPassInOrder = true;

	/** If true, checkpoint can be crossed multiple times per lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bCanPassMultipleTimes = false;

	/** If true, validates player is traveling in correct direction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction")
	bool bRequiresDirection = true;

	/** Expected travel direction when crossing (normalized) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (EditCondition = "bRequiresDirection"))
	FVector RequiredDirection = FVector::ForwardVector;

	/** Allowed angle deviation from required direction in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Direction", meta = (ClampMin = "0.0", ClampMax = "180.0", EditCondition = "bRequiresDirection"))
	float DirectionTolerance = 90.0f;

	/** Seconds added to clock when crossed (arcade time-attack mode) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0.0"))
	float TimeExtensionSeconds = 0.0f;

	/** Base points awarded for crossing this checkpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0"))
	int32 BonusPoints = 0;

	/** Minimum speed (cm/s) required for speed bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0.0"))
	float SpeedBonusThreshold = 0.0f;

	/** Extra points awarded if crossing above speed threshold */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring", meta = (ClampMin = "0"))
	int32 SpeedBonusPoints = 0;

	/** Sector this checkpoint belongs to (for split timing) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	FString SectorName;

	/** Visual color for UI/debug representation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor CheckpointColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Optional mesh/particle asset for checkpoint visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UObject> VisualAsset;
};

/**
 * Checkpoint passage record.
 * Captures all data when a player crosses a checkpoint, used for timing and replay.
 */
USTRUCT(BlueprintType)
struct FMGCheckpointPassage
{
	GENERATED_BODY()

	/** ID of the checkpoint that was crossed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	FString CheckpointId;

	/** Index of checkpoint in the layout sequence */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	/** Time since lap started when crossed (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float PassageTime = 0.0f;

	/** Cumulative split time at this checkpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float SplitTime = 0.0f;

	/** Delta from personal best at this checkpoint (+/-) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float DeltaFromBest = 0.0f;

	/** Delta from target/ghost time at this checkpoint (+/-) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float DeltaFromTarget = 0.0f;

	/** Player speed (cm/s) when crossing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float Speed = 0.0f;

	/** Race position when crossing (multiplayer) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	int32 Position = 0;

	/** Points earned from this passage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 PointsEarned = 0;

	/** True if speed bonus threshold was exceeded */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	bool bWasSpeedBonus = false;

	/** Real-world timestamp of passage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FDateTime Timestamp;
};

/**
 * Lap completion data.
 * Complete record of a finished lap including timing, sectors, and validity.
 */
USTRUCT(BlueprintType)
struct FMGLapData
{
	GENERATED_BODY()

	/** Which lap this is (1-indexed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lap")
	int32 LapNumber = 0;

	/** Total lap time in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float LapTime = 0.0f;

	/** Delta from personal best lap (+/-) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float DeltaFromBest = 0.0f;

	/** Time for each sector (S1, S2, S3, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<float> SectorTimes;

	/** Every checkpoint passage during this lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
	TArray<FMGCheckpointPassage> Passages;

	/** True if all mandatory checkpoints were passed correctly */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validity")
	bool bIsValid = true;

	/** True if this lap set a new personal best */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validity")
	bool bIsBestLap = false;

	/** Number of mandatory checkpoints that were skipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validity")
	int32 CheckpointsMissed = 0;

	/** Number of checkpoints passed in wrong direction/order */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validity")
	int32 InvalidPassages = 0;
};

/**
 * Sector definition for track timing.
 * Defines a timed section of the track between checkpoints (typically S1, S2, S3).
 */
USTRUCT(BlueprintType)
struct FMGSectorDefinition
{
	GENERATED_BODY()

	/** Unique sector identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString SectorId;

	/** Display name for UI (e.g., "Sector 1", "Tunnel Section") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText SectorName;

	/** First checkpoint index in this sector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds", meta = (ClampMin = "0"))
	int32 StartCheckpointIndex = 0;

	/** Last checkpoint index in this sector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds", meta = (ClampMin = "0"))
	int32 EndCheckpointIndex = 0;

	/** Personal best time for this sector (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BestTime = 0.0f;

	/** Target/ghost time for comparison (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TargetTime = 0.0f;

	/** UI color for sector display (green=PB, purple=record) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	FLinearColor SectorColor = FLinearColor::White;
};

/**
 * Track checkpoint layout configuration.
 * Complete checkpoint configuration for a track variant (full circuit, short layout, etc.).
 */
USTRUCT(BlueprintType)
struct FMGCheckpointLayout
{
	GENERATED_BODY()

	/** Unique layout identifier (e.g., "Downtown_Circuit_Full") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString LayoutId;

	/** Parent track this layout belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString TrackId;

	/** Display name for UI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FText LayoutName;

	/** Ordered array of all checkpoints in this layout */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoints")
	TArray<FMGCheckpointDefinition> Checkpoints;

	/** Sector definitions for split timing */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	TArray<FMGSectorDefinition> Sectors;

	/** Default number of laps for circuit races */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race", meta = (ClampMin = "1"))
	int32 TotalLaps = 1;

	/** True for closed circuits, false for point-to-point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsCircuit = true;

	/** If true, allows track cutting without penalty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
	bool bAllowCutting = false;

	/** Maximum checkpoints that can be missed before lap invalidation (0 = strict) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules", meta = (ClampMin = "0"))
	int32 MaxMissedCheckpoints = 0;

	/** Total track length in cm (for distance calculations) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata", meta = (ClampMin = "0.0"))
	float TrackLength = 0.0f;
};

/**
 * Active race checkpoint state.
 * Real-time snapshot of current race progress, timing, and statistics.
 */
USTRUCT(BlueprintType)
struct FMGActiveCheckpointState
{
	GENERATED_BODY()

	/** Currently loaded layout ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FString LayoutId;

	/** Index of next checkpoint to pass */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentCheckpoint = 0;

	/** Current lap number (1-indexed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentLap = 1;

	/** Current sector index (0-indexed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentSector = 0;

	/** Time elapsed in current lap (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float CurrentLapTime = 0.0f;

	/** Time elapsed in current sector (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float CurrentSectorTime = 0.0f;

	/** Total race time elapsed (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float TotalRaceTime = 0.0f;

	/** Best lap time this session (seconds, 0 if none) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float BestLapTime = 0.0f;

	/** Total checkpoints successfully passed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 CheckpointsPassed = 0;

	/** Total checkpoints missed/skipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	int32 CheckpointsMissed = 0;

	/** Accumulated score from checkpoints */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	int32 TotalPoints = 0;

	/** Remaining time in time-attack mode (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeAttack")
	float TimeRemaining = 0.0f;

	/** True if race has a time limit (arcade mode) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeAttack")
	bool bHasTimeLimit = false;

	/** Data for all completed laps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laps")
	TArray<FMGLapData> CompletedLaps;

	/** Data being accumulated for current in-progress lap */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Laps")
	FMGLapData CurrentLapData;
};

/**
 * Personal best times record for a layout.
 * Stored persistently to track player improvement across sessions.
 */
USTRUCT(BlueprintType)
struct FMGBestTimesRecord
{
	GENERATED_BODY()

	/** Layout these records belong to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FString LayoutId;

	/** Best single lap time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float BestLapTime = 0.0f;

	/** Best total race time (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	float BestRaceTime = 0.0f;

	/** Best time for each sector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	TArray<float> BestSectorTimes;

	/** Best split time at each checkpoint */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Records")
	TArray<float> BestSplitTimes;

	/** When record was set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Metadata")
	FDateTime RecordDate;
};

// ============================================================================
// Delegates
// ============================================================================

/** Broadcast when player successfully crosses a checkpoint */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointPassed, const FMGCheckpointPassage&, Passage, int32, CheckpointsRemaining, float, DeltaTime);

/** Broadcast when player skips a mandatory checkpoint */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointMissed, const FString&, CheckpointId, int32, TotalMissed);

/** Broadcast when checkpoint crossed in wrong direction or order */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointInvalid, const FString&, CheckpointId, const FString&, Reason);

/** Broadcast when a lap is completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLapCompleted, const FMGLapData&, LapData, int32, LapsRemaining, bool, bIsBestLap);

/** Broadcast when a sector is completed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectorCompleted, int32, SectorIndex, float, SectorTime);

/** Broadcast when a new personal best lap time is set */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBestLap, float, OldBest, float, NewBest);

/** Broadcast when a new personal best sector time is set */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBestSector, int32, SectorIndex, float, SectorTime);

/** Broadcast when time is added in arcade mode */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeExtension, float, SecondsAdded, float, NewTimeRemaining);

/** Broadcast when time runs out in time-attack mode */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeExpired);

/** Broadcast when race is completed (all laps finished) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceFinished, float, TotalTime, int32, TotalPoints);

/** Broadcast when wrong-way state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWrongWay, bool, bIsWrongWay);

/** Broadcast when player approaches next checkpoint (for UI hints) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnApproachingCheckpoint, const FString&, CheckpointId, float, Distance);

/**
 * Checkpoint Subsystem
 * Manages race checkpoints, laps, sectors, and timing
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCheckpointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnCheckpointPassed OnCheckpointPassed;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnCheckpointMissed OnCheckpointMissed;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnCheckpointInvalid OnCheckpointInvalid;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnLapCompleted OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnSectorCompleted OnSectorCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnNewBestLap OnNewBestLap;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnNewBestSector OnNewBestSector;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnTimeExtension OnTimeExtension;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnTimeExpired OnTimeExpired;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnRaceFinished OnRaceFinished;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnWrongWay OnWrongWay;

	UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
	FOnApproachingCheckpoint OnApproachingCheckpoint;

	// Layout Management
	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Layout")
	bool RegisterLayout(const FMGCheckpointLayout& Layout);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Layout")
	FMGCheckpointLayout GetLayout(const FString& LayoutId) const;

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Layout")
	TArray<FMGCheckpointLayout> GetLayoutsForTrack(const FString& TrackId) const;

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Layout")
	bool LoadLayout(const FString& LayoutId);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Layout")
	void UnloadLayout();

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Layout")
	bool IsLayoutLoaded() const;

	// Race Control
	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Race")
	void StartRace(int32 TotalLaps, float TimeLimit = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Race")
	void StopRace();

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Race")
	void PauseRace();

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Race")
	void ResumeRace();

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Race")
	void ResetRace();

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Race")
	bool IsRaceActive() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Race")
	bool IsRacePaused() const;

	// Checkpoint Detection
	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Detection")
	bool TryPassCheckpoint(FVector PlayerLocation, FVector PlayerVelocity);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Detection")
	void UpdateCheckpointDetection(FVector PlayerLocation, FVector PlayerVelocity, float MGDeltaTime);

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Detection")
	bool IsInCheckpointTrigger(FVector Location, const FMGCheckpointDefinition& Checkpoint) const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Detection")
	bool IsValidPassageDirection(FVector Velocity, const FMGCheckpointDefinition& Checkpoint) const;

	// State Queries
	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	FMGActiveCheckpointState GetActiveState() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	int32 GetCurrentCheckpointIndex() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	int32 GetCurrentLap() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	int32 GetCurrentSector() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	float GetCurrentLapTime() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	float GetTotalRaceTime() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	float GetTimeRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	int32 GetLapsRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|State")
	int32 GetCheckpointsRemaining() const;

	// Checkpoint Info
	UFUNCTION(BlueprintPure, Category = "Checkpoint|Info")
	FMGCheckpointDefinition GetCheckpoint(int32 Index) const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Info")
	FMGCheckpointDefinition GetNextCheckpoint() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Info")
	float GetDistanceToNextCheckpoint(FVector PlayerLocation) const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Info")
	FVector GetNextCheckpointLocation() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Info")
	EMGCheckpointState GetCheckpointState(int32 Index) const;

	// Timing
	UFUNCTION(BlueprintPure, Category = "Checkpoint|Timing")
	float GetBestLapTime() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Timing")
	float GetBestSectorTime(int32 SectorIndex) const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Timing")
	float GetCurrentDelta() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Timing")
	TArray<float> GetCurrentSplitTimes() const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Timing")
	FMGLapData GetBestLapData() const;

	// Best Times Management
	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Records")
	void SetTargetTimes(const TArray<float>& SplitTimes, float LapTime);

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Records")
	FMGBestTimesRecord GetBestTimesRecord(const FString& LayoutId) const;

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Records")
	void SaveBestTimes(const FString& LayoutId);

	// Wrong Way Detection
	UFUNCTION(BlueprintPure, Category = "Checkpoint|WrongWay")
	bool IsGoingWrongWay(FVector PlayerVelocity) const;

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|WrongWay")
	void UpdateWrongWayDetection(FVector PlayerVelocity);

	// Utility
	UFUNCTION(BlueprintPure, Category = "Checkpoint|Utility")
	FText FormatTime(float TimeSeconds) const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Utility")
	FText FormatDelta(float DeltaSeconds) const;

	UFUNCTION(BlueprintPure, Category = "Checkpoint|Utility")
	FLinearColor GetDeltaColor(float DeltaSeconds) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Persistence")
	void SaveCheckpointData();

	UFUNCTION(BlueprintCallable, Category = "Checkpoint|Persistence")
	void LoadCheckpointData();

protected:
	// ========================================================================
	// Internal Processing
	// ========================================================================

	/** Called each frame to update race timing */
	void TickRace(float MGDeltaTime);

	/** Handles checkpoint passage logic and scoring */
	void ProcessCheckpointPass(int32 CheckpointIndex, FVector Velocity);

	/** Handles lap completion, best time checks, and lap data finalization */
	void ProcessLapComplete();

	/** Handles sector completion and sector time recording */
	void ProcessSectorComplete();

	/** Checks if time limit has been exceeded (arcade mode) */
	void CheckTimeExpired();

	/** Updates best times if current times beat records */
	void UpdateBestTimes();

	/** Returns sector index containing the given checkpoint */
	int32 GetSectorForCheckpoint(int32 CheckpointIndex) const;

private:
	// ========================================================================
	// Layout Database
	// ========================================================================

	/** All registered checkpoint layouts (LayoutId -> Layout) */
	UPROPERTY()
	TMap<FString, FMGCheckpointLayout> RegisteredLayouts;

	/** Personal best records per layout (LayoutId -> Record) */
	UPROPERTY()
	TMap<FString, FMGBestTimesRecord> BestTimesRecords;

	// ========================================================================
	// Active Race State
	// ========================================================================

	/** Currently loaded and active layout */
	UPROPERTY()
	FMGCheckpointLayout ActiveLayout;

	/** Current race progress and timing state */
	UPROPERTY()
	FMGActiveCheckpointState ActiveState;

	/** Target split times for delta comparison (from ghost/best) */
	UPROPERTY()
	TArray<float> TargetSplitTimes;

	/** Target lap time for delta comparison */
	UPROPERTY()
	float TargetLapTime = 0.0f;

	// ========================================================================
	// Flags
	// ========================================================================

	/** True if a layout is currently loaded */
	UPROPERTY()
	bool bLayoutLoaded = false;

	/** True if race is in progress */
	UPROPERTY()
	bool bRaceActive = false;

	/** True if race is paused */
	UPROPERTY()
	bool bRacePaused = false;

	/** Previous wrong-way state for change detection */
	UPROPERTY()
	bool bWasWrongWay = false;

	/** Time spent going wrong way (for hysteresis) */
	UPROPERTY()
	float WrongWayTimer = 0.0f;

	/** Timer handle for race tick updates */
	FTimerHandle RaceTickTimer;
};

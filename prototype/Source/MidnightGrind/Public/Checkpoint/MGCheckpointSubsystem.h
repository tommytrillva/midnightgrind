// Copyright Epic Games, Inc. All Rights Reserved.

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

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCheckpointSubsystem.generated.h"

/**
 * Checkpoint type
 */
UENUM(BlueprintType)
enum class EMGCheckpointType : uint8
{
	Standard		UMETA(DisplayName = "Standard"),
	Start			UMETA(DisplayName = "Start"),
	Finish			UMETA(DisplayName = "Finish"),
	Lap				UMETA(DisplayName = "Lap/Sector"),
	Split			UMETA(DisplayName = "Split Time"),
	Secret			UMETA(DisplayName = "Secret"),
	Bonus			UMETA(DisplayName = "Bonus"),
	Mandatory		UMETA(DisplayName = "Mandatory"),
	Optional		UMETA(DisplayName = "Optional"),
	TimeExtension	UMETA(DisplayName = "Time Extension")
};

/**
 * Checkpoint state
 */
UENUM(BlueprintType)
enum class EMGCheckpointState : uint8
{
	Inactive		UMETA(DisplayName = "Inactive"),
	Active			UMETA(DisplayName = "Active"),
	Upcoming		UMETA(DisplayName = "Upcoming"),
	Passed			UMETA(DisplayName = "Passed"),
	Missed			UMETA(DisplayName = "Missed"),
	Invalid			UMETA(DisplayName = "Invalid")
};

/**
 * Checkpoint trigger shape
 */
UENUM(BlueprintType)
enum class EMGCheckpointShape : uint8
{
	Box				UMETA(DisplayName = "Box"),
	Sphere			UMETA(DisplayName = "Sphere"),
	Plane			UMETA(DisplayName = "Plane"),
	Cylinder		UMETA(DisplayName = "Cylinder"),
	Custom			UMETA(DisplayName = "Custom")
};

/**
 * Checkpoint definition
 */
USTRUCT(BlueprintType)
struct FMGCheckpointDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CheckpointId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Index = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckpointType Type = EMGCheckpointType::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckpointShape Shape = EMGCheckpointShape::Plane;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Extents = FVector(10.0f, 50.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMustPassInOrder = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanPassMultipleTimes = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresDirection = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RequiredDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DirectionTolerance = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeExtensionSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedBonusThreshold = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpeedBonusPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SectorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CheckpointColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VisualAsset;
};

/**
 * Checkpoint passage record
 */
USTRUCT(BlueprintType)
struct FMGCheckpointPassage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CheckpointId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckpointIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PassageTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SplitTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaFromBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaFromTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasSpeedBonus = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Lap data
 */
USTRUCT(BlueprintType)
struct FMGLapData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeltaFromBest = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> SectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCheckpointPassage> Passages;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBestLap = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckpointsMissed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InvalidPassages = 0;
};

/**
 * Sector definition
 */
USTRUCT(BlueprintType)
struct FMGSectorDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SectorId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SectorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartCheckpointIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EndCheckpointIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SectorColor = FLinearColor::White;
};

/**
 * Track checkpoint layout
 */
USTRUCT(BlueprintType)
struct FMGCheckpointLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LayoutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText LayoutName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCheckpointDefinition> Checkpoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGSectorDefinition> Sectors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalLaps = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCircuit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCutting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxMissedCheckpoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackLength = 0.0f;
};

/**
 * Active race checkpoint state
 */
USTRUCT(BlueprintType)
struct FMGActiveCheckpointState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LayoutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentCheckpoint = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentLap = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentSector = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentSectorTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRaceTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckpointsPassed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckpointsMissed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasTimeLimit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLapData> CompletedLaps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLapData CurrentLapData;
};

/**
 * Best times record
 */
USTRUCT(BlueprintType)
struct FMGBestTimesRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LayoutId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestLapTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestRaceTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> BestSectorTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> BestSplitTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime RecordDate;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointPassed, const FMGCheckpointPassage&, Passage, int32, CheckpointsRemaining, float, DeltaTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointMissed, const FString&, CheckpointId, int32, TotalMissed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointInvalid, const FString&, CheckpointId, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLapCompleted, const FMGLapData&, LapData, int32, LapsRemaining, bool, bIsBestLap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSectorCompleted, int32, SectorIndex, float, SectorTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBestLap, float, OldBest, float, NewBest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBestSector, int32, SectorIndex, float, SectorTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimeExtension, float, SecondsAdded, float, NewTimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeExpired);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceFinished, float, TotalTime, int32, TotalPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWrongWay, bool, bIsWrongWay);
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
	void UpdateCheckpointDetection(FVector PlayerLocation, FVector PlayerVelocity, float DeltaTime);

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
	void TickRace(float DeltaTime);
	void ProcessCheckpointPass(int32 CheckpointIndex, FVector Velocity);
	void ProcessLapComplete();
	void ProcessSectorComplete();
	void CheckTimeExpired();
	void UpdateBestTimes();
	int32 GetSectorForCheckpoint(int32 CheckpointIndex) const;

private:
	UPROPERTY()
	TMap<FString, FMGCheckpointLayout> RegisteredLayouts;

	UPROPERTY()
	TMap<FString, FMGBestTimesRecord> BestTimesRecords;

	UPROPERTY()
	FMGCheckpointLayout ActiveLayout;

	UPROPERTY()
	FMGActiveCheckpointState ActiveState;

	UPROPERTY()
	TArray<float> TargetSplitTimes;

	UPROPERTY()
	float TargetLapTime = 0.0f;

	UPROPERTY()
	bool bLayoutLoaded = false;

	UPROPERTY()
	bool bRaceActive = false;

	UPROPERTY()
	bool bRacePaused = false;

	UPROPERTY()
	bool bWasWrongWay = false;

	UPROPERTY()
	float WrongWayTimer = 0.0f;

	FTimerHandle RaceTickTimer;
};

/**
 * @file MGDragRaceHandler.h
 * @brief Drag Race Handler - Quarter-mile straight-line acceleration racing
 *
 * Drag racing is pure acceleration competition. Two cars line up side-by-side
 * and race in a straight line, typically for a quarter mile (1320 feet). The
 * focus is on reaction time off the line and perfect gear shifts.
 *
 * @section overview Overview
 * Drag racing originated at drag strips and became a staple of street racing
 * culture. This handler implements authentic drag racing mechanics including
 * the "Christmas tree" countdown, reaction time measurement, interval timing,
 * and red light (false start) detection.
 *
 * @section win_condition Win Condition
 * First car across the finish line wins, UNLESS they jumped the start (red light).
 * A red-lighted car automatically loses, regardless of their elapsed time.
 *
 * @section christmas_tree The Christmas Tree
 * The iconic starting system with staged lights:
 * 1. **Pre-Stage**: Car approaches the line
 * 2. **Staged**: First beam broken (pre-stage light)
 * 3. **Fully Staged**: Second beam broken (ready)
 * 4. **Tree Dropping**: Amber lights countdown
 * 5. **Green**: GO! (reaction time starts here)
 * 6. **Red Light**: Jumped before green (disqualified)
 *
 * @section timing Timing Points
 * Standard drag racing interval times are tracked:
 * - **60-foot**: Launch quality indicator
 * - **330-foot**: Early acceleration
 * - **660-foot**: Eighth-mile (half track)
 * - **1000-foot**: Late acceleration
 * - **1320-foot**: Quarter-mile finish (with trap speed)
 *
 * @section tree_types Tree Types
 * - **Sportsman Tree**: 0.5 seconds between amber lights (3 ambers)
 * - **Pro Tree**: 0.4 seconds, all ambers flash simultaneously
 *
 * @see UMGRaceTypeHandler Base class
 * @see EMGLaunchState Christmas tree state machine
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGDragRaceHandler.generated.h"

// ============================================================================
// LAUNCH AND SHIFT ENUMS
// ============================================================================

/**
 * @brief States of the Christmas tree staging sequence
 *
 * This enum tracks the progression through the drag race start sequence,
 * from approaching the line through launch.
 */
UENUM(BlueprintType)
enum class EMGLaunchState : uint8
{
	/** Car not yet at staging beams */
	PreStage,
	/** First staging beam broken (yellow pre-stage light) */
	Staged,
	/** Both staging beams broken (both yellow lights lit, ready to race) */
	FullyStaged,
	/** Amber lights are dropping (countdown in progress) */
	TreeDropping,
	/** Green light! Race has started */
	Green,
	/** Red light - car left too early (false start / foul) */
	RedLight
};

/**
 * @brief Quality rating for reaction time off the line
 *
 * Based on how quickly the driver reacted after the green light.
 * Negative reaction times result in a red light (foul).
 */
UENUM(BlueprintType)
enum class EMGLaunchQuality : uint8
{
	/** Left before green - automatic loss */
	RedLight		UMETA(DisplayName = "Red Light"),
	/** Reaction > 0.3 seconds */
	Poor			UMETA(DisplayName = "Poor"),
	/** Reaction 0.2 - 0.3 seconds */
	Average			UMETA(DisplayName = "Average"),
	/** Reaction 0.1 - 0.2 seconds */
	Good			UMETA(DisplayName = "Good"),
	/** Reaction 0.05 - 0.1 seconds */
	Great			UMETA(DisplayName = "Great"),
	/** Reaction 0.02 - 0.05 seconds */
	Perfect			UMETA(DisplayName = "Perfect"),
	/** Reaction < 0.02 seconds (nearly impossible) */
	Holeshot		UMETA(DisplayName = "Holeshot!")
};

/**
 * @brief Quality rating for gear shifts
 *
 * Tracks how well the driver timed their gear shifts relative
 * to the optimal RPM (redline).
 */
UENUM(BlueprintType)
enum class EMGShiftQuality : uint8
{
	/** Shift failed completely (money shift) */
	Missed,
	/** Shifted too early - left power on the table */
	Early,
	/** Shifted too late - hit the rev limiter */
	Late,
	/** Good shift timing */
	Good,
	/** Perfect shift at peak power */
	Perfect
};

// ============================================================================
// DRAG RACE DATA STRUCT
// ============================================================================

/**
 * @brief Complete data for a drag race participant
 *
 * Tracks all timing, performance, and state data for a single
 * racer during a drag race.
 */
USTRUCT(BlueprintType)
struct FMGDragRacerData
{
	GENERATED_BODY()

	// === Launch State ===

	/** Current position in the staging/launch sequence */
	UPROPERTY(BlueprintReadOnly)
	EMGLaunchState LaunchState = EMGLaunchState::PreStage;

	/** Quality of the launch based on reaction time */
	UPROPERTY(BlueprintReadOnly)
	EMGLaunchQuality LaunchQuality = EMGLaunchQuality::Average;

	/**
	 * Time from green light to car movement (seconds)
	 * Negative = red light (left before green)
	 */
	UPROPERTY(BlueprintReadOnly)
	float ReactionTime = 0.0f;

	// === Interval Times ===
	// These are cumulative times from the green light

	/** Time to reach 60 feet - indicates launch quality */
	UPROPERTY(BlueprintReadOnly)
	float SixtyFootTime = 0.0f;

	/** Time to reach 330 feet - early acceleration */
	UPROPERTY(BlueprintReadOnly)
	float ThreeThirtyTime = 0.0f;

	/** Time to reach 660 feet (1/8 mile) - half track */
	UPROPERTY(BlueprintReadOnly)
	float EighthMileTime = 0.0f;

	/** Time to reach 1000 feet - late acceleration */
	UPROPERTY(BlueprintReadOnly)
	float ThousandFootTime = 0.0f;

	/** Time to reach 1320 feet (1/4 mile) - finish line */
	UPROPERTY(BlueprintReadOnly)
	float QuarterMileTime = 0.0f;

	// === Speeds ===

	/** Speed when crossing 1/8 mile mark (mph) */
	UPROPERTY(BlueprintReadOnly)
	float EighthMileSpeed = 0.0f;

	/** Speed when crossing finish line (mph) - "trap speed" */
	UPROPERTY(BlueprintReadOnly)
	float TrapSpeed = 0.0f;

	// === Progress ===

	/** Current distance from start line (feet) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentDistance = 0.0f;

	// === Shift Performance ===

	/** Total gear shifts performed */
	UPROPERTY(BlueprintReadOnly)
	int32 ShiftCount = 0;

	/** Number of perfect shifts achieved */
	UPROPERTY(BlueprintReadOnly)
	int32 PerfectShifts = 0;

	// === Completion State ===

	/** Has this racer crossed the finish line? */
	UPROPERTY(BlueprintReadOnly)
	bool bFinished = false;

	/** Did this racer jump the start (red light)? */
	UPROPERTY(BlueprintReadOnly)
	bool bRedLight = false;
};

// ============================================================================
// DRAG RACE EVENT DELEGATES
// ============================================================================

/** Broadcast when a racer's staging state changes */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLaunchStateChanged, int32, RacerIndex, EMGLaunchState, NewState);

/** Broadcast when a racer's reaction time is recorded (at launch) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReactionTimeRecorded, int32, RacerIndex, float, ReactionTime, EMGLaunchQuality, Quality);

/** Broadcast when a racer crosses an interval timing point */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnIntervalTime, int32, RacerIndex, float, Distance, float, Time);

/** Broadcast when a racer performs a gear shift */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShiftPerformed, int32, RacerIndex, EMGShiftQuality, Quality);

/** Broadcast when a racer commits a red light foul */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRedLight, int32, RacerIndex);

// ============================================================================
// DRAG RACE HANDLER CLASS
// ============================================================================

/**
 * @brief Handler for drag (quarter-mile) racing
 *
 * Implements authentic drag racing with Christmas tree start, reaction time
 * measurement, interval timing, and trap speed calculation. Supports both
 * sportsman (cascading amber) and pro (instant amber) tree configurations.
 *
 * @section typical_flow Typical Race Flow
 * 1. Cars roll into staging area
 * 2. Each car breaks pre-stage beam (Staged)
 * 3. Each car breaks stage beam (FullyStaged)
 * 4. When both staged, tree sequence begins
 * 5. Amber lights drop (0.4 or 0.5s intervals)
 * 6. Green light - race begins
 * 7. First clean finish wins (red lights lose)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGDragRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGDragRaceHandler();

	// ==========================================
	// RACE TYPE HANDLER INTERFACE OVERRIDES
	// ==========================================

	//~ Begin UMGRaceTypeHandler Interface

	/** Set up staging and timing systems */
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;

	/** Clear all timing data and reset tree state */
	virtual void Reset() override;

	/** Begin the staging sequence (NOT the race itself) */
	virtual void OnCountdownStarted() override;

	/** Record green light time and enable racing */
	virtual void OnRaceStarted() override;

	/** Update tree sequence and racer progress */
	virtual void OnRaceTick(float DeltaTime) override;

	/** Check if racer crossed finish (and didn't red light) */
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;

	/** Position by finish time (red lights always last) */
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	/// Returns EMGRaceType::Drag
	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Drag; }

	/** Get localized display name "Drag Race" */
	virtual FText GetDisplayName() const override;

	/** Get description of drag racing rules */
	virtual FText GetDescription() const override;

	/// Drag races have no laps
	virtual bool ShouldShowLapCounter() const override { return false; }

	/// Drag races show position (lane comparison)
	virtual bool ShouldShowPosition() const override { return true; }

	/// Drag races are not score-based
	virtual bool ShouldShowScore() const override { return false; }

	/** Returns distance progress format */
	virtual FText GetProgressFormat() const override;

	/** Calculate credits based on finishing and reaction quality */
	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// CHRISTMAS TREE CONTROL
	// Methods for staging sequence
	// ==========================================

	/**
	 * @brief Start the Christmas tree sequence
	 * @note Called when both racers are fully staged
	 */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void StartChristmasTree();

	/**
	 * @brief Report that a racer has staged
	 * @param RacerIndex Index of the staging racer
	 * @param bFullyStaged True for second beam, false for first
	 */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void OnRacerStaged(int32 RacerIndex, bool bFullyStaged);

	/**
	 * @brief Get the current tree state
	 * @return Current state of the Christmas tree
	 */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	EMGLaunchState GetTreeState() const { return TreeState; }

	/**
	 * @brief Get time remaining until green light
	 * @return Seconds until green, or 0 if already green/not dropping
	 */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetTimeToGreen() const;

	// ==========================================
	// LAUNCH & TIMING
	// Methods for race timing data
	// ==========================================

	/**
	 * @brief Record when racer presses throttle
	 * @param RacerIndex Index of the launching racer
	 * @note Used to calculate reaction time
	 */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void OnLaunchInput(int32 RacerIndex);

	/**
	 * @brief Get complete drag data for a racer
	 * @param RacerIndex Index of the racer
	 * @return All timing and performance data
	 */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	FMGDragRacerData GetRacerDragData(int32 RacerIndex) const;

	/**
	 * @brief Get reaction time for a racer
	 * @param RacerIndex Index of the racer
	 * @return Reaction time in seconds (negative = red light)
	 */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetReactionTime(int32 RacerIndex) const;

	/**
	 * @brief Get elapsed time since green for a racer
	 * @param RacerIndex Index of the racer
	 * @return Elapsed time in seconds
	 */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetElapsedTime(int32 RacerIndex) const;

	/**
	 * @brief Get current distance from start for a racer
	 * @param RacerIndex Index of the racer
	 * @return Distance in feet
	 */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetCurrentDistance(int32 RacerIndex) const;

	// ==========================================
	// SHIFTING
	// Methods for manual transmission handling
	// ==========================================

	/**
	 * @brief Record a gear shift
	 * @param RacerIndex Index of the shifting racer
	 * @param RPMPercent Current RPM as percentage of redline (0-1)
	 */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void OnShift(int32 RacerIndex, float RPMPercent);

	// ==========================================
	// CONFIGURATION
	// Race setup methods
	// ==========================================

	/**
	 * @brief Set the track distance
	 * @param DistanceFeet Track length (default 1320 = quarter mile)
	 * @note 660 = eighth mile, 1320 = quarter mile
	 */
	UFUNCTION(BlueprintCallable, Category = "Drag Race|Config")
	void SetTrackDistance(float DistanceFeet) { TrackDistanceFeet = DistanceFeet; }

	/**
	 * @brief Set tree type (pro or sportsman)
	 * @param bPro True for pro tree (0.4s, instant), false for sportsman (0.5s, cascade)
	 */
	UFUNCTION(BlueprintCallable, Category = "Drag Race|Config")
	void SetProTree(bool bPro) { bUseProTree = bPro; }

	// ==========================================
	// EVENTS
	// Delegates for drag race events
	// ==========================================

	/** Broadcast when staging state changes */
	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnLaunchStateChanged OnLaunchStateChanged;

	/** Broadcast when reaction time is recorded */
	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnReactionTimeRecorded OnReactionTimeRecorded;

	/** Broadcast when crossing timing interval */
	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnIntervalTime OnIntervalTime;

	/** Broadcast on gear shift */
	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnShiftPerformed OnShiftPerformed;

	/** Broadcast on red light foul */
	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnRedLight OnRedLight;

protected:
	// ==========================================
	// TRACK CONFIGURATION
	// ==========================================

	/** Track length in feet (1320 = quarter mile, 660 = eighth mile) */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float TrackDistanceFeet = 1320.0f;

	/** Use pro tree timing (all ambers at once vs cascade) */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	bool bUseProTree = false;

	/** Get the interval between amber lights based on tree type */
	float GetTreeInterval() const { return bUseProTree ? 0.4f : 0.5f; }

	// ==========================================
	// REACTION TIME THRESHOLDS
	// Determines launch quality ratings
	// ==========================================

	/** Reaction time for "Perfect" rating (< this = Holeshot) */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float PerfectReactionThreshold = 0.02f;

	/** Reaction time for "Great" rating */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float GreatReactionThreshold = 0.05f;

	/** Reaction time for "Good" rating */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float GoodReactionThreshold = 0.1f;

	// ==========================================
	// TREE STATE
	// Christmas tree sequence tracking
	// ==========================================

	/** Current state of the Christmas tree */
	UPROPERTY()
	EMGLaunchState TreeState = EMGLaunchState::PreStage;

	/** Timer for tree sequence progression */
	float TreeTimer = 0.0f;

	/** Exact time when green light occurred */
	float GreenLightTime = 0.0f;

	/** Current amber light in sequence (0-2 for sportsman) */
	int32 CurrentAmberLight = 0;

	// ==========================================
	// RACER DATA
	// Per-participant tracking
	// ==========================================

	/** Complete drag race data for each racer */
	UPROPERTY()
	TMap<int32, FMGDragRacerData> RacerDragData;

	/**
	 * Interval distances for timing (in feet)
	 * Standard drag racing intervals
	 */
	TArray<float> IntervalDistances = { 60.0f, 330.0f, 660.0f, 1000.0f, 1320.0f };

	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Update the Christmas tree sequence each frame */
	void UpdateTreeSequence(float DeltaTime);

	/** Update a racer's position and check intervals */
	void UpdateRacerProgress(int32 RacerIndex, float DeltaTime);

	/** Check if racer crossed any interval timing points */
	void CheckIntervals(int32 RacerIndex, float OldDistance, float NewDistance);

	/** Convert Unreal units (cm) to feet */
	float CmToFeet(float Cm) const { return Cm * 0.0328084f; }

	/** Convert Unreal speed (cm/s) to mph */
	float CmsToMph(float Cms) const { return Cms * 0.0223694f; }

	/** Determine launch quality from reaction time */
	EMGLaunchQuality GetLaunchQuality(float ReactionTime) const;
};

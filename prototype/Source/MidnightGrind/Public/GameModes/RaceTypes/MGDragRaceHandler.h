// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameModes/RaceTypes/MGRaceTypeHandler.h"
#include "MGDragRaceHandler.generated.h"

/**
 * Launch state for drag racing start
 */
UENUM(BlueprintType)
enum class EMGLaunchState : uint8
{
	/** Waiting for staging */
	PreStage,
	/** Pre-staged (first light) */
	Staged,
	/** Staged (second light) */
	FullyStaged,
	/** Tree is dropping */
	TreeDropping,
	/** Green light - GO! */
	Green,
	/** Red light - jumped start */
	RedLight
};

/**
 * Launch quality rating
 */
UENUM(BlueprintType)
enum class EMGLaunchQuality : uint8
{
	/** Too early - red light */
	RedLight		UMETA(DisplayName = "Red Light"),
	/** Very slow reaction */
	Poor			UMETA(DisplayName = "Poor"),
	/** Below average */
	Average			UMETA(DisplayName = "Average"),
	/** Good reaction */
	Good			UMETA(DisplayName = "Good"),
	/** Great reaction */
	Great			UMETA(DisplayName = "Great"),
	/** Near perfect */
	Perfect			UMETA(DisplayName = "Perfect"),
	/** Exactly on green */
	Holeshot		UMETA(DisplayName = "Holeshot!")
};

/**
 * Shift quality for manual transmission
 */
UENUM(BlueprintType)
enum class EMGShiftQuality : uint8
{
	/** Missed shift */
	Missed,
	/** Early shift - lost power */
	Early,
	/** Late shift - hit limiter */
	Late,
	/** Good shift */
	Good,
	/** Perfect shift at redline */
	Perfect
};

/**
 * Per-racer drag race data
 */
USTRUCT(BlueprintType)
struct FMGDragRacerData
{
	GENERATED_BODY()

	/** Current launch state */
	UPROPERTY(BlueprintReadOnly)
	EMGLaunchState LaunchState = EMGLaunchState::PreStage;

	/** Launch quality */
	UPROPERTY(BlueprintReadOnly)
	EMGLaunchQuality LaunchQuality = EMGLaunchQuality::Average;

	/** Reaction time (seconds from green) */
	UPROPERTY(BlueprintReadOnly)
	float ReactionTime = 0.0f;

	/** 60-foot time */
	UPROPERTY(BlueprintReadOnly)
	float SixtyFootTime = 0.0f;

	/** 330-foot time */
	UPROPERTY(BlueprintReadOnly)
	float ThreeThirtyTime = 0.0f;

	/** 660-foot (1/8 mile) time */
	UPROPERTY(BlueprintReadOnly)
	float EighthMileTime = 0.0f;

	/** 1000-foot time */
	UPROPERTY(BlueprintReadOnly)
	float ThousandFootTime = 0.0f;

	/** Quarter mile (1320 feet) time */
	UPROPERTY(BlueprintReadOnly)
	float QuarterMileTime = 0.0f;

	/** Speed at 1/8 mile (mph) */
	UPROPERTY(BlueprintReadOnly)
	float EighthMileSpeed = 0.0f;

	/** Trap speed at finish (mph) */
	UPROPERTY(BlueprintReadOnly)
	float TrapSpeed = 0.0f;

	/** Current distance traveled (feet) */
	UPROPERTY(BlueprintReadOnly)
	float CurrentDistance = 0.0f;

	/** Number of shifts made */
	UPROPERTY(BlueprintReadOnly)
	int32 ShiftCount = 0;

	/** Perfect shifts count */
	UPROPERTY(BlueprintReadOnly)
	int32 PerfectShifts = 0;

	/** Has crossed finish? */
	UPROPERTY(BlueprintReadOnly)
	bool bFinished = false;

	/** Did jump start (red light)? */
	UPROPERTY(BlueprintReadOnly)
	bool bRedLight = false;
};

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLaunchStateChanged, int32, RacerIndex, EMGLaunchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReactionTimeRecorded, int32, RacerIndex, float, ReactionTime, EMGLaunchQuality, Quality);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnIntervalTime, int32, RacerIndex, float, Distance, float, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShiftPerformed, int32, RacerIndex, EMGShiftQuality, Quality);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRedLight, int32, RacerIndex);

/**
 * Drag Race Handler
 * Quarter-mile straight-line acceleration racing
 *
 * Win Condition: First across the finish line (without red light)
 * Scoring: Reaction time + ET (elapsed time)
 * Features:
 * - Christmas tree countdown
 * - Reaction time measurement
 * - Interval timing (60ft, 330ft, 660ft, 1000ft, 1320ft)
 * - Trap speed measurement
 * - Jump start (red light) detection
 * - Shift timing (optional)
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGDragRaceHandler : public UMGRaceTypeHandler
{
	GENERATED_BODY()

public:
	UMGDragRaceHandler();

	//~ Begin UMGRaceTypeHandler Interface
	virtual void Initialize(AMGRaceGameMode* InGameMode) override;
	virtual void Reset() override;
	virtual void OnCountdownStarted() override;
	virtual void OnRaceStarted() override;
	virtual void OnRaceTick(float DeltaTime) override;
	virtual EMGRaceCompletionResult CheckCompletionCondition(int32 RacerIndex) override;
	virtual void CalculatePositions(TArray<int32>& OutPositions) override;

	virtual EMGRaceType GetRaceType() const override { return EMGRaceType::Drag; }
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual bool ShouldShowLapCounter() const override { return false; }
	virtual bool ShouldShowPosition() const override { return true; }
	virtual bool ShouldShowScore() const override { return false; }
	virtual FText GetProgressFormat() const override;

	virtual int64 CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const override;
	//~ End UMGRaceTypeHandler Interface

	// ==========================================
	// CHRISTMAS TREE
	// ==========================================

	/** Start the Christmas tree sequence */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void StartChristmasTree();

	/** Racer stages (moves into position) */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void OnRacerStaged(int32 RacerIndex, bool bFullyStaged);

	/** Get current tree state */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	EMGLaunchState GetTreeState() const { return TreeState; }

	/** Get time until green (during tree drop) */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetTimeToGreen() const;

	// ==========================================
	// LAUNCH & TIMING
	// ==========================================

	/** Record launch input (throttle pressed) */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void OnLaunchInput(int32 RacerIndex);

	/** Get racer drag data */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	FMGDragRacerData GetRacerDragData(int32 RacerIndex) const;

	/** Get reaction time for racer */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetReactionTime(int32 RacerIndex) const;

	/** Get elapsed time for racer */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetElapsedTime(int32 RacerIndex) const;

	/** Get current distance (feet) */
	UFUNCTION(BlueprintPure, Category = "Drag Race")
	float GetCurrentDistance(int32 RacerIndex) const;

	// ==========================================
	// SHIFTING
	// ==========================================

	/** Record a gear shift */
	UFUNCTION(BlueprintCallable, Category = "Drag Race")
	void OnShift(int32 RacerIndex, float RPMPercent);

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Set track distance (default is quarter mile = 1320 feet) */
	UFUNCTION(BlueprintCallable, Category = "Drag Race|Config")
	void SetTrackDistance(float DistanceFeet) { TrackDistanceFeet = DistanceFeet; }

	/** Use pro tree (0.4s per light) or sportsman tree (0.5s per light) */
	UFUNCTION(BlueprintCallable, Category = "Drag Race|Config")
	void SetProTree(bool bPro) { bUseProTree = bPro; }

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnLaunchStateChanged OnLaunchStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnReactionTimeRecorded OnReactionTimeRecorded;

	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnIntervalTime OnIntervalTime;

	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnShiftPerformed OnShiftPerformed;

	UPROPERTY(BlueprintAssignable, Category = "Drag Race|Events")
	FOnRedLight OnRedLight;

protected:
	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Track distance in feet (1320 = quarter mile) */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float TrackDistanceFeet = 1320.0f;

	/** Use pro tree timing */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	bool bUseProTree = false;

	/** Time between amber lights (pro = 0.4, sportsman = 0.5) */
	float GetTreeInterval() const { return bUseProTree ? 0.4f : 0.5f; }

	/** Reaction time threshold for perfect (seconds) */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float PerfectReactionThreshold = 0.02f;

	/** Reaction time threshold for great */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float GreatReactionThreshold = 0.05f;

	/** Reaction time threshold for good */
	UPROPERTY(EditDefaultsOnly, Category = "Drag|Config")
	float GoodReactionThreshold = 0.1f;

	// ==========================================
	// STATE
	// ==========================================

	/** Current tree state */
	UPROPERTY()
	EMGLaunchState TreeState = EMGLaunchState::PreStage;

	/** Time accumulator for tree sequence */
	float TreeTimer = 0.0f;

	/** Time when green light occurred */
	float GreenLightTime = 0.0f;

	/** Current amber light (0-2 for sportsman, 0 for pro) */
	int32 CurrentAmberLight = 0;

	/** Per-racer drag data */
	UPROPERTY()
	TMap<int32, FMGDragRacerData> RacerDragData;

	/** Interval distances to track (in feet) */
	TArray<float> IntervalDistances = { 60.0f, 330.0f, 660.0f, 1000.0f, 1320.0f };

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Update tree sequence */
	void UpdateTreeSequence(float DeltaTime);

	/** Update racer progress */
	void UpdateRacerProgress(int32 RacerIndex, float DeltaTime);

	/** Check interval crossings */
	void CheckIntervals(int32 RacerIndex, float OldDistance, float NewDistance);

	/** Convert distance from cm to feet */
	float CmToFeet(float Cm) const { return Cm * 0.0328084f; }

	/** Convert speed from cm/s to mph */
	float CmsToMph(float Cms) const { return Cms * 0.0223694f; }

	/** Calculate launch quality from reaction time */
	EMGLaunchQuality GetLaunchQuality(float ReactionTime) const;
};

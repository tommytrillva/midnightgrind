// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/MGAIRacerController.h"
#include "MGRacingLineGenerator.generated.h"

class USplineComponent;
class AMGCheckpointActor;

/**
 * Track corner analysis data
 */
USTRUCT(BlueprintType)
struct FMGCornerData
{
	GENERATED_BODY()

	/** Corner start distance along track */
	UPROPERTY(BlueprintReadOnly)
	float StartDistance = 0.0f;

	/** Corner apex distance along track */
	UPROPERTY(BlueprintReadOnly)
	float ApexDistance = 0.0f;

	/** Corner end distance along track */
	UPROPERTY(BlueprintReadOnly)
	float EndDistance = 0.0f;

	/** Corner radius (smaller = tighter) */
	UPROPERTY(BlueprintReadOnly)
	float Radius = 0.0f;

	/** Corner direction (positive = right, negative = left) */
	UPROPERTY(BlueprintReadOnly)
	float Direction = 0.0f;

	/** Optimal entry speed */
	UPROPERTY(BlueprintReadOnly)
	float EntrySpeed = 0.0f;

	/** Optimal apex speed */
	UPROPERTY(BlueprintReadOnly)
	float ApexSpeed = 0.0f;

	/** Braking zone start distance */
	UPROPERTY(BlueprintReadOnly)
	float BrakingZoneStart = 0.0f;

	/** Is hairpin turn */
	UPROPERTY(BlueprintReadOnly)
	bool bIsHairpin = false;

	/** Is chicane (alternating direction) */
	UPROPERTY(BlueprintReadOnly)
	bool bIsChicane = false;
};

/**
 * Track segment type
 */
UENUM(BlueprintType)
enum class EMGTrackSegmentType : uint8
{
	Straight,
	LeftTurn,
	RightTurn,
	Chicane,
	Hairpin
};

/**
 * Track segment data
 */
USTRUCT(BlueprintType)
struct FMGTrackSegment
{
	GENERATED_BODY()

	/** Segment type */
	UPROPERTY(BlueprintReadOnly)
	EMGTrackSegmentType Type = EMGTrackSegmentType::Straight;

	/** Start distance */
	UPROPERTY(BlueprintReadOnly)
	float StartDistance = 0.0f;

	/** End distance */
	UPROPERTY(BlueprintReadOnly)
	float EndDistance = 0.0f;

	/** Average curvature */
	UPROPERTY(BlueprintReadOnly)
	float Curvature = 0.0f;

	/** Track width */
	UPROPERTY(BlueprintReadOnly)
	float Width = 10.0f;

	/** Suggested speed */
	UPROPERTY(BlueprintReadOnly)
	float SuggestedSpeed = 100.0f;
};

/**
 * Racing line generation parameters
 */
USTRUCT(BlueprintType)
struct FMGRacingLineParams
{
	GENERATED_BODY()

	/** Vehicle max speed (m/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 80.0f;

	/** Vehicle max lateral G */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLateralG = 1.2f;

	/** Braking deceleration (m/s^2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDecel = 15.0f;

	/** Acceleration rate (m/s^2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationRate = 8.0f;

	/** Sampling interval (meters) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SamplingInterval = 5.0f;

	/** Smoothing iterations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SmoothingIterations = 3;

	/** Racing line width usage (0 = center, 1 = full width) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WidthUsage = 0.9f;

	/** Corner cutting aggression (0-1) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CornerCuttingAggression = 0.7f;

	/** Late apex preference (0-1, higher = more late apex) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LateApexPreference = 0.3f;
};

/**
 * Delegate for generation complete
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacingLineGenerated, const TArray<FMGAIRacingLinePoint>&, RacingLine);

/**
 * Racing Line Generator
 * Generates optimal racing lines for AI from track splines
 *
 * Features:
 * - Automatic racing line generation from spline
 * - Corner analysis and apex detection
 * - Speed calculation based on curvature
 * - Braking zone detection
 * - Support for different vehicle performance levels
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRacingLineGenerator : public UObject
{
	GENERATED_BODY()

public:
	UMGRacingLineGenerator();

	// ==========================================
	// GENERATION
	// ==========================================

	/**
	 * Generate racing line from spline component
	 * @param TrackSpline The track centerline spline
	 * @param TrackWidth Average track width
	 * @param Params Generation parameters
	 * @return Generated racing line points
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FMGAIRacingLinePoint> GenerateFromSpline(
		USplineComponent* TrackSpline,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	/**
	 * Generate racing line from checkpoint actors
	 * Creates a spline through checkpoints then generates line
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FMGAIRacingLinePoint> GenerateFromCheckpoints(
		const TArray<AMGCheckpointActor*>& Checkpoints,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	/**
	 * Generate racing line from array of points
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FMGAIRacingLinePoint> GenerateFromPoints(
		const TArray<FVector>& CenterlinePoints,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	// ==========================================
	// ANALYSIS
	// ==========================================

	/**
	 * Analyze track and identify corners
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Analysis")
	TArray<FMGCornerData> AnalyzeTrackCorners(
		USplineComponent* TrackSpline,
		float CurvatureThreshold = 0.02f
	);

	/**
	 * Segment track into straights and turns
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Analysis")
	TArray<FMGTrackSegment> SegmentTrack(
		USplineComponent* TrackSpline,
		float SegmentLength = 20.0f
	);

	/**
	 * Calculate optimal speed for a given curvature
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Analysis")
	float CalculateCornerSpeed(float Curvature, float MaxLateralG) const;

	// ==========================================
	// MODIFICATION
	// ==========================================

	/**
	 * Adjust racing line for different skill level
	 * Lower skill = more center-line following, less optimization
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Modify")
	TArray<FMGAIRacingLinePoint> AdjustForSkillLevel(
		const TArray<FMGAIRacingLinePoint>& BaseLine,
		float SkillLevel // 0-1, 1 = optimal
	);

	/**
	 * Add variation to racing line (for different AI personalities)
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Modify")
	TArray<FMGAIRacingLinePoint> AddVariation(
		const TArray<FMGAIRacingLinePoint>& BaseLine,
		float VariationAmount,
		int32 RandomSeed = 0
	);

	/**
	 * Smooth racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Modify")
	TArray<FMGAIRacingLinePoint> SmoothRacingLine(
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		int32 Iterations = 3
	);

	// ==========================================
	// UTILITIES
	// ==========================================

	/**
	 * Calculate total track length
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Utilities")
	float CalculateTrackLength(const TArray<FMGAIRacingLinePoint>& RacingLine) const;

	/**
	 * Get point at specific distance along line
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Utilities")
	FMGAIRacingLinePoint GetPointAtDistance(
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		float Distance
	) const;

	/**
	 * Find closest point on racing line to position
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Utilities")
	int32 FindClosestPointIndex(
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		const FVector& Position
	) const;

	/**
	 * Debug draw racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Debug")
	void DebugDrawRacingLine(
		UWorld* World,
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		float Duration = 5.0f,
		bool bDrawSpeeds = true
	);

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when racing line generation is complete */
	UPROPERTY(BlueprintAssignable, Category = "Racing Line|Events")
	FOnRacingLineGenerated OnRacingLineGenerated;

protected:
	// ==========================================
	// INTERNAL GENERATION
	// ==========================================

	/** Sample centerline points from spline */
	TArray<FVector> SampleSplinePoints(
		USplineComponent* Spline,
		float SamplingInterval
	);

	/** Calculate curvature at each point */
	TArray<float> CalculateCurvatures(const TArray<FVector>& Points);

	/** Generate optimal lateral offset for racing line */
	TArray<float> GenerateLateralOffsets(
		const TArray<FVector>& CenterPoints,
		const TArray<float>& Curvatures,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	/** Apply lateral offsets to create racing line */
	TArray<FVector> ApplyLateralOffsets(
		const TArray<FVector>& CenterPoints,
		const TArray<float>& Offsets,
		USplineComponent* Spline
	);

	/** Calculate target speeds for all points */
	TArray<float> CalculateTargetSpeeds(
		const TArray<FVector>& RacingLinePoints,
		const TArray<float>& Curvatures,
		const FMGRacingLineParams& Params
	);

	/** Apply backward pass for braking zones */
	void ApplyBrakingZones(
		TArray<float>& Speeds,
		const TArray<FVector>& Points,
		float BrakingDecel
	);

	/** Apply forward pass for acceleration zones */
	void ApplyAccelerationZones(
		TArray<float>& Speeds,
		const TArray<FVector>& Points,
		float AccelerationRate
	);

	/** Identify apexes in the racing line */
	TArray<int32> FindApexIndices(
		const TArray<float>& Curvatures,
		float Threshold = 0.01f
	);

	/** Identify braking zone start indices */
	TArray<int32> FindBrakingZoneStarts(
		const TArray<float>& Speeds,
		float SpeedDropThreshold = 5.0f
	);

	/** Build final racing line points */
	TArray<FMGAIRacingLinePoint> BuildRacingLinePoints(
		const TArray<FVector>& Positions,
		const TArray<float>& Speeds,
		const TArray<float>& Curvatures,
		float TrackWidth
	);

	// ==========================================
	// MATH UTILITIES
	// ==========================================

	/** Calculate curvature from three points */
	float CalculateCurvatureFromPoints(
		const FVector& P0,
		const FVector& P1,
		const FVector& P2
	) const;

	/** Get perpendicular vector (for lateral offset) */
	FVector GetPerpendicularVector(const FVector& Direction) const;

	/** Clamp value with smoothing at edges */
	float SmoothClamp(float Value, float Min, float Max, float SmoothRange) const;

private:
	/** Cached last generation result */
	TArray<FMGAIRacingLinePoint> LastGeneratedLine;
};

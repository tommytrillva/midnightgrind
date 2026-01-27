// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGRacingLineGenerator.h
 * @brief Racing Line Generator - Computes optimal paths for AI racers
 *
 * This system generates the racing lines that AI opponents follow during races.
 * A "racing line" is the optimal path around a track that minimizes lap time,
 * taking into account cornering physics, braking zones, and track geometry.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 * --------------------------------
 * - RACING LINE: The ideal path around a track. Goes wide entering corners,
 *   clips the apex (innermost point), and exits wide to maximize speed.
 *
 * - APEX: The innermost point of a corner on the racing line. Hitting the
 *   apex at the right speed is crucial for fast lap times.
 *
 * - BRAKING ZONE: Area before a corner where the driver must slow down.
 *   The generator calculates where braking should start based on corner
 *   tightness and vehicle capabilities.
 *
 * - CURVATURE: How "tight" a section of track is. High curvature = tight
 *   corner requiring slower speeds. Zero curvature = straight.
 *
 * - LATERAL OFFSET: How far left/right of the track centerline the racing
 *   line deviates. Positive = right, negative = left.
 *
 * GENERATION ALGORITHM OVERVIEW:
 * 1. Sample points along the track centerline spline
 * 2. Calculate curvature at each point
 * 3. Generate lateral offsets to create the racing line path
 * 4. Calculate target speeds based on corner tightness
 * 5. Apply braking zones (backward pass from slow corners)
 * 6. Apply acceleration zones (forward pass from slow sections)
 * 7. Build final FMGAIRacingLinePoint array with all data
 *
 * SKILL LEVEL ADJUSTMENT:
 * The generator can produce racing lines of varying quality for different
 * AI difficulty levels. Lower skill = more centerline following, less
 * optimization, and wider safety margins.
 *
 * @see UMGRacingLineSubsystem - Manages loaded racing lines at runtime
 * @see AMGRacingAIController - Uses racing lines to control AI vehicles
 *
 * Midnight Grind - Y2K Arcade Street Racing
 */

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/MGAIRacerController.h"
#include "MGRacingLineGenerator.generated.h"

class USplineComponent;
class AMGCheckpointActor;

// ============================================================================
// TRACK ANALYSIS DATA STRUCTURES
// ============================================================================

/**
 * Track corner analysis data
 *
 * Contains detailed information about a single corner identified on the track.
 * Used both for racing line generation and for providing corner information
 * to the UI (corner names, difficulty indicators, etc.).
 *
 * CORNER ANATOMY:
 * - Entry: Where braking begins
 * - Turn-in: Where steering input starts
 * - Apex: The innermost point of the corner
 * - Exit: Where the car returns to full throttle
 */
USTRUCT(BlueprintType)
struct FMGCornerData
{
	GENERATED_BODY()

	/**
	 * Distance along track where corner begins (braking zone start)
	 * Measured in centimeters from track start/finish
	 */
	UPROPERTY(BlueprintReadOnly)
	float StartDistance = 0.0f;

	/**
	 * Distance along track to the apex (tightest point)
	 * This is where the racing line clips closest to the inside
	 */
	UPROPERTY(BlueprintReadOnly)
	float ApexDistance = 0.0f;

	/**
	 * Distance along track where corner ends
	 * After this point, full acceleration is possible
	 */
	UPROPERTY(BlueprintReadOnly)
	float EndDistance = 0.0f;

	/**
	 * Corner radius in centimeters (smaller = tighter corner)
	 * Used to calculate maximum cornering speed: v = sqrt(radius * g * friction)
	 */
	UPROPERTY(BlueprintReadOnly)
	float Radius = 0.0f;

	/**
	 * Corner direction indicator
	 * Positive values = right turn, Negative values = left turn
	 * Magnitude indicates sharpness
	 */
	UPROPERTY(BlueprintReadOnly)
	float Direction = 0.0f;

	/** Recommended speed when entering the braking zone (m/s) */
	UPROPERTY(BlueprintReadOnly)
	float EntrySpeed = 0.0f;

	/** Minimum speed at the apex point (m/s) */
	UPROPERTY(BlueprintReadOnly)
	float ApexSpeed = 0.0f;

	/**
	 * Distance where braking should begin
	 * Calculated based on entry speed, apex speed, and braking capability
	 */
	UPROPERTY(BlueprintReadOnly)
	float BrakingZoneStart = 0.0f;

	/**
	 * True if this is a hairpin turn (very tight, often 180 degrees)
	 * Hairpins require special handling: trail braking, late apex
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsHairpin = false;

	/**
	 * True if this corner alternates direction quickly (S-curve)
	 * Chicanes require smooth weight transfer between turns
	 */
	UPROPERTY(BlueprintReadOnly)
	bool bIsChicane = false;
};

// ============================================================================
// TRACK SEGMENT CLASSIFICATION
// ============================================================================

/**
 * Track segment type classification
 *
 * Categorizes sections of track for AI behavior and UI display.
 * Different segment types may trigger different driving strategies.
 */
UENUM(BlueprintType)
enum class EMGTrackSegmentType : uint8
{
	/// Long section with minimal curvature - maximize top speed
	Straight,

	/// Corner turning left - requires braking and steering
	LeftTurn,

	/// Corner turning right - requires braking and steering
	RightTurn,

	/// Quick left-right or right-left combination
	Chicane,

	/// Very tight turn, often 180 degrees or more
	Hairpin
};

/**
 * Track segment data
 *
 * Represents a classified section of the track with average properties.
 * Used for strategic decisions like overtaking zones and pit entry.
 */
USTRUCT(BlueprintType)
struct FMGTrackSegment
{
	GENERATED_BODY()

	/** Classification of this segment */
	UPROPERTY(BlueprintReadOnly)
	EMGTrackSegmentType Type = EMGTrackSegmentType::Straight;

	/** Distance along track where segment begins (cm) */
	UPROPERTY(BlueprintReadOnly)
	float StartDistance = 0.0f;

	/** Distance along track where segment ends (cm) */
	UPROPERTY(BlueprintReadOnly)
	float EndDistance = 0.0f;

	/**
	 * Average curvature through this segment
	 * 0 = straight, higher values = tighter turns
	 * Typical range: 0.0 to 0.1 (1/radius in cm)
	 */
	UPROPERTY(BlueprintReadOnly)
	float Curvature = 0.0f;

	/** Track width in this segment (meters) */
	UPROPERTY(BlueprintReadOnly)
	float Width = 10.0f;

	/** Recommended speed for this segment (m/s) */
	UPROPERTY(BlueprintReadOnly)
	float SuggestedSpeed = 100.0f;
};

// ============================================================================
// GENERATION PARAMETERS
// ============================================================================

/**
 * Racing line generation parameters
 *
 * Controls how the racing line is computed. These parameters can be
 * tuned per-vehicle class (sports cars vs trucks) or adjusted for
 * different AI skill levels.
 *
 * PARAMETER TUNING GUIDE:
 * - For faster cars: Higher MaxSpeed, MaxLateralG, and CornerCuttingAggression
 * - For heavy vehicles: Lower MaxLateralG, higher BrakingDecel
 * - For cautious AI: Lower WidthUsage and CornerCuttingAggression
 */
USTRUCT(BlueprintType)
struct FMGRacingLineParams
{
	GENERATED_BODY()

	/**
	 * Vehicle maximum straight-line speed (meters/second)
	 * Used to limit speeds on straights
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSpeed = 80.0f;

	/**
	 * Maximum lateral acceleration in G-forces
	 * Determines how fast the vehicle can take corners
	 * Sports car: ~1.2G, Truck: ~0.7G
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLateralG = 1.2f;

	/**
	 * Braking deceleration rate (m/s^2)
	 * Higher = shorter braking distances, later braking points
	 * Sports car: ~15, Truck: ~8
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BrakingDecel = 15.0f;

	/**
	 * Acceleration rate from slow speeds (m/s^2)
	 * Affects exit speed calculations
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AccelerationRate = 8.0f;

	/**
	 * Distance between sampled points (meters)
	 * Smaller = more detailed line but more computation
	 * Recommended: 5m for smooth AI, 10m for performance
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SamplingInterval = 5.0f;

	/**
	 * Number of smoothing passes on the final line
	 * Higher = smoother line but may cut corners more
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SmoothingIterations = 3;

	/**
	 * How much of the track width to use (0-1)
	 * 0 = stay on centerline, 1 = use full width
	 * Aggressive AI: 0.95, Cautious AI: 0.7
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WidthUsage = 0.9f;

	/**
	 * How aggressively to cut corners (0-1)
	 * Higher values create tighter, faster lines but risk track limits
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CornerCuttingAggression = 0.7f;

	/**
	 * Late apex preference (0-1)
	 * 0 = geometric apex, 1 = very late apex
	 * Late apex = slower entry, faster exit (better for long straights after)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float LateApexPreference = 0.3f;
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/**
 * Fired when racing line generation completes
 * @param RacingLine The generated array of racing line points
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacingLineGenerated, const TArray<FMGAIRacingLinePoint>&, RacingLine);

// ============================================================================
// RACING LINE GENERATOR CLASS
// ============================================================================

/**
 * Racing Line Generator
 *
 * Generates optimal racing lines for AI from track splines or point arrays.
 * This is typically used offline or at track load time, not during races.
 *
 * FEATURES:
 * - Automatic racing line generation from spline components
 * - Corner analysis and apex detection
 * - Speed calculation based on curvature and vehicle capabilities
 * - Braking zone identification
 * - Support for different vehicle performance levels
 * - Skill-based line adjustment for AI difficulty
 *
 * @see FMGAIRacingLinePoint - The output data structure
 * @see FMGRacingLineParams - Generation configuration
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGRacingLineGenerator : public UObject
{
	GENERATED_BODY()

public:
	UMGRacingLineGenerator();

	// ==========================================
	// PRIMARY GENERATION METHODS
	// ==========================================

	/**
	 * Generate racing line from a spline component
	 *
	 * This is the primary generation method. Takes a track centerline
	 * and produces an optimized racing line.
	 *
	 * @param TrackSpline The track centerline spline component
	 * @param TrackWidth Average track width in centimeters
	 * @param Params Generation parameters (vehicle capabilities, etc.)
	 * @return Array of racing line points ready for AI use
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FMGAIRacingLinePoint> GenerateFromSpline(
		USplineComponent* TrackSpline,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	/**
	 * Generate racing line from checkpoint actors
	 *
	 * Alternative method that creates a spline through checkpoint
	 * positions, then generates the racing line. Useful when you
	 * have checkpoint-based tracks rather than spline-based.
	 *
	 * @param Checkpoints Array of checkpoint actors in order
	 * @param TrackWidth Average track width in centimeters
	 * @param Params Generation parameters
	 * @return Generated racing line points
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FMGAIRacingLinePoint> GenerateFromCheckpoints(
		const TArray<AMGCheckpointActor*>& Checkpoints,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	/**
	 * Generate racing line from raw point array
	 *
	 * Lowest-level generation method. Use when you have pre-computed
	 * centerline points rather than a spline or checkpoints.
	 *
	 * @param CenterlinePoints Array of world positions forming the centerline
	 * @param TrackWidth Track width in centimeters
	 * @param Params Generation parameters
	 * @return Generated racing line points
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line")
	TArray<FMGAIRacingLinePoint> GenerateFromPoints(
		const TArray<FVector>& CenterlinePoints,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	// ==========================================
	// TRACK ANALYSIS METHODS
	// ==========================================

	/**
	 * Analyze track and identify all corners
	 *
	 * Scans the track spline to find corners based on curvature.
	 * Returns detailed data about each corner for UI or AI use.
	 *
	 * @param TrackSpline The track to analyze
	 * @param CurvatureThreshold Minimum curvature to qualify as corner (default 0.02)
	 * @return Array of identified corners with full analysis data
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Analysis")
	TArray<FMGCornerData> AnalyzeTrackCorners(
		USplineComponent* TrackSpline,
		float CurvatureThreshold = 0.02f
	);

	/**
	 * Segment track into classified sections
	 *
	 * Divides the track into straights, corners, chicanes, etc.
	 * Useful for strategic AI decisions and telemetry.
	 *
	 * @param TrackSpline The track to segment
	 * @param SegmentLength Minimum segment length in meters
	 * @return Array of classified track segments
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Analysis")
	TArray<FMGTrackSegment> SegmentTrack(
		USplineComponent* TrackSpline,
		float SegmentLength = 20.0f
	);

	/**
	 * Calculate maximum corner speed for a given curvature
	 *
	 * Uses the formula: v = sqrt(radius * g * friction)
	 * where radius = 1/curvature
	 *
	 * @param Curvature The curvature value (1/radius in cm)
	 * @param MaxLateralG Maximum lateral G-force the vehicle can sustain
	 * @return Maximum safe speed in m/s
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Analysis")
	float CalculateCornerSpeed(float Curvature, float MaxLateralG) const;

	// ==========================================
	// LINE MODIFICATION METHODS
	// ==========================================

	/**
	 * Adjust racing line for different skill levels
	 *
	 * Creates a modified version of the racing line that's easier
	 * or harder to follow based on skill level.
	 *
	 * SKILL EFFECTS:
	 * - Lower skill: Line moves toward centerline, wider margins
	 * - Lower skill: Target speeds reduced for more safety buffer
	 * - Higher skill: Full optimization, uses track limits
	 *
	 * @param BaseLine The optimal racing line to adjust
	 * @param SkillLevel Skill factor 0-1 (1 = optimal/pro, 0 = beginner)
	 * @return Adjusted racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Modify")
	TArray<FMGAIRacingLinePoint> AdjustForSkillLevel(
		const TArray<FMGAIRacingLinePoint>& BaseLine,
		float SkillLevel // 0-1, 1 = optimal
	);

	/**
	 * Add variation to racing line for AI personality
	 *
	 * Creates slight variations in the racing line so different AI
	 * opponents don't all follow the exact same path. Useful for
	 * creating more natural-looking AI behavior.
	 *
	 * @param BaseLine The base racing line
	 * @param VariationAmount How much variation to add (0-1)
	 * @param RandomSeed Seed for reproducible variation
	 * @return Racing line with added variation
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Modify")
	TArray<FMGAIRacingLinePoint> AddVariation(
		const TArray<FMGAIRacingLinePoint>& BaseLine,
		float VariationAmount,
		int32 RandomSeed = 0
	);

	/**
	 * Smooth a racing line to reduce sharp transitions
	 *
	 * Applies averaging passes to create smoother steering inputs.
	 * More iterations = smoother but may compromise optimization.
	 *
	 * @param RacingLine The line to smooth
	 * @param Iterations Number of smoothing passes
	 * @return Smoothed racing line
	 */
	UFUNCTION(BlueprintCallable, Category = "Racing Line|Modify")
	TArray<FMGAIRacingLinePoint> SmoothRacingLine(
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		int32 Iterations = 3
	);

	// ==========================================
	// UTILITY METHODS
	// ==========================================

	/**
	 * Calculate total length of a racing line
	 * @param RacingLine The racing line to measure
	 * @return Total length in centimeters
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Utilities")
	float CalculateTrackLength(const TArray<FMGAIRacingLinePoint>& RacingLine) const;

	/**
	 * Get interpolated point at specific distance along line
	 *
	 * Useful for finding racing line data at arbitrary positions
	 * between sampled points.
	 *
	 * @param RacingLine The racing line to query
	 * @param Distance Distance along the line in cm
	 * @return Interpolated point data
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Utilities")
	FMGAIRacingLinePoint GetPointAtDistance(
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		float Distance
	) const;

	/**
	 * Find index of closest racing line point to a world position
	 *
	 * @param RacingLine The racing line to search
	 * @param Position World position to find nearest point to
	 * @return Index into RacingLine array, -1 if line is empty
	 */
	UFUNCTION(BlueprintPure, Category = "Racing Line|Utilities")
	int32 FindClosestPointIndex(
		const TArray<FMGAIRacingLinePoint>& RacingLine,
		const FVector& Position
	) const;

	/**
	 * Draw racing line in world for debugging
	 *
	 * Renders the racing line with color-coding for speed.
	 * Red = braking zones, Green = acceleration, Yellow = coasting
	 *
	 * @param World World context for debug drawing
	 * @param RacingLine The line to visualize
	 * @param Duration How long to display (seconds)
	 * @param bDrawSpeeds Whether to show speed indicators
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

	/** Broadcast when async racing line generation completes */
	UPROPERTY(BlueprintAssignable, Category = "Racing Line|Events")
	FOnRacingLineGenerated OnRacingLineGenerated;

protected:
	// ==========================================
	// INTERNAL GENERATION PIPELINE
	// ==========================================

	/** Step 1: Sample points from the track spline at regular intervals */
	TArray<FVector> SampleSplinePoints(
		USplineComponent* Spline,
		float SamplingInterval
	);

	/** Step 2: Calculate curvature (1/radius) at each sampled point */
	TArray<float> CalculateCurvatures(const TArray<FVector>& Points);

	/**
	 * Step 3: Generate lateral offsets for racing line optimization
	 * Determines how far left/right of centerline each point should be
	 */
	TArray<float> GenerateLateralOffsets(
		const TArray<FVector>& CenterPoints,
		const TArray<float>& Curvatures,
		float TrackWidth,
		const FMGRacingLineParams& Params
	);

	/** Step 4: Apply lateral offsets to centerline to create racing line positions */
	TArray<FVector> ApplyLateralOffsets(
		const TArray<FVector>& CenterPoints,
		const TArray<float>& Offsets,
		USplineComponent* Spline
	);

	/** Step 5: Calculate initial target speeds based on curvature */
	TArray<float> CalculateTargetSpeeds(
		const TArray<FVector>& RacingLinePoints,
		const TArray<float>& Curvatures,
		const FMGRacingLineParams& Params
	);

	/**
	 * Step 6: Backward pass - propagate braking requirements
	 * Ensures vehicle can slow down in time for upcoming slow sections
	 */
	void ApplyBrakingZones(
		TArray<float>& Speeds,
		const TArray<FVector>& Points,
		float BrakingDecel
	);

	/**
	 * Step 7: Forward pass - propagate acceleration limits
	 * Ensures vehicle doesn't exceed what it can accelerate to
	 */
	void ApplyAccelerationZones(
		TArray<float>& Speeds,
		const TArray<FVector>& Points,
		float AccelerationRate
	);

	/** Identify apex points (local curvature maxima) */
	TArray<int32> FindApexIndices(
		const TArray<float>& Curvatures,
		float Threshold = 0.01f
	);

	/** Identify where braking zones begin (where speed starts dropping) */
	TArray<int32> FindBrakingZoneStarts(
		const TArray<float>& Speeds,
		float SpeedDropThreshold = 5.0f
	);

	/** Final step: Assemble all data into output racing line points */
	TArray<FMGAIRacingLinePoint> BuildRacingLinePoints(
		const TArray<FVector>& Positions,
		const TArray<float>& Speeds,
		const TArray<float>& Curvatures,
		float TrackWidth
	);

	// ==========================================
	// MATH UTILITIES
	// ==========================================

	/**
	 * Calculate curvature from three consecutive points
	 * Uses the circumradius of the triangle formed by the points
	 */
	float CalculateCurvatureFromPoints(
		const FVector& P0,
		const FVector& P1,
		const FVector& P2
	) const;

	/**
	 * Get perpendicular vector in the horizontal plane
	 * Used to calculate lateral offset direction
	 */
	FVector GetPerpendicularVector(const FVector& Direction) const;

	/**
	 * Clamp value with smooth falloff at boundaries
	 * Prevents harsh transitions at limits
	 */
	float SmoothClamp(float Value, float Min, float Max, float SmoothRange) const;

private:
	/** Cache of last generated racing line (for quick re-queries) */
	TArray<FMGAIRacingLinePoint> LastGeneratedLine;
};

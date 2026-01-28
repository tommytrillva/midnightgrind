// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTrackSpline.h
 * @brief Track layout definition using splines for racing line, boundaries, and queries.
 *
 * This file defines the AMGTrackSpline class and related types for representing
 * the physical layout of a race track. The track spline is the core data structure
 * for position tracking, AI pathfinding, respawn calculations, and boundary detection.
 *
 * @section spline_concepts Key Concepts
 *
 * SPLINE: A mathematical curve that passes smoothly through a series of control
 * points. USplineComponent in Unreal Engine provides powerful tools for defining
 * and querying 3D paths. Think of it like a flexible wire that can be bent to
 * follow any path.
 *
 * RACING LINE: The optimal path around the track. This spline defines the "center"
 * of the track and is used for distance calculations and AI navigation.
 *
 * INNER/OUTER BOUNDARIES: Two additional splines that define the edges of the
 * driveable track surface. Vehicles crossing these boundaries are considered
 * "off-track" and may receive penalties.
 *
 * TRACK SEGMENTS: The track can be divided into segments with different properties
 * (surface type, width, hazards). This allows for varied track surfaces like
 * transitions from asphalt to gravel.
 *
 * DISTANCE ALONG TRACK: A key measurement - how far along the racing line a
 * position is. Used for lap progress, position tracking, and respawn placement.
 *
 * @section spline_surface Track Surface Types
 *
 * Different surface types affect vehicle physics:
 * - Asphalt: Standard road surface with maximum grip
 * - Concrete: Slightly less grip than asphalt
 * - Gravel: Loose surface, reduced grip, vehicles may slide
 * - Dirt: Packed earth, good for controlled drifting
 * - Grass: Very low grip, usually off-track penalty
 * - Sand: Extremely low grip, slows vehicles significantly
 * - Snow/Ice: Minimal grip, requires careful throttle control
 * - Wet: Reduced grip compared to dry conditions
 *
 * @section spline_architecture Architecture
 *
 * The track spline system provides:
 * 1. A racing line spline for distance/position queries
 * 2. Inner and outer boundary splines for off-track detection
 * 3. Per-segment track properties (surface, width, hazards)
 * 4. AI pathfinding helpers (look-ahead points, curvature, speed suggestions)
 * 5. Respawn system integration (safe return points after crashes)
 *
 * @section spline_usage Usage Examples
 *
 * @code
 * // Get the track spline actor (placed in level or found via subsystem)
 * AMGTrackSpline* TrackSpline = FindTrackSpline();
 *
 * // Calculate how far along the track a vehicle is
 * float Distance = TrackSpline->GetClosestDistanceOnTrack(VehicleLocation);
 *
 * // Check if vehicle is on track
 * if (!TrackSpline->IsPositionOnTrack(VehicleLocation))
 * {
 *     ApplyOffTrackPenalty();
 * }
 *
 * // Get AI look-ahead point (for steering toward)
 * float CurrentDist = TrackSpline->GetClosestDistanceOnTrack(AILocation);
 * FVector TargetPoint = TrackSpline->GetLookAheadPoint(CurrentDist, 5000.0f);
 *
 * // Get suggested speed based on upcoming curvature
 * float SuggestedSpeed = TrackSpline->GetSuggestedSpeedAtDistance(CurrentDist);
 *
 * // Get respawn transform after a crash
 * FTransform RespawnTransform = TrackSpline->GetNearestRespawnTransform(CrashLocation);
 * @endcode
 *
 * @section spline_editor Editor Setup
 *
 * To set up a track spline:
 * 1. Place AMGTrackSpline actor in the level
 * 2. Edit the RacingLineSpline by selecting spline points and adjusting
 * 3. Add spline points (Alt+click on spline) to define the track path
 * 4. Configure bClosedLoop = true for circuit tracks
 * 5. Set DefaultTrackWidth for the overall track width
 * 6. Add TrackSegments for areas with different surfaces/properties
 *
 * @section spline_related Related Systems
 * - UMGTrackSubsystem: Higher-level track management that uses track spline
 * - AMGTrackBoundaryActor: Separate boundary definition (alternative approach)
 * - AI Controller: Queries track spline for pathfinding
 * - Respawn System: Uses spline for safe respawn positions
 *
 * @see EMGTrackSurface
 * @see FMGTrackSegment
 * @see USplineComponent
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "MGTrackSpline.generated.h"

class USplineMeshComponent;

/**
 * Track surface type
 */
UENUM(BlueprintType)
enum class EMGTrackSurface : uint8
{
	Asphalt		UMETA(DisplayName = "Asphalt"),
	Concrete	UMETA(DisplayName = "Concrete"),
	Gravel		UMETA(DisplayName = "Gravel"),
	Dirt		UMETA(DisplayName = "Dirt"),
	Grass		UMETA(DisplayName = "Grass"),
	Sand		UMETA(DisplayName = "Sand"),
	Snow		UMETA(DisplayName = "Snow"),
	Ice			UMETA(DisplayName = "Ice"),
	Wet			UMETA(DisplayName = "Wet Asphalt")
};

/**
 * Per-segment track data
 */
USTRUCT(BlueprintType)
struct FMGTrackSegment
{
	GENERATED_BODY()

	/** Spline distance where this segment starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	float StartDistance = 0.0f;

	/** Surface type for this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	EMGTrackSurface Surface = EMGTrackSurface::Asphalt;

	/** Track width at this segment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	float TrackWidth = 1000.0f;

	/** Is this segment a hazard zone? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	bool bHazardZone = false;

	/** Speed limit for this segment (for AI) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	float SuggestedSpeed = 0.0f; // 0 = no limit
};

/**
 * Track spline actor - defines the racing line and track bounds
 * Used for AI pathfinding, position calculation, and respawn points
 */
UCLASS(Blueprintable)
class MIDNIGHTGRIND_API AMGTrackSpline : public AActor
{
	GENERATED_BODY()

public:
	AMGTrackSpline();

	//~ Begin AActor Interface
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	//~ End AActor Interface

	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Main racing line spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> RacingLineSpline;

	/** Inner track boundary spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> InnerBoundarySpline;

	/** Outer track boundary spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USplineComponent> OuterBoundarySpline;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Track name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	FText TrackName;

	/** Track length (calculated from spline) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track")
	float TrackLength = 0.0f;

	/** Is this a closed loop track? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	bool bClosedLoop = true;

	/** Default track width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	float DefaultTrackWidth = 1000.0f;

	/** Track segments with variable properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track")
	TArray<FMGTrackSegment> TrackSegments;

	// ==========================================
	// QUERIES
	// ==========================================

	/** Get total track length */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetTrackLength() const { return TrackLength; }

	/** Get position on racing line at distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	FVector GetPositionAtDistance(float Distance) const;

	/** Get rotation at distance (facing direction) */
	UFUNCTION(BlueprintPure, Category = "Track")
	FRotator GetRotationAtDistance(float Distance) const;

	/** Get transform at distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	FTransform GetTransformAtDistance(float Distance) const;

	/** Get closest point on track to a world position */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetClosestDistanceOnTrack(const FVector& WorldPosition) const;

	/** Get track width at a distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetTrackWidthAtDistance(float Distance) const;

	/** Get surface type at a distance */
	UFUNCTION(BlueprintPure, Category = "Track")
	EMGTrackSurface GetSurfaceAtDistance(float Distance) const;

	/** Check if a position is on track */
	UFUNCTION(BlueprintPure, Category = "Track")
	bool IsPositionOnTrack(const FVector& WorldPosition) const;

	/** Get signed distance from racing line (positive = right, negative = left) */
	UFUNCTION(BlueprintPure, Category = "Track")
	float GetLateralDistanceFromRacingLine(const FVector& WorldPosition) const;

	// ==========================================
	// AI HELPERS
	// ==========================================

	/** Get suggested speed at distance */
	UFUNCTION(BlueprintPure, Category = "Track|AI")
	float GetSuggestedSpeedAtDistance(float Distance) const;

	/** Get racing line point ahead of current position */
	UFUNCTION(BlueprintPure, Category = "Track|AI")
	FVector GetLookAheadPoint(float CurrentDistance, float LookAheadDistance) const;

	/** Get curvature at distance (for speed calculation) */
	UFUNCTION(BlueprintPure, Category = "Track|AI")
	float GetCurvatureAtDistance(float Distance) const;

	// ==========================================
	// RESPAWN
	// ==========================================

	/** Get respawn transform at a distance */
	UFUNCTION(BlueprintPure, Category = "Track|Respawn")
	FTransform GetRespawnTransformAtDistance(float Distance) const;

	/** Get nearest respawn point to a position */
	UFUNCTION(BlueprintPure, Category = "Track|Respawn")
	FTransform GetNearestRespawnTransform(const FVector& WorldPosition) const;

protected:
	/** Calculate track length from spline */
	void CalculateTrackLength();

	/** Generate boundary splines from racing line */
	void GenerateBoundarySplines();

	/** Get segment data at distance */
	const FMGTrackSegment* GetSegmentAtDistance(float Distance) const;
};

// NOTE: UMGTrackDataAsset moved to MGTrackDataAssets.h

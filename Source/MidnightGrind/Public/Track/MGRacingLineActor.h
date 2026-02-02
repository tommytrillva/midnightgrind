// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGRacingLineActor.h
 * @brief Racing line visualization and query system for optimal path guidance.
 *
 * This file defines the AMGRacingLineActor class, which represents the optimal racing
 * path around a track. The racing line is the theoretical "fastest" path through a
 * circuit, taking into account corner entry/exit points, apex positions, and
 * acceleration/braking zones.
 *
 * @section racing_line_concepts Key Concepts
 *
 * RACING LINE: In motorsport, the racing line is the route around a track that
 * minimizes lap time. It typically involves:
 * - Late braking into corners
 * - Hitting the apex (innermost point of a corner)
 * - Early acceleration on corner exit
 * - Using the full width of the track
 *
 * SPLINE: A mathematical curve that smoothly passes through a series of control
 * points. Unreal Engine's USplineComponent allows us to define complex 3D paths
 * that can be queried for positions, directions, and distances.
 *
 * IDEAL SPEED: Each point on the racing line has an associated "ideal speed" -
 * the maximum safe velocity for that section. This helps AI drivers and can
 * provide guidance to players.
 *
 * @section racing_line_architecture Architecture
 *
 * The racing line system works as follows:
 * 1. Designer places AMGRacingLineActor in the level
 * 2. Control points are added to define the optimal path
 * 3. Each point has associated metadata (speed, braking zones, etc.)
 * 4. At runtime, AI and ghost systems query the racing line for guidance
 * 5. The line can optionally be visualized for player assistance
 *
 * @section racing_line_usage Usage Examples
 *
 * @code
 * // Get the racing line actor (typically found by subsystem)
 * AMGRacingLineActor* RacingLine = GetRacingLineActor();
 *
 * // Query the optimal position 100 meters into the track
 * FVector OptimalPosition = RacingLine->GetPositionAtDistance(10000.0f); // cm
 *
 * // Check if AI should be braking at current position
 * float CurrentDistance = RacingLine->GetDistanceAlongLine(VehicleLocation);
 * if (RacingLine->IsInBrakingZone(CurrentDistance))
 * {
 *     ApplyBrakes();
 * }
 *
 * // Get ideal speed for speed advisory
 * float TargetSpeed = RacingLine->GetIdealSpeedAtDistance(CurrentDistance);
 *
 * // Calculate how far off the racing line the player is
 * float Deviation = RacingLine->GetDeviationFromLine(VehicleLocation);
 * @endcode
 *
 * @section racing_line_related Related Systems
 * - UMGTrackSubsystem: Manages overall track data and may reference racing line
 * - AI Driving System: Uses racing line for pathfinding decisions
 * - Ghost System: Records and plays back racing line data
 * - HUD: Can display racing line for player assistance
 *
 * @see FMGRacingLinePoint
 * @see USplineComponent
 */

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RacingLine/MGRacingLineSubsystem.h"
#include "MGRacingLineActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

// FMGRacingLinePoint - REMOVED (duplicate)
// Canonical definition in: RacingLine/MGRacingLineSubsystem.h

/**
 * Racing Line Actor
 * Defines the optimal racing path around a track
 *
 * Features:
 * - Spline-based racing line
 * - Per-point speed suggestions
 * - Braking/acceleration zones
 * - AI navigation aid
 * - Ghost racing reference
 * - Visual debugging
 */
UCLASS()
class MIDNIGHTGRIND_API AMGRacingLineActor : public AActor
{
	GENERATED_BODY()

public:
	AMGRacingLineActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// ==========================================
	// QUERY FUNCTIONS
	// ==========================================

	/** Get position on racing line at distance */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	FVector GetPositionAtDistance(float Distance) const;

	/** Get direction at distance */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	FRotator GetDirectionAtDistance(float Distance) const;

	/** Get ideal speed at distance */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	float GetIdealSpeedAtDistance(float Distance) const;

	/** Get closest point on racing line to world position */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	FVector GetClosestPointOnLine(FVector WorldPosition) const;

	/** Get distance along line for world position */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	float GetDistanceAlongLine(FVector WorldPosition) const;

	/** Get deviation from racing line */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	float GetDeviationFromLine(FVector WorldPosition) const;

	/** Get total racing line length */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	float GetTotalLength() const;

	/** Is position in braking zone */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	bool IsInBrakingZone(float Distance) const;

	/** Get racing line points */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	TArray<FMGRacingLinePoint> GetRacingLinePoints() const { return RacingLinePoints; }

	/** Get spline component */
	UFUNCTION(BlueprintPure, Category = "RacingLine")
	USplineComponent* GetSplineComponent() const { return RacingLineSpline; }

	// ==========================================
	// MODIFICATION
	// ==========================================

	/** Set racing line from points */
	UFUNCTION(BlueprintCallable, Category = "RacingLine")
	void SetRacingLineFromPoints(const TArray<FMGRacingLinePoint>& Points);

	/** Add point to racing line */
	UFUNCTION(BlueprintCallable, Category = "RacingLine")
	void AddRacingLinePoint(const FMGRacingLinePoint& Point);

	/** Clear racing line */
	UFUNCTION(BlueprintCallable, Category = "RacingLine")
	void ClearRacingLine();

	// ==========================================
	// VISUALIZATION
	// ==========================================

	/** Show/hide racing line visual */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Visual")
	void SetLineVisible(bool bVisible);

	/** Set racing line color */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Visual")
	void SetLineColor(FLinearColor Color);

	/** Show speed zones */
	UFUNCTION(BlueprintCallable, Category = "RacingLine|Visual")
	void SetShowSpeedZones(bool bShow);

protected:
	// ==========================================
	// COMPONENTS
	// ==========================================

	/** Racing line spline */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* RacingLineSpline;

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** Racing line points with additional data */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
	TArray<FMGRacingLinePoint> RacingLinePoints;

	/** Is this a closed loop */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine")
	bool bIsClosedLoop = true;

	/** Line color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine|Visual")
	FLinearColor LineColor = FLinearColor(0.0f, 0.5f, 1.0f, 1.0f);

	/** Line width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine|Visual")
	float LineWidth = 50.0f;

	/** Show visual in game */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine|Visual")
	bool bShowInGame = false;

	/** Show speed zones */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine|Visual")
	bool bShowSpeedZones = false;

	/** Braking zone color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine|Visual")
	FLinearColor BrakingZoneColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Acceleration zone color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RacingLine|Visual")
	FLinearColor AccelerationZoneColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Build spline from points */
	void BuildSplineFromPoints();

	/** Update visual representation */
	void UpdateVisuals();

	/** Get interpolated point data at distance */
	FMGRacingLinePoint GetInterpolatedPointData(float Distance) const;
};

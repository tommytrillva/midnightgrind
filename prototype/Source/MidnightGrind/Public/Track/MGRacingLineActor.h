// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MGRacingLineActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

/**
 * Racing line point data
 */
USTRUCT(BlueprintType)
struct FMGRacingLinePoint
{
	GENERATED_BODY()

	/** World position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Position = FVector::ZeroVector;

	/** Ideal speed at this point (KPH) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IdealSpeed = 100.0f;

	/** Track width at this point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TrackWidth = 1000.0f;

	/** Is braking zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsBrakingZone = false;

	/** Is acceleration zone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAccelerationZone = false;

	/** Suggested gear */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SuggestedGear = 3;
};

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

// Copyright Midnight Grind. All Rights Reserved.

#include "Track/MGRacingLineActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

AMGRacingLineActor::AMGRacingLineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create spline component
	RacingLineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RacingLineSpline"));
	RootComponent = RacingLineSpline;

	RacingLineSpline->SetClosedLoop(bIsClosedLoop);
}

void AMGRacingLineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	BuildSplineFromPoints();
	UpdateVisuals();
}

void AMGRacingLineActor::BeginPlay()
{
	Super::BeginPlay();

	// Set visibility based on settings
	SetLineVisible(bShowInGame);
}

// ==========================================
// QUERY FUNCTIONS
// ==========================================

FVector AMGRacingLineActor::GetPositionAtDistance(float Distance) const
{
	if (!RacingLineSpline)
	{
		return FVector::ZeroVector;
	}

	return RacingLineSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator AMGRacingLineActor::GetDirectionAtDistance(float Distance) const
{
	if (!RacingLineSpline)
	{
		return FRotator::ZeroRotator;
	}

	return RacingLineSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float AMGRacingLineActor::GetIdealSpeedAtDistance(float Distance) const
{
	FMGRacingLinePoint PointData = GetInterpolatedPointData(Distance);
	return PointData.IdealSpeed;
}

FVector AMGRacingLineActor::GetClosestPointOnLine(FVector WorldPosition) const
{
	if (!RacingLineSpline)
	{
		return WorldPosition;
	}

	float Key = RacingLineSpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	return RacingLineSpline->GetLocationAtSplineInputKey(Key, ESplineCoordinateSpace::World);
}

float AMGRacingLineActor::GetDistanceAlongLine(FVector WorldPosition) const
{
	if (!RacingLineSpline)
	{
		return 0.0f;
	}

	float Key = RacingLineSpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	return RacingLineSpline->GetDistanceAlongSplineAtSplineInputKey(Key);
}

float AMGRacingLineActor::GetDeviationFromLine(FVector WorldPosition) const
{
	FVector ClosestPoint = GetClosestPointOnLine(WorldPosition);
	return FVector::Dist(WorldPosition, ClosestPoint);
}

float AMGRacingLineActor::GetTotalLength() const
{
	if (!RacingLineSpline)
	{
		return 0.0f;
	}

	return RacingLineSpline->GetSplineLength();
}

bool AMGRacingLineActor::IsInBrakingZone(float Distance) const
{
	FMGRacingLinePoint PointData = GetInterpolatedPointData(Distance);
	return PointData.bIsBrakingZone;
}

// ==========================================
// MODIFICATION
// ==========================================

void AMGRacingLineActor::SetRacingLineFromPoints(const TArray<FMGRacingLinePoint>& Points)
{
	RacingLinePoints = Points;
	BuildSplineFromPoints();
	UpdateVisuals();
}

void AMGRacingLineActor::AddRacingLinePoint(const FMGRacingLinePoint& Point)
{
	RacingLinePoints.Add(Point);
	BuildSplineFromPoints();
	UpdateVisuals();
}

void AMGRacingLineActor::ClearRacingLine()
{
	RacingLinePoints.Empty();
	if (RacingLineSpline)
	{
		RacingLineSpline->ClearSplinePoints();
	}
}

// ==========================================
// VISUALIZATION
// ==========================================

void AMGRacingLineActor::SetLineVisible(bool bVisible)
{
	if (RacingLineSpline)
	{
		RacingLineSpline->SetVisibility(bVisible, true);
	}
}

void AMGRacingLineActor::SetLineColor(FLinearColor Color)
{
	LineColor = Color;
	UpdateVisuals();
}

void AMGRacingLineActor::SetShowSpeedZones(bool bShow)
{
	bShowSpeedZones = bShow;
	UpdateVisuals();
}

// ==========================================
// INTERNAL
// ==========================================

void AMGRacingLineActor::BuildSplineFromPoints()
{
	if (!RacingLineSpline)
	{
		return;
	}

	RacingLineSpline->ClearSplinePoints();

	if (RacingLinePoints.Num() == 0)
	{
		return;
	}

	// Add spline points
	for (int32 i = 0; i < RacingLinePoints.Num(); ++i)
	{
		const FMGRacingLinePoint& Point = RacingLinePoints[i];
		RacingLineSpline->AddSplinePoint(Point.Position, ESplineCoordinateSpace::World, false);
	}

	RacingLineSpline->SetClosedLoop(bIsClosedLoop);
	RacingLineSpline->UpdateSpline();
}

void AMGRacingLineActor::UpdateVisuals()
{
	// Visual updates would set up spline mesh components or materials
	// Implementation depends on desired visual style
}

FMGRacingLinePoint AMGRacingLineActor::GetInterpolatedPointData(float Distance) const
{
	FMGRacingLinePoint Result;

	if (RacingLinePoints.Num() == 0 || !RacingLineSpline)
	{
		return Result;
	}

	float TotalLength = RacingLineSpline->GetSplineLength();
	if (TotalLength <= 0.0f)
	{
		return RacingLinePoints[0];
	}

	// Handle looping
	if (bIsClosedLoop)
	{
		Distance = FMath::Fmod(Distance, TotalLength);
		if (Distance < 0.0f)
		{
			Distance += TotalLength;
		}
	}
	else
	{
		Distance = FMath::Clamp(Distance, 0.0f, TotalLength);
	}

	// Find surrounding points
	float DistancePerPoint = TotalLength / RacingLinePoints.Num();

	int32 LowerIndex = FMath::FloorToInt(Distance / DistancePerPoint);
	int32 UpperIndex = LowerIndex + 1;

	if (bIsClosedLoop)
	{
		LowerIndex = LowerIndex % RacingLinePoints.Num();
		UpperIndex = UpperIndex % RacingLinePoints.Num();
	}
	else
	{
		LowerIndex = FMath::Clamp(LowerIndex, 0, RacingLinePoints.Num() - 1);
		UpperIndex = FMath::Clamp(UpperIndex, 0, RacingLinePoints.Num() - 1);
	}

	// Interpolate
	float LowerDist = LowerIndex * DistancePerPoint;
	float Alpha = (Distance - LowerDist) / DistancePerPoint;
	Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);

	const FMGRacingLinePoint& Lower = RacingLinePoints[LowerIndex];
	const FMGRacingLinePoint& Upper = RacingLinePoints[UpperIndex];

	Result.Position = FMath::Lerp(Lower.Position, Upper.Position, Alpha);
	Result.IdealSpeed = FMath::Lerp(Lower.IdealSpeed, Upper.IdealSpeed, Alpha);
	Result.TrackWidth = FMath::Lerp(Lower.TrackWidth, Upper.TrackWidth, Alpha);
	Result.bIsBrakingZone = Alpha < 0.5f ? Lower.bIsBrakingZone : Upper.bIsBrakingZone;
	Result.bIsAccelerationZone = Alpha < 0.5f ? Lower.bIsAccelerationZone : Upper.bIsAccelerationZone;
	Result.SuggestedGear = Alpha < 0.5f ? Lower.SuggestedGear : Upper.SuggestedGear;

	return Result;
}

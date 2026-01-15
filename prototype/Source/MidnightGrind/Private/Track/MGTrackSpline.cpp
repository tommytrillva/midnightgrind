// Copyright Midnight Grind. All Rights Reserved.

#include "Track/MGTrackSpline.h"
#include "Components/SplineComponent.h"

AMGTrackSpline::AMGTrackSpline()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create racing line spline
	RacingLineSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RacingLineSpline"));
	RacingLineSpline->SetupAttachment(Root);
	RacingLineSpline->SetClosedLoop(true);
	RacingLineSpline->SetDrawDebug(true);

	// Create boundary splines
	InnerBoundarySpline = CreateDefaultSubobject<USplineComponent>(TEXT("InnerBoundarySpline"));
	InnerBoundarySpline->SetupAttachment(Root);
	InnerBoundarySpline->SetClosedLoop(true);
	InnerBoundarySpline->SetDrawDebug(false);

	OuterBoundarySpline = CreateDefaultSubobject<USplineComponent>(TEXT("OuterBoundarySpline"));
	OuterBoundarySpline->SetupAttachment(Root);
	OuterBoundarySpline->SetClosedLoop(true);
	OuterBoundarySpline->SetDrawDebug(false);

	// Add default spline points
	RacingLineSpline->ClearSplinePoints();
	RacingLineSpline->AddSplinePoint(FVector(0.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
	RacingLineSpline->AddSplinePoint(FVector(1000.0f, 0.0f, 0.0f), ESplineCoordinateSpace::Local);
	RacingLineSpline->AddSplinePoint(FVector(1000.0f, 1000.0f, 0.0f), ESplineCoordinateSpace::Local);
	RacingLineSpline->AddSplinePoint(FVector(0.0f, 1000.0f, 0.0f), ESplineCoordinateSpace::Local);
}

void AMGTrackSpline::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Update spline settings
	RacingLineSpline->SetClosedLoop(bClosedLoop);
	InnerBoundarySpline->SetClosedLoop(bClosedLoop);
	OuterBoundarySpline->SetClosedLoop(bClosedLoop);

	// Calculate track properties
	CalculateTrackLength();
	GenerateBoundarySplines();
}

void AMGTrackSpline::BeginPlay()
{
	Super::BeginPlay();

	CalculateTrackLength();
}

void AMGTrackSpline::CalculateTrackLength()
{
	if (RacingLineSpline)
	{
		TrackLength = RacingLineSpline->GetSplineLength();
	}
}

void AMGTrackSpline::GenerateBoundarySplines()
{
	if (!RacingLineSpline)
	{
		return;
	}

	// Clear existing boundary points
	InnerBoundarySpline->ClearSplinePoints();
	OuterBoundarySpline->ClearSplinePoints();

	// Generate boundary points based on racing line
	int32 NumPoints = RacingLineSpline->GetNumberOfSplinePoints();
	float SplineLength = RacingLineSpline->GetSplineLength();

	// Sample more densely for smooth boundaries
	int32 NumSamples = FMath::Max(NumPoints * 4, 32);
	float SampleStep = SplineLength / NumSamples;

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Distance = i * SampleStep;

		FVector Position = RacingLineSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		FVector RightVector = RacingLineSpline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		float HalfWidth = GetTrackWidthAtDistance(Distance) * 0.5f;

		FVector InnerPoint = Position - RightVector * HalfWidth;
		FVector OuterPoint = Position + RightVector * HalfWidth;

		InnerBoundarySpline->AddSplinePoint(InnerPoint, ESplineCoordinateSpace::World, false);
		OuterBoundarySpline->AddSplinePoint(OuterPoint, ESplineCoordinateSpace::World, false);
	}

	InnerBoundarySpline->UpdateSpline();
	OuterBoundarySpline->UpdateSpline();
}

// ==========================================
// QUERIES
// ==========================================

FVector AMGTrackSpline::GetPositionAtDistance(float Distance) const
{
	if (!RacingLineSpline)
	{
		return FVector::ZeroVector;
	}

	// Handle looping
	if (bClosedLoop && TrackLength > 0.0f)
	{
		Distance = FMath::Fmod(Distance, TrackLength);
		if (Distance < 0.0f)
		{
			Distance += TrackLength;
		}
	}

	return RacingLineSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator AMGTrackSpline::GetRotationAtDistance(float Distance) const
{
	if (!RacingLineSpline)
	{
		return FRotator::ZeroRotator;
	}

	// Handle looping
	if (bClosedLoop && TrackLength > 0.0f)
	{
		Distance = FMath::Fmod(Distance, TrackLength);
		if (Distance < 0.0f)
		{
			Distance += TrackLength;
		}
	}

	return RacingLineSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FTransform AMGTrackSpline::GetTransformAtDistance(float Distance) const
{
	if (!RacingLineSpline)
	{
		return FTransform::Identity;
	}

	// Handle looping
	if (bClosedLoop && TrackLength > 0.0f)
	{
		Distance = FMath::Fmod(Distance, TrackLength);
		if (Distance < 0.0f)
		{
			Distance += TrackLength;
		}
	}

	return RacingLineSpline->GetTransformAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

float AMGTrackSpline::GetClosestDistanceOnTrack(const FVector& WorldPosition) const
{
	if (!RacingLineSpline)
	{
		return 0.0f;
	}

	// Find closest point on spline
	float InputKey = RacingLineSpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	return RacingLineSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);
}

float AMGTrackSpline::GetTrackWidthAtDistance(float Distance) const
{
	const FMGTrackSegment* Segment = GetSegmentAtDistance(Distance);
	if (Segment)
	{
		return Segment->TrackWidth;
	}
	return DefaultTrackWidth;
}

EMGTrackSurface AMGTrackSpline::GetSurfaceAtDistance(float Distance) const
{
	const FMGTrackSegment* Segment = GetSegmentAtDistance(Distance);
	if (Segment)
	{
		return Segment->Surface;
	}
	return EMGTrackSurface::Asphalt;
}

bool AMGTrackSpline::IsPositionOnTrack(const FVector& WorldPosition) const
{
	float Distance = GetClosestDistanceOnTrack(WorldPosition);
	FVector TrackPosition = GetPositionAtDistance(Distance);

	float DistanceFromTrack = FVector::Dist2D(WorldPosition, TrackPosition);
	float HalfWidth = GetTrackWidthAtDistance(Distance) * 0.5f;

	return DistanceFromTrack <= HalfWidth;
}

float AMGTrackSpline::GetLateralDistanceFromRacingLine(const FVector& WorldPosition) const
{
	if (!RacingLineSpline)
	{
		return 0.0f;
	}

	float Distance = GetClosestDistanceOnTrack(WorldPosition);
	FVector TrackPosition = GetPositionAtDistance(Distance);
	FVector RightVector = RacingLineSpline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

	FVector ToPosition = WorldPosition - TrackPosition;

	// Positive = right of racing line, negative = left
	return FVector::DotProduct(ToPosition, RightVector);
}

// ==========================================
// AI HELPERS
// ==========================================

float AMGTrackSpline::GetSuggestedSpeedAtDistance(float Distance) const
{
	const FMGTrackSegment* Segment = GetSegmentAtDistance(Distance);
	if (Segment && Segment->SuggestedSpeed > 0.0f)
	{
		return Segment->SuggestedSpeed;
	}

	// Calculate based on curvature
	float Curvature = GetCurvatureAtDistance(Distance);

	// Higher curvature = lower speed
	// This is a simplified formula - real AI would be more sophisticated
	float BaseSpe = 200.0f; // Max speed in MPH
	float SpeedReduction = Curvature * 1000.0f; // Adjust multiplier for tuning

	return FMath::Max(30.0f, BaseSpe - SpeedReduction);
}

FVector AMGTrackSpline::GetLookAheadPoint(float CurrentDistance, float LookAheadDistance) const
{
	return GetPositionAtDistance(CurrentDistance + LookAheadDistance);
}

float AMGTrackSpline::GetCurvatureAtDistance(float Distance) const
{
	if (!RacingLineSpline)
	{
		return 0.0f;
	}

	// Handle looping
	if (bClosedLoop && TrackLength > 0.0f)
	{
		Distance = FMath::Fmod(Distance, TrackLength);
		if (Distance < 0.0f)
		{
			Distance += TrackLength;
		}
	}

	// Sample tangent at two nearby points to estimate curvature
	float SampleOffset = 100.0f; // cm

	FVector Tangent1 = RacingLineSpline->GetDirectionAtDistanceAlongSpline(
		FMath::Max(0.0f, Distance - SampleOffset), ESplineCoordinateSpace::World);
	FVector Tangent2 = RacingLineSpline->GetDirectionAtDistanceAlongSpline(
		FMath::Min(TrackLength, Distance + SampleOffset), ESplineCoordinateSpace::World);

	// Curvature approximation
	float DotProduct = FVector::DotProduct(Tangent1, Tangent2);
	return 1.0f - DotProduct; // 0 = straight, ~2 = 180 degree turn
}

// ==========================================
// RESPAWN
// ==========================================

FTransform AMGTrackSpline::GetRespawnTransformAtDistance(float Distance) const
{
	FTransform Transform = GetTransformAtDistance(Distance);

	// Lift slightly above track
	FVector Location = Transform.GetLocation();
	Location.Z += 50.0f;
	Transform.SetLocation(Location);

	return Transform;
}

FTransform AMGTrackSpline::GetNearestRespawnTransform(const FVector& WorldPosition) const
{
	float Distance = GetClosestDistanceOnTrack(WorldPosition);
	return GetRespawnTransformAtDistance(Distance);
}

// ==========================================
// INTERNAL
// ==========================================

const FMGTrackSegment* AMGTrackSpline::GetSegmentAtDistance(float Distance) const
{
	// Handle looping
	if (bClosedLoop && TrackLength > 0.0f)
	{
		Distance = FMath::Fmod(Distance, TrackLength);
		if (Distance < 0.0f)
		{
			Distance += TrackLength;
		}
	}

	// Find segment that contains this distance
	const FMGTrackSegment* CurrentSegment = nullptr;

	for (const FMGTrackSegment& Segment : TrackSegments)
	{
		if (Segment.StartDistance <= Distance)
		{
			CurrentSegment = &Segment;
		}
		else
		{
			break;
		}
	}

	return CurrentSegment;
}

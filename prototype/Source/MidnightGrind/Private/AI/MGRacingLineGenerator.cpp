// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGRacingLineGenerator.h"
#include "Components/SplineComponent.h"
#include "Track/MGCheckpointActor.h"
#include "DrawDebugHelpers.h"

UMGRacingLineGenerator::UMGRacingLineGenerator()
{
}

// ==========================================
// GENERATION
// ==========================================

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::GenerateFromSpline(
	USplineComponent* TrackSpline,
	float TrackWidth,
	const FMGRacingLineParams& Params)
{
	TArray<FMGAIRacingLinePoint> Result;

	if (!TrackSpline)
	{
		return Result;
	}

	// 1. Sample centerline points
	TArray<FVector> CenterPoints = SampleSplinePoints(TrackSpline, Params.SamplingInterval);

	if (CenterPoints.Num() < 3)
	{
		return Result;
	}

	// 2. Calculate curvatures
	TArray<float> Curvatures = CalculateCurvatures(CenterPoints);

	// 3. Generate lateral offsets for racing line
	TArray<float> LateralOffsets = GenerateLateralOffsets(
		CenterPoints,
		Curvatures,
		TrackWidth,
		Params
	);

	// 4. Apply offsets to create racing line positions
	TArray<FVector> RacingLinePositions = ApplyLateralOffsets(CenterPoints, LateralOffsets, TrackSpline);

	// 5. Smooth the racing line
	for (int32 Iteration = 0; Iteration < Params.SmoothingIterations; ++Iteration)
	{
		TArray<FVector> SmoothedPositions;
		SmoothedPositions.Reserve(RacingLinePositions.Num());

		for (int32 i = 0; i < RacingLinePositions.Num(); ++i)
		{
			int32 Prev = (i - 1 + RacingLinePositions.Num()) % RacingLinePositions.Num();
			int32 Next = (i + 1) % RacingLinePositions.Num();

			FVector Smoothed = (RacingLinePositions[Prev] + RacingLinePositions[i] * 2.0f + RacingLinePositions[Next]) / 4.0f;
			SmoothedPositions.Add(Smoothed);
		}

		RacingLinePositions = SmoothedPositions;
	}

	// 6. Recalculate curvatures for smoothed line
	Curvatures = CalculateCurvatures(RacingLinePositions);

	// 7. Calculate target speeds
	TArray<float> TargetSpeeds = CalculateTargetSpeeds(RacingLinePositions, Curvatures, Params);

	// 8. Apply braking zones (backward pass)
	ApplyBrakingZones(TargetSpeeds, RacingLinePositions, Params.BrakingDecel);

	// 9. Apply acceleration zones (forward pass)
	ApplyAccelerationZones(TargetSpeeds, RacingLinePositions, Params.AccelerationRate);

	// 10. Build final racing line points
	Result = BuildRacingLinePoints(RacingLinePositions, TargetSpeeds, Curvatures, TrackWidth);

	LastGeneratedLine = Result;
	OnRacingLineGenerated.Broadcast(Result);

	return Result;
}

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::GenerateFromCheckpoints(
	const TArray<AMGCheckpointActor*>& Checkpoints,
	float TrackWidth,
	const FMGRacingLineParams& Params)
{
	TArray<FMGAIRacingLinePoint> Result;

	if (Checkpoints.Num() < 3)
	{
		return Result;
	}

	// Extract positions from checkpoints
	TArray<FVector> CheckpointPositions;
	for (const AMGCheckpointActor* CP : Checkpoints)
	{
		if (CP)
		{
			CheckpointPositions.Add(CP->GetActorLocation());
		}
	}

	return GenerateFromPoints(CheckpointPositions, TrackWidth, Params);
}

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::GenerateFromPoints(
	const TArray<FVector>& CenterlinePoints,
	float TrackWidth,
	const FMGRacingLineParams& Params)
{
	TArray<FMGAIRacingLinePoint> Result;

	if (CenterlinePoints.Num() < 3)
	{
		return Result;
	}

	// Resample points to consistent spacing
	TArray<FVector> ResampledPoints;
	ResampledPoints.Reserve(CenterlinePoints.Num() * 2);

	float AccumulatedDistance = 0.0f;

	for (int32 i = 0; i < CenterlinePoints.Num(); ++i)
	{
		int32 Next = (i + 1) % CenterlinePoints.Num();
		FVector Current = CenterlinePoints[i];
		FVector NextPoint = CenterlinePoints[Next];

		float SegmentLength = FVector::Dist(Current, NextPoint);
		int32 NumSamples = FMath::Max(1, FMath::CeilToInt(SegmentLength / Params.SamplingInterval));

		for (int32 j = 0; j < NumSamples; ++j)
		{
			float T = (float)j / (float)NumSamples;
			FVector Point = FMath::Lerp(Current, NextPoint, T);
			ResampledPoints.Add(Point);
		}
	}

	// Calculate curvatures
	TArray<float> Curvatures = CalculateCurvatures(ResampledPoints);

	// Generate lateral offsets
	TArray<float> LateralOffsets = GenerateLateralOffsets(
		ResampledPoints,
		Curvatures,
		TrackWidth,
		Params
	);

	// Apply offsets (without spline - use direction-based perpendicular)
	TArray<FVector> RacingLinePositions;
	RacingLinePositions.Reserve(ResampledPoints.Num());

	for (int32 i = 0; i < ResampledPoints.Num(); ++i)
	{
		int32 Prev = (i - 1 + ResampledPoints.Num()) % ResampledPoints.Num();
		int32 Next = (i + 1) % ResampledPoints.Num();

		FVector Direction = (ResampledPoints[Next] - ResampledPoints[Prev]).GetSafeNormal();
		FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();

		FVector Position = ResampledPoints[i] + Perpendicular * LateralOffsets[i];
		RacingLinePositions.Add(Position);
	}

	// Smooth
	for (int32 Iteration = 0; Iteration < Params.SmoothingIterations; ++Iteration)
	{
		TArray<FVector> SmoothedPositions;
		SmoothedPositions.Reserve(RacingLinePositions.Num());

		for (int32 i = 0; i < RacingLinePositions.Num(); ++i)
		{
			int32 Prev = (i - 1 + RacingLinePositions.Num()) % RacingLinePositions.Num();
			int32 Next = (i + 1) % RacingLinePositions.Num();

			FVector Smoothed = (RacingLinePositions[Prev] + RacingLinePositions[i] * 2.0f + RacingLinePositions[Next]) / 4.0f;
			SmoothedPositions.Add(Smoothed);
		}

		RacingLinePositions = SmoothedPositions;
	}

	// Recalculate curvatures and speeds
	Curvatures = CalculateCurvatures(RacingLinePositions);
	TArray<float> TargetSpeeds = CalculateTargetSpeeds(RacingLinePositions, Curvatures, Params);
	ApplyBrakingZones(TargetSpeeds, RacingLinePositions, Params.BrakingDecel);
	ApplyAccelerationZones(TargetSpeeds, RacingLinePositions, Params.AccelerationRate);

	Result = BuildRacingLinePoints(RacingLinePositions, TargetSpeeds, Curvatures, TrackWidth);

	LastGeneratedLine = Result;
	OnRacingLineGenerated.Broadcast(Result);

	return Result;
}

// ==========================================
// ANALYSIS
// ==========================================

TArray<FMGCornerData> UMGRacingLineGenerator::AnalyzeTrackCorners(
	USplineComponent* TrackSpline,
	float CurvatureThreshold)
{
	TArray<FMGCornerData> Corners;

	if (!TrackSpline)
	{
		return Corners;
	}

	float SplineLength = TrackSpline->GetSplineLength();
	float SampleInterval = 5.0f; // meters

	TArray<float> Curvatures;
	TArray<float> Distances;

	// Sample curvatures along spline
	for (float Distance = 0.0f; Distance < SplineLength; Distance += SampleInterval)
	{
		FVector Location = TrackSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		FVector Tangent = TrackSpline->GetTangentAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);

		float NextDistance = FMath::Min(Distance + SampleInterval, SplineLength - 1.0f);
		FVector NextTangent = TrackSpline->GetTangentAtDistanceAlongSpline(NextDistance, ESplineCoordinateSpace::World);

		float Curvature = FMath::Acos(FMath::Clamp(FVector::DotProduct(Tangent.GetSafeNormal(), NextTangent.GetSafeNormal()), -1.0f, 1.0f)) / SampleInterval;

		Curvatures.Add(Curvature);
		Distances.Add(Distance);
	}

	// Find corner regions (above curvature threshold)
	bool bInCorner = false;
	FMGCornerData CurrentCorner;
	float MaxCurvature = 0.0f;
	float CurvatureSum = 0.0f;
	int32 SampleCount = 0;

	for (int32 i = 0; i < Curvatures.Num(); ++i)
	{
		if (Curvatures[i] > CurvatureThreshold)
		{
			if (!bInCorner)
			{
				// Start of corner
				bInCorner = true;
				CurrentCorner = FMGCornerData();
				CurrentCorner.StartDistance = Distances[i];
				MaxCurvature = 0.0f;
				CurvatureSum = 0.0f;
				SampleCount = 0;
			}

			// Track apex (point of highest curvature)
			if (Curvatures[i] > MaxCurvature)
			{
				MaxCurvature = Curvatures[i];
				CurrentCorner.ApexDistance = Distances[i];
			}

			CurvatureSum += Curvatures[i];
			SampleCount++;

			// Determine direction from tangent cross product
			if (i < Distances.Num() - 1)
			{
				FVector Tangent = TrackSpline->GetTangentAtDistanceAlongSpline(Distances[i], ESplineCoordinateSpace::World);
				FVector NextTangent = TrackSpline->GetTangentAtDistanceAlongSpline(Distances[i] + SampleInterval, ESplineCoordinateSpace::World);
				FVector Cross = FVector::CrossProduct(Tangent, NextTangent);
				CurrentCorner.Direction += Cross.Z > 0 ? 1.0f : -1.0f;
			}
		}
		else if (bInCorner)
		{
			// End of corner
			bInCorner = false;
			CurrentCorner.EndDistance = Distances[FMath::Max(0, i - 1)];

			// Calculate average curvature and radius
			float AvgCurvature = CurvatureSum / FMath::Max(1, SampleCount);
			CurrentCorner.Radius = AvgCurvature > 0.001f ? 1.0f / AvgCurvature : 1000.0f;

			// Normalize direction
			CurrentCorner.Direction = FMath::Sign(CurrentCorner.Direction);

			// Calculate speeds
			CurrentCorner.ApexSpeed = CalculateCornerSpeed(MaxCurvature, 1.2f);
			CurrentCorner.EntrySpeed = CurrentCorner.ApexSpeed * 1.2f;

			// Braking zone
			float SpeedDiff = CurrentCorner.EntrySpeed - CurrentCorner.ApexSpeed;
			float BrakingDistance = (SpeedDiff * SpeedDiff) / (2.0f * 15.0f); // v^2 / 2a
			CurrentCorner.BrakingZoneStart = CurrentCorner.StartDistance - BrakingDistance;

			// Detect hairpin (very tight, > 90 degree turn)
			CurrentCorner.bIsHairpin = CurrentCorner.Radius < 20.0f;

			Corners.Add(CurrentCorner);
		}
	}

	// Detect chicanes (alternating direction corners close together)
	for (int32 i = 0; i < Corners.Num() - 1; ++i)
	{
		if (Corners[i].Direction * Corners[i + 1].Direction < 0)
		{
			float Gap = Corners[i + 1].StartDistance - Corners[i].EndDistance;
			if (Gap < 50.0f) // Close together
			{
				Corners[i].bIsChicane = true;
				Corners[i + 1].bIsChicane = true;
			}
		}
	}

	return Corners;
}

TArray<FMGTrackSegment> UMGRacingLineGenerator::SegmentTrack(
	USplineComponent* TrackSpline,
	float SegmentLength)
{
	TArray<FMGTrackSegment> Segments;

	if (!TrackSpline)
	{
		return Segments;
	}

	float SplineLength = TrackSpline->GetSplineLength();
	int32 NumSegments = FMath::CeilToInt(SplineLength / SegmentLength);

	for (int32 i = 0; i < NumSegments; ++i)
	{
		float StartDist = i * SegmentLength;
		float EndDist = FMath::Min((i + 1) * SegmentLength, SplineLength);

		FMGTrackSegment Segment;
		Segment.StartDistance = StartDist;
		Segment.EndDistance = EndDist;

		// Calculate average curvature in segment
		float CurvatureSum = 0.0f;
		int32 Samples = 0;
		float DirectionSum = 0.0f;

		for (float D = StartDist; D < EndDist; D += 2.0f)
		{
			FVector T1 = TrackSpline->GetTangentAtDistanceAlongSpline(D, ESplineCoordinateSpace::World);
			FVector T2 = TrackSpline->GetTangentAtDistanceAlongSpline(D + 2.0f, ESplineCoordinateSpace::World);

			float Angle = FMath::Acos(FMath::Clamp(FVector::DotProduct(T1.GetSafeNormal(), T2.GetSafeNormal()), -1.0f, 1.0f));
			CurvatureSum += Angle / 2.0f;

			FVector Cross = FVector::CrossProduct(T1, T2);
			DirectionSum += Cross.Z;

			Samples++;
		}

		Segment.Curvature = Samples > 0 ? CurvatureSum / Samples : 0.0f;

		// Determine segment type
		if (Segment.Curvature < 0.01f)
		{
			Segment.Type = EMGTrackSegmentType::Straight;
		}
		else if (DirectionSum > 0)
		{
			Segment.Type = Segment.Curvature > 0.1f ? EMGTrackSegmentType::Hairpin : EMGTrackSegmentType::RightTurn;
		}
		else
		{
			Segment.Type = Segment.Curvature > 0.1f ? EMGTrackSegmentType::Hairpin : EMGTrackSegmentType::LeftTurn;
		}

		Segment.SuggestedSpeed = CalculateCornerSpeed(Segment.Curvature, 1.2f);

		Segments.Add(Segment);
	}

	return Segments;
}

float UMGRacingLineGenerator::CalculateCornerSpeed(float Curvature, float MaxLateralG) const
{
	// v = sqrt(a * r) where r = 1/curvature
	// a = lateral G * 9.81 m/s^2

	if (Curvature < 0.001f)
	{
		return 1000.0f; // Effectively straight
	}

	float Radius = 1.0f / Curvature;
	float LateralAccel = MaxLateralG * 9.81f;
	float Speed = FMath::Sqrt(LateralAccel * Radius);

	return Speed; // m/s
}

// ==========================================
// MODIFICATION
// ==========================================

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::AdjustForSkillLevel(
	const TArray<FMGAIRacingLinePoint>& BaseLine,
	float SkillLevel)
{
	TArray<FMGAIRacingLinePoint> AdjustedLine = BaseLine;

	// Lower skill = slower speeds, more center-line following
	float SpeedMultiplier = FMath::Lerp(0.7f, 1.0f, SkillLevel);
	float WidthReduction = 1.0f - SkillLevel;

	for (FMGAIRacingLinePoint& Point : AdjustedLine)
	{
		// Reduce target speed
		Point.TargetSpeed *= SpeedMultiplier;

		// Pull line toward center (would need original centerline for this)
		// For now, just note that width usage should decrease
	}

	return AdjustedLine;
}

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::AddVariation(
	const TArray<FMGAIRacingLinePoint>& BaseLine,
	float VariationAmount,
	int32 RandomSeed)
{
	TArray<FMGAIRacingLinePoint> VariedLine = BaseLine;

	FRandomStream Random(RandomSeed);

	for (int32 i = 0; i < VariedLine.Num(); ++i)
	{
		FMGAIRacingLinePoint& Point = VariedLine[i];

		// Add lateral variation
		float LateralOffset = Random.FRandRange(-1.0f, 1.0f) * VariationAmount * Point.TrackWidth;
		FVector Perpendicular = FVector::CrossProduct(Point.Direction, FVector::UpVector).GetSafeNormal();
		Point.Position += Perpendicular * LateralOffset;

		// Add speed variation
		float SpeedVariation = Random.FRandRange(0.95f, 1.05f);
		Point.TargetSpeed *= SpeedVariation;
	}

	// Smooth the varied line
	return SmoothRacingLine(VariedLine, 2);
}

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::SmoothRacingLine(
	const TArray<FMGAIRacingLinePoint>& RacingLine,
	int32 Iterations)
{
	TArray<FMGAIRacingLinePoint> SmoothedLine = RacingLine;

	for (int32 Iter = 0; Iter < Iterations; ++Iter)
	{
		TArray<FMGAIRacingLinePoint> TempLine = SmoothedLine;

		for (int32 i = 0; i < SmoothedLine.Num(); ++i)
		{
			int32 Prev = (i - 1 + SmoothedLine.Num()) % SmoothedLine.Num();
			int32 Next = (i + 1) % SmoothedLine.Num();

			TempLine[i].Position = (SmoothedLine[Prev].Position + SmoothedLine[i].Position * 2.0f + SmoothedLine[Next].Position) / 4.0f;
			TempLine[i].TargetSpeed = (SmoothedLine[Prev].TargetSpeed + SmoothedLine[i].TargetSpeed * 2.0f + SmoothedLine[Next].TargetSpeed) / 4.0f;
		}

		SmoothedLine = TempLine;
	}

	// Recalculate directions
	for (int32 i = 0; i < SmoothedLine.Num(); ++i)
	{
		int32 Next = (i + 1) % SmoothedLine.Num();
		SmoothedLine[i].Direction = (SmoothedLine[Next].Position - SmoothedLine[i].Position).GetSafeNormal();
	}

	return SmoothedLine;
}

// ==========================================
// UTILITIES
// ==========================================

float UMGRacingLineGenerator::CalculateTrackLength(const TArray<FMGAIRacingLinePoint>& RacingLine) const
{
	if (RacingLine.Num() < 2)
	{
		return 0.0f;
	}

	float TotalLength = 0.0f;
	for (int32 i = 0; i < RacingLine.Num(); ++i)
	{
		int32 Next = (i + 1) % RacingLine.Num();
		TotalLength += FVector::Dist(RacingLine[i].Position, RacingLine[Next].Position);
	}

	return TotalLength;
}

FMGAIRacingLinePoint UMGRacingLineGenerator::GetPointAtDistance(
	const TArray<FMGAIRacingLinePoint>& RacingLine,
	float Distance) const
{
	if (RacingLine.Num() == 0)
	{
		return FMGAIRacingLinePoint();
	}

	// Find segment containing distance
	for (int32 i = 0; i < RacingLine.Num(); ++i)
	{
		int32 Next = (i + 1) % RacingLine.Num();

		if (Distance >= RacingLine[i].DistanceAlongTrack && Distance < RacingLine[Next].DistanceAlongTrack)
		{
			float T = (Distance - RacingLine[i].DistanceAlongTrack) /
				FMath::Max(0.001f, RacingLine[Next].DistanceAlongTrack - RacingLine[i].DistanceAlongTrack);

			FMGAIRacingLinePoint Result;
			Result.Position = FMath::Lerp(RacingLine[i].Position, RacingLine[Next].Position, T);
			Result.Direction = FMath::Lerp(RacingLine[i].Direction, RacingLine[Next].Direction, T).GetSafeNormal();
			Result.TargetSpeed = FMath::Lerp(RacingLine[i].TargetSpeed, RacingLine[Next].TargetSpeed, T);
			Result.TrackWidth = FMath::Lerp(RacingLine[i].TrackWidth, RacingLine[Next].TrackWidth, T);
			Result.DistanceAlongTrack = Distance;

			return Result;
		}
	}

	// Wrap around or return last point
	return RacingLine.Last();
}

int32 UMGRacingLineGenerator::FindClosestPointIndex(
	const TArray<FMGAIRacingLinePoint>& RacingLine,
	const FVector& Position) const
{
	int32 ClosestIndex = 0;
	float ClosestDistSq = FLT_MAX;

	for (int32 i = 0; i < RacingLine.Num(); ++i)
	{
		float DistSq = FVector::DistSquared(RacingLine[i].Position, Position);
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestIndex = i;
		}
	}

	return ClosestIndex;
}

void UMGRacingLineGenerator::DebugDrawRacingLine(
	UWorld* World,
	const TArray<FMGAIRacingLinePoint>& RacingLine,
	float Duration,
	bool bDrawSpeeds)
{
	if (!World || RacingLine.Num() < 2)
	{
		return;
	}

	for (int32 i = 0; i < RacingLine.Num(); ++i)
	{
		int32 Next = (i + 1) % RacingLine.Num();
		const FMGAIRacingLinePoint& Point = RacingLine[i];
		const FMGAIRacingLinePoint& NextPoint = RacingLine[Next];

		// Color based on target speed (red = slow, green = fast)
		float SpeedNorm = FMath::Clamp(Point.TargetSpeed / 80.0f, 0.0f, 1.0f);
		FColor LineColor = FColor::MakeRedToGreenColorFromScalar(SpeedNorm);

		// Draw line segment
		DrawDebugLine(World, Point.Position, NextPoint.Position, LineColor, false, Duration, 0, 2.0f);

		// Draw apex markers
		if (Point.bIsApex)
		{
			DrawDebugSphere(World, Point.Position, 50.0f, 8, FColor::Yellow, false, Duration);
		}

		// Draw braking zones
		if (Point.bIsBrakingZone)
		{
			DrawDebugSphere(World, Point.Position, 30.0f, 4, FColor::Red, false, Duration);
		}

		// Draw speed text
		if (bDrawSpeeds && i % 10 == 0)
		{
			FString SpeedText = FString::Printf(TEXT("%.0f"), Point.TargetSpeed * 3.6f); // m/s to km/h
			DrawDebugString(World, Point.Position + FVector(0, 0, 100), SpeedText, nullptr, FColor::White, Duration);
		}
	}
}

// ==========================================
// INTERNAL GENERATION
// ==========================================

TArray<FVector> UMGRacingLineGenerator::SampleSplinePoints(
	USplineComponent* Spline,
	float SamplingInterval)
{
	TArray<FVector> Points;

	if (!Spline)
	{
		return Points;
	}

	float SplineLength = Spline->GetSplineLength();
	int32 NumSamples = FMath::CeilToInt(SplineLength / SamplingInterval);

	Points.Reserve(NumSamples);

	for (int32 i = 0; i < NumSamples; ++i)
	{
		float Distance = (float)i * SamplingInterval;
		FVector Point = Spline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		Points.Add(Point);
	}

	return Points;
}

TArray<float> UMGRacingLineGenerator::CalculateCurvatures(const TArray<FVector>& Points)
{
	TArray<float> Curvatures;
	Curvatures.Reserve(Points.Num());

	for (int32 i = 0; i < Points.Num(); ++i)
	{
		int32 Prev = (i - 1 + Points.Num()) % Points.Num();
		int32 Next = (i + 1) % Points.Num();

		float Curvature = CalculateCurvatureFromPoints(Points[Prev], Points[i], Points[Next]);
		Curvatures.Add(Curvature);
	}

	return Curvatures;
}

TArray<float> UMGRacingLineGenerator::GenerateLateralOffsets(
	const TArray<FVector>& CenterPoints,
	const TArray<float>& Curvatures,
	float TrackWidth,
	const FMGRacingLineParams& Params)
{
	TArray<float> Offsets;
	Offsets.SetNumZeroed(CenterPoints.Num());

	float HalfWidth = TrackWidth * 0.5f * Params.WidthUsage;

	for (int32 i = 0; i < CenterPoints.Num(); ++i)
	{
		// Determine turn direction from curvature change
		int32 Prev = (i - 1 + CenterPoints.Num()) % CenterPoints.Num();
		int32 Next = (i + 1) % CenterPoints.Num();

		FVector ToPrev = CenterPoints[Prev] - CenterPoints[i];
		FVector ToNext = CenterPoints[Next] - CenterPoints[i];
		FVector Cross = FVector::CrossProduct(ToPrev, ToNext);

		float TurnDirection = FMath::Sign(Cross.Z); // Positive = right turn, Negative = left turn

		// Racing line goes to outside before turn, inside at apex, outside after
		float CurvatureHere = Curvatures[i];

		// Look ahead to find upcoming turn direction
		float LookAheadCurvature = 0.0f;
		int32 LookAheadSteps = FMath::Min(20, CenterPoints.Num() / 4);
		for (int32 j = 1; j <= LookAheadSteps; ++j)
		{
			int32 AheadIndex = (i + j) % CenterPoints.Num();
			LookAheadCurvature = FMath::Max(LookAheadCurvature, Curvatures[AheadIndex]);
		}

		// Calculate offset
		float Offset = 0.0f;

		if (CurvatureHere > 0.005f)
		{
			// In a turn - go to inside (apex)
			float ApexFactor = FMath::Min(1.0f, CurvatureHere / 0.05f);
			Offset = -TurnDirection * HalfWidth * ApexFactor * Params.CornerCuttingAggression;
		}
		else if (LookAheadCurvature > 0.005f)
		{
			// Approaching a turn - go to outside
			float OutsideFactor = FMath::Min(1.0f, LookAheadCurvature / 0.05f);
			// Need to determine which way the upcoming turn goes
			Offset = HalfWidth * OutsideFactor * 0.5f; // Simplified - would need turn direction
		}

		Offsets[i] = FMath::Clamp(Offset, -HalfWidth, HalfWidth);
	}

	return Offsets;
}

TArray<FVector> UMGRacingLineGenerator::ApplyLateralOffsets(
	const TArray<FVector>& CenterPoints,
	const TArray<float>& Offsets,
	USplineComponent* Spline)
{
	TArray<FVector> RacingLine;
	RacingLine.Reserve(CenterPoints.Num());

	for (int32 i = 0; i < CenterPoints.Num(); ++i)
	{
		FVector RightVector;

		if (Spline)
		{
			float Distance = (float)i * 5.0f; // Approximate
			RightVector = Spline->GetRightVectorAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		}
		else
		{
			int32 Next = (i + 1) % CenterPoints.Num();
			FVector Direction = (CenterPoints[Next] - CenterPoints[i]).GetSafeNormal();
			RightVector = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
		}

		FVector Position = CenterPoints[i] + RightVector * Offsets[i];
		RacingLine.Add(Position);
	}

	return RacingLine;
}

TArray<float> UMGRacingLineGenerator::CalculateTargetSpeeds(
	const TArray<FVector>& RacingLinePoints,
	const TArray<float>& Curvatures,
	const FMGRacingLineParams& Params)
{
	TArray<float> Speeds;
	Speeds.Reserve(RacingLinePoints.Num());

	for (int32 i = 0; i < RacingLinePoints.Num(); ++i)
	{
		float CornerSpeed = CalculateCornerSpeed(Curvatures[i], Params.MaxLateralG);
		float Speed = FMath::Min(CornerSpeed, Params.MaxSpeed);
		Speeds.Add(Speed);
	}

	return Speeds;
}

void UMGRacingLineGenerator::ApplyBrakingZones(
	TArray<float>& Speeds,
	const TArray<FVector>& Points,
	float BrakingDecel)
{
	// Backward pass - ensure we can brake to reach required speeds
	for (int32 i = Speeds.Num() - 1; i >= 0; --i)
	{
		int32 Next = (i + 1) % Speeds.Num();
		float Distance = FVector::Dist(Points[i], Points[Next]);

		// v^2 = v0^2 + 2*a*d
		// v0 = sqrt(v^2 - 2*a*d)
		float MaxEntrySpeedSq = (Speeds[Next] * Speeds[Next]) + (2.0f * BrakingDecel * Distance);
		float MaxEntrySpeed = FMath::Sqrt(FMath::Max(0.0f, MaxEntrySpeedSq));

		Speeds[i] = FMath::Min(Speeds[i], MaxEntrySpeed);
	}
}

void UMGRacingLineGenerator::ApplyAccelerationZones(
	TArray<float>& Speeds,
	const TArray<FVector>& Points,
	float AccelerationRate)
{
	// Forward pass - ensure we can accelerate to reach speeds
	for (int32 i = 0; i < Speeds.Num(); ++i)
	{
		int32 Prev = (i - 1 + Speeds.Num()) % Speeds.Num();
		float Distance = FVector::Dist(Points[Prev], Points[i]);

		// v^2 = v0^2 + 2*a*d
		float MaxExitSpeedSq = (Speeds[Prev] * Speeds[Prev]) + (2.0f * AccelerationRate * Distance);
		float MaxExitSpeed = FMath::Sqrt(FMath::Max(0.0f, MaxExitSpeedSq));

		Speeds[i] = FMath::Min(Speeds[i], MaxExitSpeed);
	}
}

TArray<int32> UMGRacingLineGenerator::FindApexIndices(
	const TArray<float>& Curvatures,
	float Threshold)
{
	TArray<int32> Apexes;

	for (int32 i = 1; i < Curvatures.Num() - 1; ++i)
	{
		if (Curvatures[i] > Threshold &&
			Curvatures[i] >= Curvatures[i - 1] &&
			Curvatures[i] >= Curvatures[i + 1])
		{
			Apexes.Add(i);
		}
	}

	return Apexes;
}

TArray<int32> UMGRacingLineGenerator::FindBrakingZoneStarts(
	const TArray<float>& Speeds,
	float SpeedDropThreshold)
{
	TArray<int32> BrakingStarts;

	for (int32 i = 1; i < Speeds.Num(); ++i)
	{
		float SpeedDrop = Speeds[i - 1] - Speeds[i];
		if (SpeedDrop > SpeedDropThreshold)
		{
			BrakingStarts.Add(i);
		}
	}

	return BrakingStarts;
}

TArray<FMGAIRacingLinePoint> UMGRacingLineGenerator::BuildRacingLinePoints(
	const TArray<FVector>& Positions,
	const TArray<float>& Speeds,
	const TArray<float>& Curvatures,
	float TrackWidth)
{
	TArray<FMGAIRacingLinePoint> Result;
	Result.Reserve(Positions.Num());

	// Find apexes and braking zones
	TArray<int32> ApexIndices = FindApexIndices(Curvatures, 0.01f);
	TArray<int32> BrakingStarts = FindBrakingZoneStarts(Speeds, 2.0f);

	TSet<int32> ApexSet(ApexIndices);
	TSet<int32> BrakingSet(BrakingStarts);

	float AccumulatedDistance = 0.0f;

	for (int32 i = 0; i < Positions.Num(); ++i)
	{
		FMGAIRacingLinePoint Point;
		Point.Position = Positions[i];
		Point.TargetSpeed = Speeds[i];
		Point.TrackWidth = TrackWidth;
		Point.DistanceAlongTrack = AccumulatedDistance;

		// Calculate direction
		int32 Next = (i + 1) % Positions.Num();
		Point.Direction = (Positions[Next] - Positions[i]).GetSafeNormal();

		// Mark special zones
		Point.bIsApex = ApexSet.Contains(i);
		Point.bIsBrakingZone = BrakingSet.Contains(i);
		Point.bIsAccelerationZone = i > 0 && Speeds[i] > Speeds[i - 1];

		Result.Add(Point);

		// Update accumulated distance
		AccumulatedDistance += FVector::Dist(Positions[i], Positions[Next]);
	}

	return Result;
}

// ==========================================
// MATH UTILITIES
// ==========================================

float UMGRacingLineGenerator::CalculateCurvatureFromPoints(
	const FVector& P0,
	const FVector& P1,
	const FVector& P2) const
{
	// Menger curvature: k = 4*Area / (|P0-P1| * |P1-P2| * |P2-P0|)
	FVector A = P1 - P0;
	FVector B = P2 - P1;
	FVector C = P0 - P2;

	float AreaTimes2 = FVector::CrossProduct(A, B).Size();
	float Denom = A.Size() * B.Size() * C.Size();

	if (Denom < 0.0001f)
	{
		return 0.0f;
	}

	return (2.0f * AreaTimes2) / Denom;
}

FVector UMGRacingLineGenerator::GetPerpendicularVector(const FVector& Direction) const
{
	return FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
}

float UMGRacingLineGenerator::SmoothClamp(float Value, float Min, float Max, float SmoothRange) const
{
	if (Value < Min + SmoothRange)
	{
		float T = (Value - Min) / SmoothRange;
		return Min + SmoothRange * T * T * (3.0f - 2.0f * T);
	}
	else if (Value > Max - SmoothRange)
	{
		float T = (Max - Value) / SmoothRange;
		return Max - SmoothRange * T * T * (3.0f - 2.0f * T);
	}
	return Value;
}

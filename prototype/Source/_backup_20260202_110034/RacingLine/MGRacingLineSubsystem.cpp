// Copyright Midnight Grind. All Rights Reserved.

#include "RacingLine/MGRacingLineSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGRacingLineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default assist settings
	AssistSettings.bShowRacingLine = true;
	AssistSettings.VisualMode = EMGLineVisualMode::Simple;
	AssistSettings.bShowBrakingPoints = true;
	AssistSettings.bShowApexMarkers = true;
	AssistSettings.bShowSpeedAdvisor = true;
	AssistSettings.bShowGearSuggestion = true;
	AssistSettings.LineOpacity = 0.8f;
	AssistSettings.LineWidth = 0.5f;
	AssistSettings.AccelerateColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
	AssistSettings.BrakeColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
	AssistSettings.CoastColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	AssistSettings.LookAheadDistance = 100.0f;
	AssistSettings.FadeDistance = 50.0f;

	bLineLoaded = false;
	bRecording = false;

	LoadLineData();

	// Start line tick
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			LineTickHandle,
			this,
			&UMGRacingLineSubsystem::OnLineTick,
			0.05f,
			true
		);
	}
}

void UMGRacingLineSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LineTickHandle);
	}

	SaveLineData();

	Super::Deinitialize();
}

bool UMGRacingLineSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UMGRacingLineSubsystem::OnLineTick()
{
	if (!bLineLoaded)
	{
		return;
	}

	UpdateVehicleDeviations();

	// Recording logic
	if (bRecording && VehiclePositions.Contains(RecordingVehicle))
	{
		FMGRacingLinePoint NewPoint;
		NewPoint.WorldPosition = VehiclePositions[RecordingVehicle];
		NewPoint.DistanceAlongTrack = RecordedPoints.Num() > 0 ?
			RecordedPoints.Last().DistanceAlongTrack +
			FVector::Dist(RecordedPoints.Last().WorldPosition, NewPoint.WorldPosition) : 0.0f;

		if (VehicleSpeeds.Contains(RecordingVehicle))
		{
			NewPoint.OptimalSpeed = VehicleSpeeds[RecordingVehicle];
		}

		// Only record if we've moved enough
		if (RecordedPoints.Num() == 0 ||
			FVector::Dist(RecordedPoints.Last().WorldPosition, NewPoint.WorldPosition) > 2.0f)
		{
			RecordedPoints.Add(NewPoint);
		}
	}
}

void UMGRacingLineSubsystem::UpdateVehicleDeviations()
{
	for (auto& Pair : VehiclePositions)
	{
		FName VehicleID = Pair.Key;
		FVector Position = Pair.Value;

		// Find nearest point on line
		FMGRacingLinePoint NearestPoint = GetNearestPoint(Position);

		// Calculate deviation
		FMGLineDeviation Deviation;
		Deviation.LateralDeviation = FVector::Dist2D(Position, NearestPoint.WorldPosition);

		if (VehicleSpeeds.Contains(VehicleID))
		{
			float CurrentSpeed = VehicleSpeeds[VehicleID];
			Deviation.SpeedDeviation = CurrentSpeed - NearestPoint.OptimalSpeed;
			Deviation.bTooFast = Deviation.SpeedDeviation > 20.0f;
			Deviation.bTooSlow = Deviation.SpeedDeviation < -30.0f;
		}

		Deviation.bOnLine = Deviation.LateralDeviation < 3.0f;

		// Calculate score (100 = perfect)
		float LateralPenalty = FMath::Min(Deviation.LateralDeviation * 5.0f, 50.0f);
		float SpeedPenalty = FMath::Min(FMath::Abs(Deviation.SpeedDeviation) * 0.5f, 50.0f);
		Deviation.DeviationScore = 100.0f - LateralPenalty - SpeedPenalty;
		Deviation.DeviationScore = FMath::Max(Deviation.DeviationScore, 0.0f);

		VehicleDeviations.Add(VehicleID, Deviation);

		// Update performance tracking
		if (!VehiclePerformances.Contains(VehicleID))
		{
			FMGLinePerformance Perf;
			Perf.VehicleID = VehicleID;
			VehiclePerformances.Add(VehicleID, Perf);
		}

		// Check for corner approach
		if (VehicleDistances.Contains(VehicleID))
		{
			float Distance = VehicleDistances[VehicleID];
			CheckCornerApproach(VehicleID, Distance);
			CheckBrakingZone(VehicleID, Distance);
		}

		OnLineDeviationUpdated.Broadcast(VehicleID, Deviation);
	}
}

void UMGRacingLineSubsystem::CheckCornerApproach(FName VehicleID, float Distance)
{
	FMGCornerData NextCorner = GetNextCorner(Distance);
	float DistanceToCorner = GetDistanceToNextCorner(Distance);

	// Warn when approaching corner (within 100m)
	if (DistanceToCorner > 0 && DistanceToCorner < 100.0f)
	{
		int32 LastWarning = VehicleLastCornerWarning.Contains(VehicleID) ?
			VehicleLastCornerWarning[VehicleID] : -1;

		if (NextCorner.CornerNumber != LastWarning)
		{
			VehicleLastCornerWarning.Add(VehicleID, NextCorner.CornerNumber);
			OnCornerApproaching.Broadcast(VehicleID, NextCorner);
		}
	}
}

void UMGRacingLineSubsystem::CheckBrakingZone(FName VehicleID, float Distance)
{
	FMGBrakingZone NextZone = GetNextBrakingZone(Distance);
	float DistanceToZone = GetDistanceToNextBrakingZone(Distance);

	// Warn when entering braking zone
	if (DistanceToZone >= 0 && DistanceToZone < 10.0f)
	{
		int32 LastWarning = VehicleLastBrakingWarning.Contains(VehicleID) ?
			VehicleLastBrakingWarning[VehicleID] : -1;

		if (NextZone.AssociatedCorner != LastWarning)
		{
			VehicleLastBrakingWarning.Add(VehicleID, NextZone.AssociatedCorner);
			OnBrakingZoneEntered.Broadcast(VehicleID, NextZone);
		}
	}
}

bool UMGRacingLineSubsystem::LoadRacingLine(FName TrackID, EMGRacingLineType LineType)
{
	if (TrackLines.Contains(TrackID))
	{
		const TArray<FMGRacingLine>& Lines = TrackLines[TrackID];
		for (const FMGRacingLine& Line : Lines)
		{
			if (Line.LineType == LineType)
			{
				CurrentLine = Line;
				bLineLoaded = true;
				CalculateBrakingZones();
				IdentifyCorners();
				OnRacingLineLoaded.Broadcast(TrackID, LineType);
				return true;
			}
		}
	}

	// Generate default line if not found
	CurrentLine = GenerateOptimalLine(TrackID, NAME_None);
	CurrentLine.LineType = LineType;
	bLineLoaded = CurrentLine.Points.Num() > 0;

	if (bLineLoaded)
	{
		CalculateBrakingZones();
		IdentifyCorners();
		OnRacingLineLoaded.Broadcast(TrackID, LineType);
	}

	return bLineLoaded;
}

bool UMGRacingLineSubsystem::LoadCustomLine(FName TrackID, const FMGRacingLine& CustomLine)
{
	CurrentLine = CustomLine;
	CurrentLine.TrackID = TrackID;
	CurrentLine.LineType = EMGRacingLineType::Custom;
	bLineLoaded = CurrentLine.Points.Num() > 0;

	if (bLineLoaded)
	{
		CalculateBrakingZones();
		IdentifyCorners();
		OnRacingLineLoaded.Broadcast(TrackID, EMGRacingLineType::Custom);
	}

	return bLineLoaded;
}

void UMGRacingLineSubsystem::UnloadRacingLine()
{
	CurrentLine = FMGRacingLine();
	BrakingZones.Empty();
	bLineLoaded = false;
}

TArray<EMGRacingLineType> UMGRacingLineSubsystem::GetAvailableLineTypes(FName TrackID) const
{
	TArray<EMGRacingLineType> Types;

	if (TrackLines.Contains(TrackID))
	{
		for (const FMGRacingLine& Line : TrackLines[TrackID])
		{
			Types.AddUnique(Line.LineType);
		}
	}

	// Always offer optimal as default
	Types.AddUnique(EMGRacingLineType::Optimal);

	return Types;
}

FMGRacingLinePoint UMGRacingLineSubsystem::GetPointAtDistance(float Distance) const
{
	return InterpolatePoint(Distance);
}

FMGRacingLinePoint UMGRacingLineSubsystem::GetNearestPoint(const FVector& WorldPosition) const
{
	if (CurrentLine.Points.Num() == 0)
	{
		return FMGRacingLinePoint();
	}

	float MinDist = FLT_MAX;
	int32 NearestIndex = 0;

	for (int32 i = 0; i < CurrentLine.Points.Num(); i++)
	{
		float Dist = FVector::DistSquared(WorldPosition, CurrentLine.Points[i].WorldPosition);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			NearestIndex = i;
		}
	}

	return CurrentLine.Points[NearestIndex];
}

float UMGRacingLineSubsystem::GetDistanceAlongLine(const FVector& WorldPosition) const
{
	FMGRacingLinePoint NearestPoint = GetNearestPoint(WorldPosition);
	return NearestPoint.DistanceAlongTrack;
}

FVector UMGRacingLineSubsystem::GetLinePositionAtDistance(float Distance) const
{
	return InterpolatePoint(Distance).WorldPosition;
}

FVector UMGRacingLineSubsystem::GetLineDirectionAtDistance(float Distance) const
{
	return InterpolatePoint(Distance).Direction;
}

float UMGRacingLineSubsystem::GetOptimalSpeedAtDistance(float Distance) const
{
	return InterpolatePoint(Distance).OptimalSpeed;
}

TArray<FMGRacingLinePoint> UMGRacingLineSubsystem::GetPointsInRange(float StartDistance, float EndDistance) const
{
	TArray<FMGRacingLinePoint> Points;

	for (const FMGRacingLinePoint& Point : CurrentLine.Points)
	{
		if (Point.DistanceAlongTrack >= StartDistance && Point.DistanceAlongTrack <= EndDistance)
		{
			Points.Add(Point);
		}
	}

	return Points;
}

TArray<FMGCornerData> UMGRacingLineSubsystem::GetAllCorners() const
{
	return CurrentLine.Corners;
}

FMGCornerData UMGRacingLineSubsystem::GetCorner(int32 CornerNumber) const
{
	for (const FMGCornerData& Corner : CurrentLine.Corners)
	{
		if (Corner.CornerNumber == CornerNumber)
		{
			return Corner;
		}
	}
	return FMGCornerData();
}

FMGCornerData UMGRacingLineSubsystem::GetNextCorner(float CurrentDistance) const
{
	for (const FMGCornerData& Corner : CurrentLine.Corners)
	{
		if (Corner.EntryDistance > CurrentDistance)
		{
			return Corner;
		}
	}

	// Wrap around to first corner
	if (CurrentLine.Corners.Num() > 0)
	{
		return CurrentLine.Corners[0];
	}

	return FMGCornerData();
}

float UMGRacingLineSubsystem::GetDistanceToNextCorner(float CurrentDistance) const
{
	FMGCornerData NextCorner = GetNextCorner(CurrentDistance);

	if (NextCorner.EntryDistance > CurrentDistance)
	{
		return NextCorner.EntryDistance - CurrentDistance;
	}

	// Wrap around calculation
	return (CurrentLine.TotalDistance - CurrentDistance) + NextCorner.EntryDistance;
}

EMGCornerPhase UMGRacingLineSubsystem::GetCornerPhase(float CurrentDistance) const
{
	for (const FMGCornerData& Corner : CurrentLine.Corners)
	{
		if (CurrentDistance >= Corner.EntryDistance && CurrentDistance <= Corner.ExitDistance)
		{
			// Within corner
			float CornerProgress = (CurrentDistance - Corner.EntryDistance) /
								   (Corner.ExitDistance - Corner.EntryDistance);

			if (CornerProgress < 0.2f)
			{
				return EMGCornerPhase::BrakingZone;
			}
			else if (CornerProgress < 0.35f)
			{
				return EMGCornerPhase::TurnIn;
			}
			else if (CornerProgress < 0.55f)
			{
				return EMGCornerPhase::Apex;
			}
			else if (CornerProgress < 0.75f)
			{
				return EMGCornerPhase::Exit;
			}
			else
			{
				return EMGCornerPhase::Acceleration;
			}
		}

		// Check approach
		float DistanceToEntry = Corner.EntryDistance - CurrentDistance;
		if (DistanceToEntry > 0 && DistanceToEntry < 100.0f)
		{
			return EMGCornerPhase::Approach;
		}
	}

	return EMGCornerPhase::Approach;
}

bool UMGRacingLineSubsystem::IsInCorner(float CurrentDistance) const
{
	for (const FMGCornerData& Corner : CurrentLine.Corners)
	{
		if (CurrentDistance >= Corner.EntryDistance && CurrentDistance <= Corner.ExitDistance)
		{
			return true;
		}
	}
	return false;
}

TArray<FMGBrakingZone> UMGRacingLineSubsystem::GetAllBrakingZones() const
{
	return BrakingZones;
}

FMGBrakingZone UMGRacingLineSubsystem::GetNextBrakingZone(float CurrentDistance) const
{
	for (const FMGBrakingZone& Zone : BrakingZones)
	{
		if (Zone.StartDistance > CurrentDistance)
		{
			return Zone;
		}
	}

	// Wrap around
	if (BrakingZones.Num() > 0)
	{
		return BrakingZones[0];
	}

	return FMGBrakingZone();
}

float UMGRacingLineSubsystem::GetDistanceToNextBrakingZone(float CurrentDistance) const
{
	FMGBrakingZone NextZone = GetNextBrakingZone(CurrentDistance);

	if (NextZone.StartDistance > CurrentDistance)
	{
		return NextZone.StartDistance - CurrentDistance;
	}

	return (CurrentLine.TotalDistance - CurrentDistance) + NextZone.StartDistance;
}

bool UMGRacingLineSubsystem::IsInBrakingZone(float CurrentDistance) const
{
	for (const FMGBrakingZone& Zone : BrakingZones)
	{
		if (CurrentDistance >= Zone.StartDistance && CurrentDistance <= Zone.EndDistance)
		{
			return true;
		}
	}
	return false;
}

EMGBrakingIndicator UMGRacingLineSubsystem::GetBrakingIndicator(float CurrentDistance) const
{
	FMGRacingLinePoint Point = InterpolatePoint(CurrentDistance);
	return Point.BrakingLevel;
}

void UMGRacingLineSubsystem::RegisterVehicle(FName VehicleID)
{
	VehiclePositions.Add(VehicleID, FVector::ZeroVector);
	VehicleSpeeds.Add(VehicleID, 0.0f);
	VehicleDistances.Add(VehicleID, 0.0f);
}

void UMGRacingLineSubsystem::UnregisterVehicle(FName VehicleID)
{
	VehiclePositions.Remove(VehicleID);
	VehicleSpeeds.Remove(VehicleID);
	VehicleDistances.Remove(VehicleID);
	VehicleDeviations.Remove(VehicleID);
	VehiclePerformances.Remove(VehicleID);
	VehicleLastCornerWarning.Remove(VehicleID);
	VehicleLastBrakingWarning.Remove(VehicleID);
}

void UMGRacingLineSubsystem::UpdateVehiclePosition(FName VehicleID, const FVector& Position, float CurrentSpeed, float Throttle, float Brake)
{
	VehiclePositions.Add(VehicleID, Position);
	VehicleSpeeds.Add(VehicleID, CurrentSpeed);

	float Distance = GetDistanceAlongLine(Position);
	VehicleDistances.Add(VehicleID, Distance);
}

FMGLineDeviation UMGRacingLineSubsystem::GetVehicleDeviation(FName VehicleID) const
{
	if (VehicleDeviations.Contains(VehicleID))
	{
		return VehicleDeviations[VehicleID];
	}
	return FMGLineDeviation();
}

FMGLinePerformance UMGRacingLineSubsystem::GetVehiclePerformance(FName VehicleID) const
{
	if (VehiclePerformances.Contains(VehicleID))
	{
		return VehiclePerformances[VehicleID];
	}
	return FMGLinePerformance();
}

float UMGRacingLineSubsystem::GetRecommendedSpeed(FName VehicleID) const
{
	if (!VehicleDistances.Contains(VehicleID))
	{
		return 0.0f;
	}

	float Distance = VehicleDistances[VehicleID];
	return GetOptimalSpeedAtDistance(Distance);
}

float UMGRacingLineSubsystem::GetSpeedDifference(FName VehicleID) const
{
	if (!VehicleSpeeds.Contains(VehicleID) || !VehicleDistances.Contains(VehicleID))
	{
		return 0.0f;
	}

	float CurrentSpeed = VehicleSpeeds[VehicleID];
	float RecommendedSpeed = GetRecommendedSpeed(VehicleID);

	return CurrentSpeed - RecommendedSpeed;
}

int32 UMGRacingLineSubsystem::GetRecommendedGear(FName VehicleID) const
{
	if (!VehicleDistances.Contains(VehicleID))
	{
		return 3;
	}

	float Distance = VehicleDistances[VehicleID];
	FMGRacingLinePoint Point = InterpolatePoint(Distance);

	return Point.GearSuggestion > 0 ? Point.GearSuggestion : 3;
}

float UMGRacingLineSubsystem::GetRecommendedThrottle(FName VehicleID) const
{
	if (!VehicleDistances.Contains(VehicleID))
	{
		return 1.0f;
	}

	float Distance = VehicleDistances[VehicleID];
	FMGRacingLinePoint Point = InterpolatePoint(Distance);

	return Point.ThrottlePercent;
}

float UMGRacingLineSubsystem::GetRecommendedBrake(FName VehicleID) const
{
	if (!VehicleDistances.Contains(VehicleID))
	{
		return 0.0f;
	}

	float Distance = VehicleDistances[VehicleID];
	FMGRacingLinePoint Point = InterpolatePoint(Distance);

	return Point.BrakePercent;
}

void UMGRacingLineSubsystem::SetAssistSettings(const FMGDriverAssistSettings& Settings)
{
	AssistSettings = Settings;
	OnAssistSettingsChanged.Broadcast(AssistSettings);
}

void UMGRacingLineSubsystem::SetLineVisibility(bool bVisible)
{
	AssistSettings.bShowRacingLine = bVisible;
	OnAssistSettingsChanged.Broadcast(AssistSettings);
}

void UMGRacingLineSubsystem::SetVisualMode(EMGLineVisualMode Mode)
{
	AssistSettings.VisualMode = Mode;
	OnAssistSettingsChanged.Broadcast(AssistSettings);
}

void UMGRacingLineSubsystem::SetLookAheadDistance(float Distance)
{
	AssistSettings.LookAheadDistance = Distance;
}

FMGRacingLine UMGRacingLineSubsystem::GenerateLineFromSpline(const TArray<FVector>& SplinePoints, FName TrackID)
{
	FMGRacingLine NewLine;
	NewLine.TrackID = TrackID;
	NewLine.LineType = EMGRacingLineType::Custom;
	NewLine.CreatedDate = FDateTime::Now();

	float TotalDist = 0.0f;

	for (int32 i = 0; i < SplinePoints.Num(); i++)
	{
		FMGRacingLinePoint Point;
		Point.WorldPosition = SplinePoints[i];

		if (i > 0)
		{
			TotalDist += FVector::Dist(SplinePoints[i - 1], SplinePoints[i]);
		}
		Point.DistanceAlongTrack = TotalDist;

		// Calculate direction
		if (i < SplinePoints.Num() - 1)
		{
			Point.Direction = (SplinePoints[i + 1] - SplinePoints[i]).GetSafeNormal();
		}
		else if (SplinePoints.Num() > 1)
		{
			Point.Direction = (SplinePoints[i] - SplinePoints[i - 1]).GetSafeNormal();
		}

		// Estimate optimal speed based on curvature
		if (i > 0 && i < SplinePoints.Num() - 1)
		{
			FVector PrevDir = (SplinePoints[i] - SplinePoints[i - 1]).GetSafeNormal();
			FVector NextDir = (SplinePoints[i + 1] - SplinePoints[i]).GetSafeNormal();
			float Curvature = 1.0f - FVector::DotProduct(PrevDir, NextDir);
			Point.Curvature = Curvature;

			// Higher curvature = lower speed
			Point.OptimalSpeed = 200.0f * (1.0f - Curvature * 0.8f);
			Point.OptimalSpeed = FMath::Max(Point.OptimalSpeed, 40.0f);
		}
		else
		{
			Point.OptimalSpeed = 200.0f;
		}

		NewLine.Points.Add(Point);
	}

	NewLine.TotalDistance = TotalDist;

	return NewLine;
}

FMGRacingLine UMGRacingLineSubsystem::GenerateOptimalLine(FName TrackID, FName VehicleClass)
{
	FMGRacingLine NewLine;
	NewLine.TrackID = TrackID;
	NewLine.VehicleClass = VehicleClass;
	NewLine.LineType = EMGRacingLineType::Optimal;
	NewLine.CreatedDate = FDateTime::Now();

	// Generate a default circular track for testing
	const int32 NumPoints = 100;
	const float TrackRadius = 500.0f;

	for (int32 i = 0; i < NumPoints; i++)
	{
		float Angle = (float)i / NumPoints * 2.0f * PI;

		FMGRacingLinePoint Point;
		Point.WorldPosition = FVector(
			FMath::Cos(Angle) * TrackRadius,
			FMath::Sin(Angle) * TrackRadius,
			0.0f
		);

		Point.DistanceAlongTrack = (float)i / NumPoints * 2.0f * PI * TrackRadius;

		// Direction is tangent to circle
		Point.Direction = FVector(
			-FMath::Sin(Angle),
			FMath::Cos(Angle),
			0.0f
		);

		Point.OptimalSpeed = 150.0f;
		Point.Curvature = (TrackRadius > KINDA_SMALL_NUMBER) ? 1.0f / TrackRadius : 0.0f;

		NewLine.Points.Add(Point);
	}

	NewLine.TotalDistance = 2.0f * PI * TrackRadius;

	return NewLine;
}

void UMGRacingLineSubsystem::RecordPlayerLine(FName VehicleID)
{
	bRecording = true;
	RecordingVehicle = VehicleID;
	RecordedPoints.Empty();
}

void UMGRacingLineSubsystem::StopRecordingLine()
{
	bRecording = false;
}

FMGRacingLine UMGRacingLineSubsystem::GetRecordedLine() const
{
	FMGRacingLine RecordedLine;
	RecordedLine.LineType = EMGRacingLineType::Custom;
	RecordedLine.Points = RecordedPoints;

	if (RecordedPoints.Num() > 0)
	{
		RecordedLine.TotalDistance = RecordedPoints.Last().DistanceAlongTrack;
	}

	return RecordedLine;
}

void UMGRacingLineSubsystem::SaveLine(const FMGRacingLine& Line, FName TrackID, FName LineName)
{
	if (!TrackLines.Contains(TrackID))
	{
		TrackLines.Add(TrackID, TArray<FMGRacingLine>());
	}

	TrackLines[TrackID].Add(Line);
}

TArray<FVector> UMGRacingLineSubsystem::GetVisibleLinePoints(const FVector& ViewerPosition, float LookAhead) const
{
	TArray<FVector> VisiblePoints;

	if (!bLineLoaded)
	{
		return VisiblePoints;
	}

	float CurrentDistance = GetDistanceAlongLine(ViewerPosition);
	float EndDistance = CurrentDistance + LookAhead;

	for (const FMGRacingLinePoint& Point : CurrentLine.Points)
	{
		float Dist = Point.DistanceAlongTrack;

		// Handle wrap around
		if (EndDistance > CurrentLine.TotalDistance)
		{
			if (Dist >= CurrentDistance || Dist <= (EndDistance - CurrentLine.TotalDistance))
			{
				VisiblePoints.Add(Point.WorldPosition);
			}
		}
		else if (Dist >= CurrentDistance && Dist <= EndDistance)
		{
			VisiblePoints.Add(Point.WorldPosition);
		}
	}

	return VisiblePoints;
}

FLinearColor UMGRacingLineSubsystem::GetLineColorAtDistance(float Distance) const
{
	FMGRacingLinePoint Point = InterpolatePoint(Distance);

	if (Point.BrakePercent > 0.1f)
	{
		return AssistSettings.BrakeColor;
	}
	else if (Point.ThrottlePercent > 0.9f)
	{
		return AssistSettings.AccelerateColor;
	}
	else
	{
		return AssistSettings.CoastColor;
	}
}

TArray<FVector> UMGRacingLineSubsystem::GetBrakingMarkerPositions() const
{
	TArray<FVector> Positions;

	for (const FMGBrakingZone& Zone : BrakingZones)
	{
		Positions.Add(Zone.StartPosition);
	}

	return Positions;
}

TArray<FVector> UMGRacingLineSubsystem::GetApexMarkerPositions() const
{
	TArray<FVector> Positions;

	for (const FMGCornerData& Corner : CurrentLine.Corners)
	{
		Positions.Add(Corner.ApexPosition);
	}

	return Positions;
}

void UMGRacingLineSubsystem::CalculateBrakingZones()
{
	BrakingZones.Empty();

	if (CurrentLine.Points.Num() < 10)
	{
		return;
	}

	bool bInBrakingZone = false;
	FMGBrakingZone CurrentZone;
	int32 CornerCounter = 0;

	for (int32 i = 1; i < CurrentLine.Points.Num(); i++)
	{
		const FMGRacingLinePoint& Prev = CurrentLine.Points[i - 1];
		const FMGRacingLinePoint& Curr = CurrentLine.Points[i];

		// Detect start of braking zone (speed decrease)
		if (!bInBrakingZone && Curr.OptimalSpeed < Prev.OptimalSpeed - 10.0f)
		{
			bInBrakingZone = true;
			CurrentZone = FMGBrakingZone();
			CurrentZone.StartDistance = Prev.DistanceAlongTrack;
			CurrentZone.StartPosition = Prev.WorldPosition;
			CurrentZone.EntrySpeed = Prev.OptimalSpeed;
			CurrentZone.AssociatedCorner = CornerCounter;
		}

		// Detect end of braking zone (speed stabilizes or increases)
		if (bInBrakingZone && Curr.OptimalSpeed >= Prev.OptimalSpeed)
		{
			CurrentZone.EndDistance = Prev.DistanceAlongTrack;
			CurrentZone.EndPosition = Prev.WorldPosition;
			CurrentZone.ExitSpeed = Prev.OptimalSpeed;
			CurrentZone.BrakingDistance = CurrentZone.EndDistance - CurrentZone.StartDistance;
			CurrentZone.OptimalBrakeForce = (CurrentZone.EntrySpeed - CurrentZone.ExitSpeed) /
											 CurrentZone.BrakingDistance;

			BrakingZones.Add(CurrentZone);
			bInBrakingZone = false;
			CornerCounter++;
		}
	}
}

void UMGRacingLineSubsystem::IdentifyCorners()
{
	CurrentLine.Corners.Empty();

	if (CurrentLine.Points.Num() < 10)
	{
		return;
	}

	bool bInCorner = false;
	FMGCornerData CurrentCorner;
	int32 CornerNumber = 1;
	float MaxCurvature = 0.0f;
	int32 ApexIndex = 0;

	for (int32 i = 1; i < CurrentLine.Points.Num() - 1; i++)
	{
		const FMGRacingLinePoint& Curr = CurrentLine.Points[i];

		// Detect corner start
		if (!bInCorner && Curr.Curvature > 0.01f)
		{
			bInCorner = true;
			CurrentCorner = FMGCornerData();
			CurrentCorner.CornerNumber = CornerNumber;
			CurrentCorner.EntryDistance = Curr.DistanceAlongTrack;
			MaxCurvature = Curr.Curvature;
			ApexIndex = i;
		}

		// Track apex (max curvature)
		if (bInCorner && Curr.Curvature > MaxCurvature)
		{
			MaxCurvature = Curr.Curvature;
			ApexIndex = i;
		}

		// Detect corner end
		if (bInCorner && Curr.Curvature < 0.005f)
		{
			CurrentCorner.ExitDistance = Curr.DistanceAlongTrack;
			CurrentCorner.ApexDistance = CurrentLine.Points[ApexIndex].DistanceAlongTrack;
			CurrentCorner.ApexPosition = CurrentLine.Points[ApexIndex].WorldPosition;
			CurrentCorner.OptimalApexSpeed = CurrentLine.Points[ApexIndex].OptimalSpeed;

			// Determine direction
			if (ApexIndex > 0)
			{
				FVector EntryDir = CurrentLine.Points[ApexIndex - 1].Direction;
				FVector ApexDir = CurrentLine.Points[ApexIndex].Direction;
				float Cross = EntryDir.X * ApexDir.Y - EntryDir.Y * ApexDir.X;
				CurrentCorner.bLeftHander = Cross > 0;
			}

			CurrentCorner.CornerAngle = FMath::RadiansToDegrees(FMath::Acos(1.0f - MaxCurvature * 100.0f));
			CurrentCorner.bHairpin = CurrentCorner.CornerAngle > 90.0f;

			CurrentLine.Corners.Add(CurrentCorner);
			bInCorner = false;
			CornerNumber++;
			MaxCurvature = 0.0f;
		}
	}
}

FMGRacingLinePoint UMGRacingLineSubsystem::InterpolatePoint(float Distance) const
{
	if (CurrentLine.Points.Num() == 0)
	{
		return FMGRacingLinePoint();
	}

	// Handle wrap around
	while (Distance < 0)
	{
		Distance += CurrentLine.TotalDistance;
	}
	while (Distance > CurrentLine.TotalDistance)
	{
		Distance -= CurrentLine.TotalDistance;
	}

	// Find surrounding points
	int32 LowerIndex = 0;
	int32 UpperIndex = 0;

	for (int32 i = 0; i < CurrentLine.Points.Num() - 1; i++)
	{
		if (CurrentLine.Points[i].DistanceAlongTrack <= Distance &&
			CurrentLine.Points[i + 1].DistanceAlongTrack > Distance)
		{
			LowerIndex = i;
			UpperIndex = i + 1;
			break;
		}
	}

	// Handle end of track
	if (UpperIndex >= CurrentLine.Points.Num())
	{
		return CurrentLine.Points.Last();
	}

	const FMGRacingLinePoint& Lower = CurrentLine.Points[LowerIndex];
	const FMGRacingLinePoint& Upper = CurrentLine.Points[UpperIndex];

	float SegmentLength = Upper.DistanceAlongTrack - Lower.DistanceAlongTrack;
	float Alpha = (SegmentLength > 0) ? (Distance - Lower.DistanceAlongTrack) / SegmentLength : 0.0f;

	// Interpolate
	FMGRacingLinePoint Result;
	Result.WorldPosition = FMath::Lerp(Lower.WorldPosition, Upper.WorldPosition, Alpha);
	Result.Direction = FMath::Lerp(Lower.Direction, Upper.Direction, Alpha).GetSafeNormal();
	Result.DistanceAlongTrack = Distance;
	Result.OptimalSpeed = FMath::Lerp(Lower.OptimalSpeed, Upper.OptimalSpeed, Alpha);
	Result.ThrottlePercent = FMath::Lerp(Lower.ThrottlePercent, Upper.ThrottlePercent, Alpha);
	Result.BrakePercent = FMath::Lerp(Lower.BrakePercent, Upper.BrakePercent, Alpha);
	Result.Curvature = FMath::Lerp(Lower.Curvature, Upper.Curvature, Alpha);
	Result.BrakingLevel = Alpha < 0.5f ? Lower.BrakingLevel : Upper.BrakingLevel;
	Result.GearSuggestion = Alpha < 0.5f ? Lower.GearSuggestion : Upper.GearSuggestion;

	return Result;
}

int32 UMGRacingLineSubsystem::FindNearestPointIndex(float Distance) const
{
	if (CurrentLine.Points.Num() == 0)
	{
		return -1;
	}

	for (int32 i = 0; i < CurrentLine.Points.Num() - 1; i++)
	{
		if (CurrentLine.Points[i].DistanceAlongTrack <= Distance &&
			CurrentLine.Points[i + 1].DistanceAlongTrack > Distance)
		{
			return i;
		}
	}

	return CurrentLine.Points.Num() - 1;
}

void UMGRacingLineSubsystem::SaveLineData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("RacingLine");
	IFileManager::Get().MakeDirectory(*SaveDir, true);

	FString FilePath = SaveDir / TEXT("racing_lines.dat");

	FBufferArchive SaveArchive;

	// Save version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save custom track lines
	int32 TrackCount = TrackLines.Num();
	SaveArchive << TrackCount;

	for (auto& TrackPair : TrackLines)
	{
		FString TrackIDStr = TrackPair.Key.ToString();
		SaveArchive << TrackIDStr;

		int32 LineCount = TrackPair.Value.Num();
		SaveArchive << LineCount;

		for (const FMGRacingLine& Line : TrackPair.Value)
		{
			int32 LineType = static_cast<int32>(Line.LineType);
			SaveArchive << LineType;

			FString VehicleClassStr = Line.VehicleClass.ToString();
			SaveArchive << VehicleClassStr;

			SaveArchive << Line.TotalDistance;

			// Save points (limited to avoid huge files)
			int32 PointCount = FMath::Min(Line.Points.Num(), 1000);
			SaveArchive << PointCount;

			for (int32 i = 0; i < PointCount; i++)
			{
				const FMGRacingLinePoint& Point = Line.Points[i];
				SaveArchive << Point.WorldPosition;
				SaveArchive << Point.Direction;
				SaveArchive << Point.DistanceAlongTrack;
				SaveArchive << Point.OptimalSpeed;
				SaveArchive << Point.Curvature;
				SaveArchive << Point.ThrottlePercent;
				SaveArchive << Point.BrakePercent;
				SaveArchive << Point.GearSuggestion;
			}
		}
	}

	// Save performance data
	int32 PerfCount = VehiclePerformances.Num();
	SaveArchive << PerfCount;

	for (auto& PerfPair : VehiclePerformances)
	{
		FString VehicleIDStr = PerfPair.Key.ToString();
		SaveArchive << VehicleIDStr;

		FMGLinePerformance& Perf = PerfPair.Value;
		SaveArchive << Perf.AverageDeviation;
		SaveArchive << Perf.ApexHitPercentage;
		SaveArchive << Perf.BrakingEfficiency;
		SaveArchive << Perf.ConsistencyScore;
		SaveArchive << Perf.TotalCornersTaken;
		SaveArchive << Perf.PerfectApexes;
	}

	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	SaveArchive.FlushCache();
	SaveArchive.Empty();
}

void UMGRacingLineSubsystem::LoadLineData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("RacingLine") / TEXT("racing_lines.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		return;
	}

	// Load custom track lines
	int32 TrackCount;
	LoadArchive << TrackCount;

	for (int32 t = 0; t < TrackCount; t++)
	{
		FString TrackIDStr;
		LoadArchive << TrackIDStr;
		FName TrackID(*TrackIDStr);

		int32 LineCount;
		LoadArchive << LineCount;

		TArray<FMGRacingLine> Lines;

		for (int32 l = 0; l < LineCount; l++)
		{
			FMGRacingLine Line;
			Line.TrackID = TrackID;

			int32 LineType;
			LoadArchive << LineType;
			Line.LineType = static_cast<EMGRacingLineType>(LineType);

			FString VehicleClassStr;
			LoadArchive << VehicleClassStr;
			Line.VehicleClass = FName(*VehicleClassStr);

			LoadArchive << Line.TotalDistance;

			int32 PointCount;
			LoadArchive << PointCount;

			for (int32 p = 0; p < PointCount; p++)
			{
				FMGRacingLinePoint Point;
				LoadArchive << Point.WorldPosition;
				LoadArchive << Point.Direction;
				LoadArchive << Point.DistanceAlongTrack;
				LoadArchive << Point.OptimalSpeed;
				LoadArchive << Point.Curvature;
				LoadArchive << Point.ThrottlePercent;
				LoadArchive << Point.BrakePercent;
				LoadArchive << Point.GearSuggestion;
				Line.Points.Add(Point);
			}

			Lines.Add(Line);
		}

		TrackLines.Add(TrackID, Lines);
	}

	// Load performance data
	int32 PerfCount;
	LoadArchive << PerfCount;

	for (int32 i = 0; i < PerfCount; i++)
	{
		FString VehicleIDStr;
		LoadArchive << VehicleIDStr;
		FName VehicleID(*VehicleIDStr);

		FMGLinePerformance Perf;
		Perf.VehicleID = VehicleID;
		LoadArchive << Perf.AverageDeviation;
		LoadArchive << Perf.ApexHitPercentage;
		LoadArchive << Perf.BrakingEfficiency;
		LoadArchive << Perf.ConsistencyScore;
		LoadArchive << Perf.TotalCornersTaken;
		LoadArchive << Perf.PerfectApexes;

		VehiclePerformances.Add(VehicleID, Perf);
	}

	LoadArchive.FlushCache();
	LoadArchive.Close();
}

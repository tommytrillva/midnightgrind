// Copyright Midnight Grind. All Rights Reserved.

#include "Track/MGTrackSubsystem.h"
#include "Track/MGCheckpointActor.h"
#include "Track/MGRacingLineActor.h"
#include "Track/MGTrackDataAssets.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CollisionQueryParams.h"

void UMGTrackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem initialized"));
}

void UMGTrackSubsystem::Deinitialize()
{
	Checkpoints.Empty();
	RacerProgressMap.Empty();
	RacerActors.Empty();

	Super::Deinitialize();
}

void UMGTrackSubsystem::Tick(float DeltaTime)
{
	// Update race timer
	if (bRaceTimerRunning)
	{
		RaceTime += DeltaTime;
	}

	// Update positions
	UpdatePositions();

	// Update wrong way status for each racer
	for (auto& Pair : RacerActors)
	{
		if (AActor* Actor = Pair.Value.Get())
		{
			UpdateRacerWrongWay(Pair.Key, Actor->GetActorLocation(), Actor->GetVelocity());
		}
	}
}

bool UMGTrackSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

// ==========================================
// TRACK SETUP
// ==========================================

void UMGTrackSubsystem::InitializeTrack(UMGTrackData* TrackData)
{
	// UMGTrackData is an alias for UMGTrackDataAsset
	UMGTrackDataAsset* DataAsset = Cast<UMGTrackDataAsset>(TrackData);
	if (!DataAsset)
	{
		return;
	}

	// Clear existing data
	ClearCheckpoints();
	RacerProgressMap.Empty();

	// Load track configuration from data asset
	TrackConfig.TrackName = DataAsset->TrackID;
	TrackConfig.DisplayName = DataAsset->TrackName;
	TrackConfig.bIsCircuit = DataAsset->bIsCircuit;
	TrackConfig.TrackLength = DataAsset->TrackLength * 100.0f; // Convert meters to cm
	TrackConfig.NumSectors = DataAsset->Sectors.Num();
	TrackConfig.TrackRecordTime = DataAsset->TrackRecord.LapTime;
	TrackConfig.TrackRecordHolder = DataAsset->TrackRecord.PlayerName;

	// Set up sectors from data asset
	for (const FMGTrackSector& Sector : DataAsset->Sectors)
	{
		// Create checkpoint data for sector splits
		if (Sector.EndCheckpointIndex > 0)
		{
			FMGCheckpointData SectorCheckpoint;
			SectorCheckpoint.Index = Sector.EndCheckpointIndex;
			SectorCheckpoint.bIsSectorSplit = true;
			SectorCheckpoint.SectorIndex = Sector.SectorIndex;
			SectorCheckpoint.DistanceFromStart = Sector.Length * 100.0f; // Convert to cm
			// Position will be set when checkpoint actors are found
			RegisterCheckpoint(SectorCheckpoint);
		}
	}

	// Now load the track level to find checkpoint actors
	LoadTrack(DataAsset->TrackID);

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Initialized track '%s' from data asset (Length: %.0fm, Sectors: %d)"),
		*DataAsset->TrackID.ToString(), DataAsset->TrackLength, TrackConfig.NumSectors);
}

void UMGTrackSubsystem::LoadTrack(FName TrackID)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGTrackSubsystem::LoadTrack - No world available"));
		return;
	}

	// Clear existing data
	ClearCheckpoints();
	RacerProgressMap.Empty();

	// Set track name in config
	TrackConfig.TrackName = TrackID;

	// Find all checkpoint actors in the world
	TArray<AActor*> FoundCheckpoints;
	UGameplayStatics::GetAllActorsOfClass(World, AMGCheckpointActor::StaticClass(), FoundCheckpoints);

	if (FoundCheckpoints.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGTrackSubsystem::LoadTrack - No checkpoints found for track '%s'"), *TrackID.ToString());
		return;
	}

	// Register each checkpoint using its data struct
	for (AActor* Actor : FoundCheckpoints)
	{
		if (AMGCheckpointActor* Checkpoint = Cast<AMGCheckpointActor>(Actor))
		{
			FMGCheckpointData Data = Checkpoint->GetCheckpointData();
			RegisterCheckpoint(Data);
		}
	}

	// Calculate distances from start
	float TotalDistance = 0.0f;
	for (int32 i = 0; i < Checkpoints.Num(); ++i)
	{
		Checkpoints[i].DistanceFromStart = TotalDistance;

		if (i < Checkpoints.Num() - 1)
		{
			TotalDistance += FVector::Dist(Checkpoints[i].Position, Checkpoints[i + 1].Position);
		}
	}

	// If circuit, add distance from last checkpoint back to start
	if (TrackConfig.bIsCircuit && Checkpoints.Num() > 1)
	{
		TotalDistance += FVector::Dist(Checkpoints.Last().Position, Checkpoints[0].Position);
	}

	TrackConfig.TrackLength = TotalDistance;

	// Find racing line spline if available
	TArray<AActor*> RacingLines;
	UGameplayStatics::GetAllActorsOfClass(World, AMGRacingLineActor::StaticClass(), RacingLines);
	if (RacingLines.Num() > 0)
	{
		if (AMGRacingLineActor* RacingLine = Cast<AMGRacingLineActor>(RacingLines[0]))
		{
			TrackSpline = RacingLine->GetSplineComponent();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem::LoadTrack - Loaded track '%s' with %d checkpoints (Length: %.0fm)"),
		*TrackID.ToString(), Checkpoints.Num(), TrackConfig.TrackLength / 100.0f);
}

void UMGTrackSubsystem::SetTrackConfig(const FMGTrackConfig& Config)
{
	TrackConfig = Config;

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Track set to '%s' (Length: %.0fm, Sectors: %d)"),
		*Config.TrackName.ToString(), Config.TrackLength, Config.NumSectors);
}

void UMGTrackSubsystem::RegisterCheckpoint(const FMGCheckpointData& Checkpoint)
{
	// Insert in order by index
	int32 InsertIndex = 0;
	for (int32 i = 0; i < Checkpoints.Num(); ++i)
	{
		if (Checkpoints[i].Index > Checkpoint.Index)
		{
			InsertIndex = i;
			break;
		}
		InsertIndex = i + 1;
	}

	Checkpoints.Insert(Checkpoint, InsertIndex);

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Registered checkpoint %d at (%.0f, %.0f, %.0f)"),
		Checkpoint.Index, Checkpoint.Position.X, Checkpoint.Position.Y, Checkpoint.Position.Z);
}

void UMGTrackSubsystem::ClearCheckpoints()
{
	Checkpoints.Empty();
}

bool UMGTrackSubsystem::GetCheckpoint(int32 Index, FMGCheckpointData& OutCheckpoint) const
{
	for (const FMGCheckpointData& CP : Checkpoints)
	{
		if (CP.Index == Index)
		{
			OutCheckpoint = CP;
			return true;
		}
	}
	return false;
}

// ==========================================
// RACER TRACKING
// ==========================================

void UMGTrackSubsystem::RegisterRacer(int32 RacerID, AActor* RacerActor)
{
	FMGRacerProgress Progress;
	Progress.RacerID = RacerID;
	Progress.CurrentLap = 0;
	Progress.LastCheckpoint = -1;
	Progress.TotalCheckpointsPassed = 0;

	// Initialize sector times
	Progress.CurrentSectorTimes.SetNum(TrackConfig.NumSectors);
	Progress.BestSectorTimes.SetNum(TrackConfig.NumSectors);
	for (int32 i = 0; i < TrackConfig.NumSectors; ++i)
	{
		Progress.CurrentSectorTimes[i] = 0.0f;
		Progress.BestSectorTimes[i] = -1.0f; // -1 = no time set
	}

	RacerProgressMap.Add(RacerID, Progress);
	RacerActors.Add(RacerID, RacerActor);

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Registered racer %d"), RacerID);
}

void UMGTrackSubsystem::UnregisterRacer(int32 RacerID)
{
	RacerProgressMap.Remove(RacerID);
	RacerActors.Remove(RacerID);
}

bool UMGTrackSubsystem::GetRacerProgress(int32 RacerID, FMGRacerProgress& OutProgress) const
{
	if (const FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID))
	{
		OutProgress = *Progress;
		return true;
	}
	return false;
}

TArray<FMGRacerProgress> UMGTrackSubsystem::GetAllRacerProgress() const
{
	TArray<FMGRacerProgress> Result;

	// Get all progress
	for (const auto& Pair : RacerProgressMap)
	{
		Result.Add(Pair.Value);
	}

	// Sort by total distance (descending)
	Result.Sort([this](const FMGRacerProgress& A, const FMGRacerProgress& B)
	{
		// Finished racers first
		if (A.bHasFinished != B.bHasFinished)
		{
			return A.bHasFinished;
		}

		// If both finished, sort by finish time
		if (A.bHasFinished && B.bHasFinished)
		{
			return A.FinishTime < B.FinishTime;
		}

		// Otherwise sort by total distance
		return CalculateTotalDistance(A) > CalculateTotalDistance(B);
	});

	return Result;
}

int32 UMGTrackSubsystem::GetRacerPosition(int32 RacerID) const
{
	TArray<FMGRacerProgress> Sorted = GetAllRacerProgress();

	for (int32 i = 0; i < Sorted.Num(); ++i)
	{
		if (Sorted[i].RacerID == RacerID)
		{
			return i + 1; // 1-indexed
		}
	}

	return -1;
}

int32 UMGTrackSubsystem::GetRacerAhead(int32 RacerID) const
{
	int32 Position = GetRacerPosition(RacerID);
	if (Position <= 1)
	{
		return -1; // No one ahead
	}

	TArray<FMGRacerProgress> Sorted = GetAllRacerProgress();
	if (Position - 2 >= 0 && Position - 2 < Sorted.Num())
	{
		return Sorted[Position - 2].RacerID;
	}

	return -1;
}

int32 UMGTrackSubsystem::GetRacerBehind(int32 RacerID) const
{
	int32 Position = GetRacerPosition(RacerID);
	TArray<FMGRacerProgress> Sorted = GetAllRacerProgress();

	if (Position >= Sorted.Num())
	{
		return -1; // No one behind
	}

	if (Position >= 0 && Position < Sorted.Num())
	{
		return Sorted[Position].RacerID;
	}

	return -1;
}

float UMGTrackSubsystem::GetGapToRacer(int32 FromRacerID, int32 ToRacerID) const
{
	const FMGRacerProgress* FromProgress = RacerProgressMap.Find(FromRacerID);
	const FMGRacerProgress* ToProgress = RacerProgressMap.Find(ToRacerID);

	if (!FromProgress || !ToProgress)
	{
		return 0.0f;
	}

	float FromDistance = CalculateTotalDistance(*FromProgress);
	float ToDistance = CalculateTotalDistance(*ToProgress);

	// Positive = behind, Negative = ahead
	return ToDistance - FromDistance;
}

// ==========================================
// CHECKPOINT CROSSING
// ==========================================

void UMGTrackSubsystem::OnCheckpointCrossed(int32 RacerID, int32 CheckpointIndex)
{
	FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID);
	if (!Progress)
	{
		return;
	}

	// Validate checkpoint progression
	if (!IsValidCheckpointProgression(RacerID, CheckpointIndex))
	{
		return;
	}

	// Update progress
	Progress->LastCheckpoint = CheckpointIndex;
	Progress->TotalCheckpointsPassed++;
	Progress->LastCheckpointTime = RaceTime;

	// Check for sector split
	for (const FMGCheckpointData& CP : Checkpoints)
	{
		if (CP.Index == CheckpointIndex && CP.bIsSectorSplit)
		{
			float SectorTime = RaceTime - Progress->LapTimes.Num() > 0 ?
				Progress->LapTimes.Last() : RaceTime;

			// Calculate sector time properly
			if (CP.SectorIndex > 0 && Progress->CurrentSectorTimes.IsValidIndex(CP.SectorIndex - 1))
			{
				for (int32 i = 0; i < CP.SectorIndex; ++i)
				{
					SectorTime -= Progress->CurrentSectorTimes[i];
				}
			}

			Progress->CurrentSectorTimes[CP.SectorIndex] = SectorTime;
			ProcessSectorCompletion(RacerID, CP.SectorIndex, SectorTime);
		}
	}

	OnCheckpointPassed.Broadcast(RacerID, CheckpointIndex);

	UE_LOG(LogTemp, Verbose, TEXT("MGTrackSubsystem: Racer %d passed checkpoint %d"), RacerID, CheckpointIndex);
}

void UMGTrackSubsystem::OnFinishLineCrossed(int32 RacerID)
{
	FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID);
	if (!Progress)
	{
		return;
	}

	// Must have passed all checkpoints
	if (Progress->LastCheckpoint < Checkpoints.Num() - 1)
	{
		return;
	}

	// Process lap completion
	ProcessLapCompletion(RacerID);

	// Reset checkpoint for new lap
	Progress->LastCheckpoint = -1;
}

// ==========================================
// WRONG WAY
// ==========================================

bool UMGTrackSubsystem::IsRacerWrongWay(int32 RacerID) const
{
	if (const FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID))
	{
		return Progress->bWrongWay;
	}
	return false;
}

void UMGTrackSubsystem::UpdateRacerWrongWay(int32 RacerID, FVector Position, FVector Velocity)
{
	FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID);
	if (!Progress)
	{
		return;
	}

	// Need velocity to determine direction
	if (Velocity.SizeSquared() < 100.0f) // Basically stationary
	{
		return;
	}

	// Find next checkpoint
	int32 NextCheckpoint = GetNextCheckpointForRacer(RacerID);
	if (NextCheckpoint < 0 || !Checkpoints.IsValidIndex(NextCheckpoint))
	{
		return;
	}

	const FMGCheckpointData& NextCP = Checkpoints[NextCheckpoint];

	// Calculate direction to next checkpoint
	FVector ToCheckpoint = (NextCP.Position - Position).GetSafeNormal();
	FVector VelocityDir = Velocity.GetSafeNormal();

	// If moving away from next checkpoint, wrong way
	float Dot = FVector::DotProduct(ToCheckpoint, VelocityDir);

	bool bNewWrongWay = Dot < -0.5f; // More than 120 degrees off

	if (bNewWrongWay != Progress->bWrongWay)
	{
		Progress->bWrongWay = bNewWrongWay;
		OnWrongWayChanged.Broadcast(RacerID, bNewWrongWay);
	}
}

// ==========================================
// TIMING
// ==========================================

void UMGTrackSubsystem::StartRaceTimer()
{
	bRaceTimerRunning = true;
	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Race timer started"));
}

void UMGTrackSubsystem::StopRaceTimer()
{
	bRaceTimerRunning = false;
	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Race timer stopped at %.3f"), RaceTime);
}

void UMGTrackSubsystem::ResetRaceTimer()
{
	RaceTime = 0.0f;
	bRaceTimerRunning = false;

	// Reset all racer progress
	for (auto& Pair : RacerProgressMap)
	{
		FMGRacerProgress& Progress = Pair.Value;
		Progress.CurrentLap = 0;
		Progress.LastCheckpoint = -1;
		Progress.TotalCheckpointsPassed = 0;
		Progress.TotalDistance = 0.0f;
		Progress.LapTimes.Empty();
		Progress.bHasFinished = false;
		Progress.FinishTime = 0.0f;

		for (int32 i = 0; i < Progress.CurrentSectorTimes.Num(); ++i)
		{
			Progress.CurrentSectorTimes[i] = 0.0f;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Race timer reset"));
}

// ==========================================
// TRACK QUERY
// ==========================================

float UMGTrackSubsystem::GetDistanceAlongTrack(FVector WorldPosition) const
{
	if (!TrackSpline.IsValid())
	{
		// Fallback: use checkpoint distances
		float ClosestDistance = 0.0f;
		float ClosestDist = FLT_MAX;

		for (const FMGCheckpointData& CP : Checkpoints)
		{
			float Dist = FVector::DistSquared(WorldPosition, CP.Position);
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				ClosestDistance = CP.DistanceFromStart;
			}
		}

		return ClosestDistance;
	}

	// Use spline
	float Key = TrackSpline->FindInputKeyClosestToWorldLocation(WorldPosition);
	return TrackSpline->GetDistanceAlongSplineAtSplineInputKey(Key);
}

FVector UMGTrackSubsystem::GetPositionAtDistance(float Distance) const
{
	if (!TrackSpline.IsValid())
	{
		// Fallback: interpolate between checkpoints
		for (int32 i = 0; i < Checkpoints.Num() - 1; ++i)
		{
			if (Distance >= Checkpoints[i].DistanceFromStart && Distance < Checkpoints[i + 1].DistanceFromStart)
			{
				float Alpha = (Distance - Checkpoints[i].DistanceFromStart) /
					(Checkpoints[i + 1].DistanceFromStart - Checkpoints[i].DistanceFromStart);
				return FMath::Lerp(Checkpoints[i].Position, Checkpoints[i + 1].Position, Alpha);
			}
		}
		return FVector::ZeroVector;
	}

	return TrackSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

FRotator UMGTrackSubsystem::GetDirectionAtDistance(float Distance) const
{
	if (!TrackSpline.IsValid())
	{
		// Fallback: use checkpoint direction
		for (int32 i = 0; i < Checkpoints.Num() - 1; ++i)
		{
			if (Distance >= Checkpoints[i].DistanceFromStart && Distance < Checkpoints[i + 1].DistanceFromStart)
			{
				return Checkpoints[i].Rotation;
			}
		}
		return FRotator::ZeroRotator;
	}

	return TrackSpline->GetRotationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
}

int32 UMGTrackSubsystem::GetNextCheckpointForRacer(int32 RacerID) const
{
	if (const FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID))
	{
		int32 NextCP = Progress->LastCheckpoint + 1;
		if (NextCP >= Checkpoints.Num())
		{
			return 0; // Wrap to start (finish line)
		}
		return NextCP;
	}
	return 0;
}

float UMGTrackSubsystem::GetDistanceToNextCheckpoint(int32 RacerID) const
{
	AActor* const* ActorPtr = nullptr;
	for (const auto& Pair : RacerActors)
	{
		if (Pair.Key == RacerID)
		{
			if (Pair.Value.IsValid())
			{
				int32 NextCP = GetNextCheckpointForRacer(RacerID);
				if (Checkpoints.IsValidIndex(NextCP))
				{
					return FVector::Dist(Pair.Value->GetActorLocation(), Checkpoints[NextCP].Position);
				}
			}
			break;
		}
	}
	return 0.0f;
}

EMGTrackSurface UMGTrackSubsystem::GetSurfaceAtPosition(FVector Position) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return EMGTrackSurface::Asphalt;
	}

	// Perform line trace downward to detect surface
	FVector TraceStart = Position + FVector(0.0f, 0.0f, 100.0f); // Start slightly above
	FVector TraceEnd = Position - FVector(0.0f, 0.0f, 200.0f);   // Trace down

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;
	QueryParams.bTraceComplex = false;

	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		// Check physical material
		if (UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.Get())
		{
			FName MaterialName = PhysMat->GetFName();
			FString MaterialNameStr = MaterialName.ToString().ToLower();

			// Map physical material names to surface types
			if (MaterialNameStr.Contains(TEXT("asphalt")) || MaterialNameStr.Contains(TEXT("road")))
			{
				return EMGTrackSurface::Asphalt;
			}
			else if (MaterialNameStr.Contains(TEXT("concrete")) || MaterialNameStr.Contains(TEXT("cement")))
			{
				return EMGTrackSurface::Concrete;
			}
			else if (MaterialNameStr.Contains(TEXT("cobble")) || MaterialNameStr.Contains(TEXT("brick")))
			{
				return EMGTrackSurface::Cobblestone;
			}
			else if (MaterialNameStr.Contains(TEXT("dirt")) || MaterialNameStr.Contains(TEXT("mud")))
			{
				return EMGTrackSurface::Dirt;
			}
			else if (MaterialNameStr.Contains(TEXT("gravel")) || MaterialNameStr.Contains(TEXT("rock")))
			{
				return EMGTrackSurface::Gravel;
			}
			else if (MaterialNameStr.Contains(TEXT("grass")) || MaterialNameStr.Contains(TEXT("turf")))
			{
				return EMGTrackSurface::Grass;
			}
			else if (MaterialNameStr.Contains(TEXT("water")) || MaterialNameStr.Contains(TEXT("puddle")))
			{
				return EMGTrackSurface::Water;
			}
			else if (MaterialNameStr.Contains(TEXT("ice")) || MaterialNameStr.Contains(TEXT("snow")))
			{
				return EMGTrackSurface::Ice;
			}
			else if (MaterialNameStr.Contains(TEXT("metal")) || MaterialNameStr.Contains(TEXT("steel")))
			{
				return EMGTrackSurface::Metal;
			}

			// Also check surface type enum on physical material if available
			// This uses friction values as fallback heuristic
			float Friction = PhysMat->Friction;
			if (Friction >= 0.9f)
			{
				return EMGTrackSurface::Asphalt; // High friction = grippy surface
			}
			else if (Friction >= 0.7f)
			{
				return EMGTrackSurface::Concrete;
			}
			else if (Friction >= 0.5f)
			{
				return EMGTrackSurface::Dirt;
			}
			else if (Friction >= 0.3f)
			{
				return EMGTrackSurface::Gravel;
			}
			else if (Friction >= 0.1f)
			{
				return EMGTrackSurface::Ice;
			}
			else
			{
				return EMGTrackSurface::Water; // Very low friction
			}
		}

		// No physical material - check component tags as fallback
		if (HitResult.Component.IsValid())
		{
			for (const FName& Tag : HitResult.Component->ComponentTags)
			{
				FString TagStr = Tag.ToString().ToLower();
				if (TagStr.Contains(TEXT("grass")))
				{
					return EMGTrackSurface::Grass;
				}
				else if (TagStr.Contains(TEXT("dirt")))
				{
					return EMGTrackSurface::Dirt;
				}
				else if (TagStr.Contains(TEXT("gravel")))
				{
					return EMGTrackSurface::Gravel;
				}
			}
		}
	}

	// Default to asphalt (most common racing surface)
	return EMGTrackSurface::Asphalt;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGTrackSubsystem::UpdatePositions()
{
	TArray<FMGRacerProgress> Sorted = GetAllRacerProgress();

	// Check for position changes
	for (int32 i = 0; i < Sorted.Num(); ++i)
	{
		int32 NewPosition = i + 1;
		int32 RacerID = Sorted[i].RacerID;

		// Check cached position
		if (CachedPositions.IsValidIndex(RacerID))
		{
			int32 OldPosition = CachedPositions[RacerID];
			if (OldPosition != NewPosition && OldPosition > 0)
			{
				OnPositionChanged.Broadcast(RacerID, OldPosition, NewPosition);
			}
		}
	}

	// Update cache
	CachedPositions.SetNum(FMath::Max(CachedPositions.Num(), Sorted.Num() + 1));
	for (int32 i = 0; i < Sorted.Num(); ++i)
	{
		int32 RacerID = Sorted[i].RacerID;
		if (RacerID >= 0 && RacerID < CachedPositions.Num())
		{
			CachedPositions[RacerID] = i + 1;
		}
	}
}

float UMGTrackSubsystem::CalculateTotalDistance(const FMGRacerProgress& Progress) const
{
	// Total distance = completed laps * track length + progress in current lap
	float LapDistance = Progress.CurrentLap * TrackConfig.TrackLength;

	// Add checkpoint progress
	if (Progress.LastCheckpoint >= 0 && Checkpoints.IsValidIndex(Progress.LastCheckpoint))
	{
		LapDistance += Checkpoints[Progress.LastCheckpoint].DistanceFromStart;
	}

	// Add segment progress
	LapDistance += Progress.DistanceInSegment;

	return LapDistance;
}

int32 UMGTrackSubsystem::GetCheckpointForSector(int32 SectorIndex) const
{
	for (const FMGCheckpointData& CP : Checkpoints)
	{
		if (CP.bIsSectorSplit && CP.SectorIndex == SectorIndex)
		{
			return CP.Index;
		}
	}
	return -1;
}

bool UMGTrackSubsystem::IsValidCheckpointProgression(int32 RacerID, int32 CheckpointIndex) const
{
	const FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID);
	if (!Progress)
	{
		return false;
	}

	// First checkpoint or sequential
	if (Progress->LastCheckpoint < 0)
	{
		return CheckpointIndex == 0;
	}

	// Must be next checkpoint
	int32 Expected = Progress->LastCheckpoint + 1;
	if (TrackConfig.bIsCircuit && Expected >= Checkpoints.Num())
	{
		Expected = 0; // Wrap for circuit
	}

	return CheckpointIndex == Expected;
}

void UMGTrackSubsystem::ProcessLapCompletion(int32 RacerID)
{
	FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID);
	if (!Progress)
	{
		return;
	}

	Progress->CurrentLap++;

	// Calculate lap time
	float LapTime = RaceTime;
	if (Progress->LapTimes.Num() > 0)
	{
		float PreviousLapEnd = 0.0f;
		for (float Time : Progress->LapTimes)
		{
			PreviousLapEnd += Time;
		}
		LapTime = RaceTime - PreviousLapEnd;
	}

	Progress->LapTimes.Add(LapTime);

	// Check for track record
	CheckTrackRecord(LapTime, RacerID);

	OnLapCompleted.Broadcast(RacerID, Progress->CurrentLap, LapTime);

	UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: Racer %d completed lap %d in %.3fs"), RacerID, Progress->CurrentLap, LapTime);

	// Reset sector times for new lap
	for (int32 i = 0; i < Progress->CurrentSectorTimes.Num(); ++i)
	{
		Progress->CurrentSectorTimes[i] = 0.0f;
	}
}

void UMGTrackSubsystem::ProcessSectorCompletion(int32 RacerID, int32 SectorIndex, float SectorTime)
{
	FMGRacerProgress* Progress = RacerProgressMap.Find(RacerID);
	if (!Progress)
	{
		return;
	}

	bool bIsBestSector = false;

	if (Progress->BestSectorTimes.IsValidIndex(SectorIndex))
	{
		if (Progress->BestSectorTimes[SectorIndex] < 0.0f || SectorTime < Progress->BestSectorTimes[SectorIndex])
		{
			Progress->BestSectorTimes[SectorIndex] = SectorTime;
			bIsBestSector = true;
		}
	}

	OnSectorCompleted.Broadcast(RacerID, SectorIndex, SectorTime, bIsBestSector);
}

void UMGTrackSubsystem::CheckTrackRecord(float LapTime, int32 RacerID)
{
	if (TrackConfig.TrackRecordTime <= 0.0f || LapTime < TrackConfig.TrackRecordTime)
	{
		TrackConfig.TrackRecordTime = LapTime;

		// Get racer name (would come from player/AI data)
		FString RecordHolder = FString::Printf(TEXT("Racer_%d"), RacerID);
		TrackConfig.TrackRecordHolder = RecordHolder;

		OnNewTrackRecord.Broadcast(LapTime, RecordHolder);

		UE_LOG(LogTemp, Log, TEXT("MGTrackSubsystem: New track record! %.3fs by %s"), LapTime, *RecordHolder);
	}
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Checkpoint/MGCheckpointSubsystem.h"
#include "TimerManager.h"
#include "Save/MGSaveManagerSubsystem.h"
#include "Save/MGSaveGame.h"

void UMGCheckpointSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	bLayoutLoaded = false;
	bRaceActive = false;
	bRacePaused = false;
	bWasWrongWay = false;
	WrongWayTimer = 0.0f;
	TargetLapTime = 0.0f;

	// Load any saved best times
	LoadCheckpointData();
}

void UMGCheckpointSubsystem::Deinitialize()
{
	// Stop any active race
	if (bRaceActive)
	{
		StopRace();
	}

	// Save best times
	SaveCheckpointData();

	Super::Deinitialize();
}

// ============================================================================
// Layout Management
// ============================================================================

bool UMGCheckpointSubsystem::RegisterLayout(const FMGCheckpointLayout& Layout)
{
	if (Layout.LayoutId.IsEmpty())
	{
		return false;
	}

	if (Layout.Checkpoints.Num() == 0)
	{
		return false;
	}

	RegisteredLayouts.Add(Layout.LayoutId, Layout);
	return true;
}

FMGCheckpointLayout UMGCheckpointSubsystem::GetLayout(const FString& LayoutId) const
{
	if (const FMGCheckpointLayout* Found = RegisteredLayouts.Find(LayoutId))
	{
		return *Found;
	}

	return FMGCheckpointLayout();
}

TArray<FMGCheckpointLayout> UMGCheckpointSubsystem::GetLayoutsForTrack(const FString& TrackId) const
{
	TArray<FMGCheckpointLayout> Result;

	for (const auto& Pair : RegisteredLayouts)
	{
		if (Pair.Value.TrackId == TrackId)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

bool UMGCheckpointSubsystem::LoadLayout(const FString& LayoutId)
{
	const FMGCheckpointLayout* Found = RegisteredLayouts.Find(LayoutId);
	if (!Found)
	{
		return false;
	}

	// Stop any active race first
	if (bRaceActive)
	{
		StopRace();
	}

	ActiveLayout = *Found;
	bLayoutLoaded = true;

	// Reset active state
	ActiveState = FMGActiveCheckpointState();
	ActiveState.LayoutId = LayoutId;

	// Load best times for this layout if available
	if (const FMGBestTimesRecord* BestTimes = BestTimesRecords.Find(LayoutId))
	{
		ActiveState.BestLapTime = BestTimes->BestLapTime;
	}

	return true;
}

void UMGCheckpointSubsystem::UnloadLayout()
{
	if (bRaceActive)
	{
		StopRace();
	}

	ActiveLayout = FMGCheckpointLayout();
	ActiveState = FMGActiveCheckpointState();
	bLayoutLoaded = false;
}

bool UMGCheckpointSubsystem::IsLayoutLoaded() const
{
	return bLayoutLoaded;
}

// ============================================================================
// Race Control
// ============================================================================

void UMGCheckpointSubsystem::StartRace(int32 TotalLaps, float TimeLimit)
{
	if (!bLayoutLoaded)
	{
		return;
	}

	// Reset state
	ActiveState = FMGActiveCheckpointState();
	ActiveState.LayoutId = ActiveLayout.LayoutId;
	ActiveState.CurrentCheckpoint = 0;
	ActiveState.CurrentLap = 1;
	ActiveState.CurrentSector = 0;
	ActiveState.CurrentLapTime = 0.0f;
	ActiveState.CurrentSectorTime = 0.0f;
	ActiveState.TotalRaceTime = 0.0f;
	ActiveState.CheckpointsPassed = 0;
	ActiveState.CheckpointsMissed = 0;
	ActiveState.TotalPoints = 0;

	// Set up time limit if specified
	if (TimeLimit > 0.0f)
	{
		ActiveState.bHasTimeLimit = true;
		ActiveState.TimeRemaining = TimeLimit;
	}
	else
	{
		ActiveState.bHasTimeLimit = false;
		ActiveState.TimeRemaining = 0.0f;
	}

	// Initialize current lap data
	ActiveState.CurrentLapData = FMGLapData();
	ActiveState.CurrentLapData.LapNumber = 1;
	ActiveState.CurrentLapData.bIsValid = true;

	// Load best lap time if available
	if (const FMGBestTimesRecord* BestTimes = BestTimesRecords.Find(ActiveLayout.LayoutId))
	{
		ActiveState.BestLapTime = BestTimes->BestLapTime;
	}

	// Override total laps if specified
	if (TotalLaps > 0)
	{
		ActiveLayout.TotalLaps = TotalLaps;
	}

	bRaceActive = true;
	bRacePaused = false;
	bWasWrongWay = false;
	WrongWayTimer = 0.0f;

	// Start race tick timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGCheckpointSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			RaceTickTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid() && !WeakThis->bRacePaused)
				{
					WeakThis->TickRace(0.016f); // ~60fps tick
				}
			},
			0.016f,
			true
		);
	}
}

void UMGCheckpointSubsystem::StopRace()
{
	if (!bRaceActive)
	{
		return;
	}

	bRaceActive = false;
	bRacePaused = false;

	// Stop tick timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RaceTickTimer);
	}

	// Update best times if valid
	UpdateBestTimes();

	// Broadcast race finished
	OnRaceFinished.Broadcast(ActiveState.TotalRaceTime, ActiveState.TotalPoints);
}

void UMGCheckpointSubsystem::PauseRace()
{
	if (bRaceActive)
	{
		bRacePaused = true;
	}
}

void UMGCheckpointSubsystem::ResumeRace()
{
	if (bRaceActive)
	{
		bRacePaused = false;
	}
}

void UMGCheckpointSubsystem::ResetRace()
{
	bool WasActive = bRaceActive;
	int32 SavedLaps = ActiveLayout.TotalLaps;
	float SavedTimeLimit = ActiveState.bHasTimeLimit ? ActiveState.TimeRemaining : 0.0f;

	if (bRaceActive)
	{
		StopRace();
	}

	if (WasActive || bLayoutLoaded)
	{
		// Keep saved time limit only if it was set
		float TimeLimit = ActiveState.bHasTimeLimit ? SavedTimeLimit : 0.0f;
		StartRace(SavedLaps, TimeLimit);
	}
}

bool UMGCheckpointSubsystem::IsRaceActive() const
{
	return bRaceActive;
}

bool UMGCheckpointSubsystem::IsRacePaused() const
{
	return bRacePaused;
}

// ============================================================================
// Checkpoint Detection
// ============================================================================

bool UMGCheckpointSubsystem::TryPassCheckpoint(FVector PlayerLocation, FVector PlayerVelocity)
{
	if (!bRaceActive || bRacePaused || !bLayoutLoaded)
	{
		return false;
	}

	// Check current expected checkpoint
	int32 ExpectedIndex = ActiveState.CurrentCheckpoint;
	if (ExpectedIndex < 0 || ExpectedIndex >= ActiveLayout.Checkpoints.Num())
	{
		return false;
	}

	const FMGCheckpointDefinition& Checkpoint = ActiveLayout.Checkpoints[ExpectedIndex];

	// Check if player is in trigger
	if (!IsInCheckpointTrigger(PlayerLocation, Checkpoint))
	{
		return false;
	}

	// Check direction if required
	if (Checkpoint.bRequiresDirection)
	{
		if (!IsValidPassageDirection(PlayerVelocity, Checkpoint))
		{
			OnCheckpointInvalid.Broadcast(Checkpoint.CheckpointId, TEXT("Wrong direction"));
			return false;
		}
	}

	// Valid pass
	ProcessCheckpointPass(ExpectedIndex, PlayerVelocity);
	return true;
}

void UMGCheckpointSubsystem::UpdateCheckpointDetection(FVector PlayerLocation, FVector PlayerVelocity, float DeltaTime)
{
	if (!bRaceActive || bRacePaused || !bLayoutLoaded)
	{
		return;
	}

	// Try to pass current checkpoint
	TryPassCheckpoint(PlayerLocation, PlayerVelocity);

	// Check for approaching checkpoint notification
	const FMGCheckpointDefinition& NextCP = GetNextCheckpoint();
	float Distance = FVector::Dist(PlayerLocation, NextCP.Location);

	// Notify when approaching (within 100 meters)
	if (Distance < 10000.0f) // 100m in UE units
	{
		OnApproachingCheckpoint.Broadcast(NextCP.CheckpointId, Distance);
	}

	// Update wrong way detection
	UpdateWrongWayDetection(PlayerVelocity);
}

bool UMGCheckpointSubsystem::IsInCheckpointTrigger(FVector Location, const FMGCheckpointDefinition& Checkpoint) const
{
	switch (Checkpoint.Shape)
	{
		case EMGCheckpointShape::Box:
		{
			// Transform location to checkpoint space
			FVector LocalLocation = Location - Checkpoint.Location;
			LocalLocation = Checkpoint.Rotation.UnrotateVector(LocalLocation);

			// Check if within extents
			return FMath::Abs(LocalLocation.X) <= Checkpoint.Extents.X &&
			       FMath::Abs(LocalLocation.Y) <= Checkpoint.Extents.Y &&
			       FMath::Abs(LocalLocation.Z) <= Checkpoint.Extents.Z;
		}

		case EMGCheckpointShape::Sphere:
		{
			float DistSq = FVector::DistSquared(Location, Checkpoint.Location);
			return DistSq <= FMath::Square(Checkpoint.Radius * 100.0f); // Convert to cm
		}

		case EMGCheckpointShape::Plane:
		{
			// Simplified plane check - check distance from plane and within radius
			FVector ToLocation = Location - Checkpoint.Location;
			FVector Forward = Checkpoint.Rotation.RotateVector(FVector::ForwardVector);

			// Distance from plane
			float PlaneDistance = FMath::Abs(FVector::DotProduct(ToLocation, Forward));

			// Check if close to plane and within radius
			if (PlaneDistance > Checkpoint.Extents.X)
			{
				return false;
			}

			// Project onto plane and check distance
			FVector Projected = ToLocation - Forward * FVector::DotProduct(ToLocation, Forward);
			float ProjectedDist = Projected.Size();

			return ProjectedDist <= Checkpoint.Radius * 100.0f;
		}

		case EMGCheckpointShape::Cylinder:
		{
			FVector ToLocation = Location - Checkpoint.Location;
			FVector Up = Checkpoint.Rotation.RotateVector(FVector::UpVector);

			// Height along cylinder axis
			float HeightAlongAxis = FVector::DotProduct(ToLocation, Up);
			if (FMath::Abs(HeightAlongAxis) > Checkpoint.Extents.Z)
			{
				return false;
			}

			// Distance from axis
			FVector Projected = ToLocation - Up * HeightAlongAxis;
			float DistFromAxis = Projected.Size();

			return DistFromAxis <= Checkpoint.Radius * 100.0f;
		}

		case EMGCheckpointShape::Custom:
		default:
		{
			// Fall back to simple sphere check
			float DistSq = FVector::DistSquared(Location, Checkpoint.Location);
			return DistSq <= FMath::Square(Checkpoint.Radius * 100.0f);
		}
	}
}

bool UMGCheckpointSubsystem::IsValidPassageDirection(FVector Velocity, const FMGCheckpointDefinition& Checkpoint) const
{
	if (Velocity.IsNearlyZero())
	{
		return false;
	}

	FVector VelocityDir = Velocity.GetSafeNormal();
	FVector RequiredDir = Checkpoint.Rotation.RotateVector(Checkpoint.RequiredDirection);

	float DotProduct = FVector::DotProduct(VelocityDir, RequiredDir);
	float AngleDegrees = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(DotProduct, -1.0f, 1.0f)));

	return AngleDegrees <= Checkpoint.DirectionTolerance;
}

// ============================================================================
// State Queries
// ============================================================================

FMGActiveCheckpointState UMGCheckpointSubsystem::GetActiveState() const
{
	return ActiveState;
}

int32 UMGCheckpointSubsystem::GetCurrentCheckpointIndex() const
{
	return ActiveState.CurrentCheckpoint;
}

int32 UMGCheckpointSubsystem::GetCurrentLap() const
{
	return ActiveState.CurrentLap;
}

int32 UMGCheckpointSubsystem::GetCurrentSector() const
{
	return ActiveState.CurrentSector;
}

float UMGCheckpointSubsystem::GetCurrentLapTime() const
{
	return ActiveState.CurrentLapTime;
}

float UMGCheckpointSubsystem::GetTotalRaceTime() const
{
	return ActiveState.TotalRaceTime;
}

float UMGCheckpointSubsystem::GetTimeRemaining() const
{
	return ActiveState.TimeRemaining;
}

int32 UMGCheckpointSubsystem::GetLapsRemaining() const
{
	if (!bLayoutLoaded)
	{
		return 0;
	}

	return FMath::Max(0, ActiveLayout.TotalLaps - ActiveState.CurrentLap + 1);
}

int32 UMGCheckpointSubsystem::GetCheckpointsRemaining() const
{
	if (!bLayoutLoaded)
	{
		return 0;
	}

	// Checkpoints remaining in current lap
	int32 CheckpointsInLap = ActiveLayout.Checkpoints.Num() - ActiveState.CurrentCheckpoint;

	// Add checkpoints for remaining laps
	int32 RemainingLaps = GetLapsRemaining() - 1;
	if (RemainingLaps > 0)
	{
		CheckpointsInLap += RemainingLaps * ActiveLayout.Checkpoints.Num();
	}

	return CheckpointsInLap;
}

// ============================================================================
// Checkpoint Info
// ============================================================================

FMGCheckpointDefinition UMGCheckpointSubsystem::GetCheckpoint(int32 Index) const
{
	if (!bLayoutLoaded || Index < 0 || Index >= ActiveLayout.Checkpoints.Num())
	{
		return FMGCheckpointDefinition();
	}

	return ActiveLayout.Checkpoints[Index];
}

FMGCheckpointDefinition UMGCheckpointSubsystem::GetNextCheckpoint() const
{
	return GetCheckpoint(ActiveState.CurrentCheckpoint);
}

float UMGCheckpointSubsystem::GetDistanceToNextCheckpoint(FVector PlayerLocation) const
{
	if (!bLayoutLoaded || ActiveState.CurrentCheckpoint < 0 ||
	    ActiveState.CurrentCheckpoint >= ActiveLayout.Checkpoints.Num())
	{
		return 0.0f;
	}

	const FMGCheckpointDefinition& NextCP = ActiveLayout.Checkpoints[ActiveState.CurrentCheckpoint];
	return FVector::Dist(PlayerLocation, NextCP.Location) / 100.0f; // Convert to meters
}

FVector UMGCheckpointSubsystem::GetNextCheckpointLocation() const
{
	if (!bLayoutLoaded || ActiveState.CurrentCheckpoint < 0 ||
	    ActiveState.CurrentCheckpoint >= ActiveLayout.Checkpoints.Num())
	{
		return FVector::ZeroVector;
	}

	return ActiveLayout.Checkpoints[ActiveState.CurrentCheckpoint].Location;
}

EMGCheckpointState UMGCheckpointSubsystem::GetCheckpointState(int32 Index) const
{
	if (!bLayoutLoaded || !bRaceActive)
	{
		return EMGCheckpointState::Inactive;
	}

	if (Index < 0 || Index >= ActiveLayout.Checkpoints.Num())
	{
		return EMGCheckpointState::Invalid;
	}

	if (Index < ActiveState.CurrentCheckpoint)
	{
		return EMGCheckpointState::Passed;
	}
	else if (Index == ActiveState.CurrentCheckpoint)
	{
		return EMGCheckpointState::Active;
	}
	else if (Index == ActiveState.CurrentCheckpoint + 1)
	{
		return EMGCheckpointState::Upcoming;
	}

	return EMGCheckpointState::Inactive;
}

// ============================================================================
// Timing
// ============================================================================

float UMGCheckpointSubsystem::GetBestLapTime() const
{
	return ActiveState.BestLapTime;
}

float UMGCheckpointSubsystem::GetBestSectorTime(int32 SectorIndex) const
{
	if (!bLayoutLoaded)
	{
		return 0.0f;
	}

	if (const FMGBestTimesRecord* Record = BestTimesRecords.Find(ActiveLayout.LayoutId))
	{
		if (SectorIndex >= 0 && SectorIndex < Record->BestSectorTimes.Num())
		{
			return Record->BestSectorTimes[SectorIndex];
		}
	}

	return 0.0f;
}

float UMGCheckpointSubsystem::GetCurrentDelta() const
{
	if (!bRaceActive || ActiveState.BestLapTime <= 0.0f)
	{
		return 0.0f;
	}

	// Calculate expected time at current checkpoint based on best lap
	// This is a simplified calculation
	float ProgressPercent = static_cast<float>(ActiveState.CurrentCheckpoint) /
	                        FMath::Max(1, ActiveLayout.Checkpoints.Num());
	float ExpectedTime = ActiveState.BestLapTime * ProgressPercent;

	return ActiveState.CurrentLapTime - ExpectedTime;
}

TArray<float> UMGCheckpointSubsystem::GetCurrentSplitTimes() const
{
	TArray<float> SplitTimes;

	// Collect split times from current lap passages
	for (const FMGCheckpointPassage& Passage : ActiveState.CurrentLapData.Passages)
	{
		SplitTimes.Add(Passage.SplitTime);
	}

	return SplitTimes;
}

FMGLapData UMGCheckpointSubsystem::GetBestLapData() const
{
	FMGLapData BestLap;

	// Find best lap from completed laps
	for (const FMGLapData& Lap : ActiveState.CompletedLaps)
	{
		if (Lap.bIsBestLap || (Lap.bIsValid && (BestLap.LapTime <= 0.0f || Lap.LapTime < BestLap.LapTime)))
		{
			BestLap = Lap;
		}
	}

	return BestLap;
}

// ============================================================================
// Best Times Management
// ============================================================================

void UMGCheckpointSubsystem::SetTargetTimes(const TArray<float>& SplitTimes, float LapTime)
{
	TargetSplitTimes = SplitTimes;
	TargetLapTime = LapTime;
}

FMGBestTimesRecord UMGCheckpointSubsystem::GetBestTimesRecord(const FString& LayoutId) const
{
	if (const FMGBestTimesRecord* Found = BestTimesRecords.Find(LayoutId))
	{
		return *Found;
	}

	return FMGBestTimesRecord();
}

void UMGCheckpointSubsystem::SaveBestTimes(const FString& LayoutId)
{
	// Save specific layout's best time to save game
	if (const FMGBestTimesRecord* Record = BestTimesRecords.Find(LayoutId))
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
			{
				if (UMGSaveGame* SaveData = SaveManager->GetSaveDataMutable())
				{
					FName TrackName = FName(*LayoutId);
					SaveData->TrackBestTimes.Add(TrackName, Record->BestLapTime);
				}
			}
		}
	}
}

// ============================================================================
// Wrong Way Detection
// ============================================================================

bool UMGCheckpointSubsystem::IsGoingWrongWay(FVector PlayerVelocity) const
{
	if (!bRaceActive || !bLayoutLoaded || PlayerVelocity.IsNearlyZero())
	{
		return false;
	}

	// Get current checkpoint
	if (ActiveState.CurrentCheckpoint < 0 || ActiveState.CurrentCheckpoint >= ActiveLayout.Checkpoints.Num())
	{
		return false;
	}

	const FMGCheckpointDefinition& CurrentCP = ActiveLayout.Checkpoints[ActiveState.CurrentCheckpoint];

	// Check if velocity is opposite to required direction
	FVector VelocityDir = PlayerVelocity.GetSafeNormal();
	FVector RequiredDir = CurrentCP.Rotation.RotateVector(CurrentCP.RequiredDirection);

	float DotProduct = FVector::DotProduct(VelocityDir, RequiredDir);

	// Consider wrong way if angle is greater than 120 degrees (dot < -0.5)
	return DotProduct < -0.5f;
}

void UMGCheckpointSubsystem::UpdateWrongWayDetection(FVector PlayerVelocity)
{
	bool CurrentlyWrongWay = IsGoingWrongWay(PlayerVelocity);

	if (CurrentlyWrongWay)
	{
		WrongWayTimer += 0.016f;

		// Only trigger wrong way if going wrong way for more than 1 second
		if (WrongWayTimer > 1.0f && !bWasWrongWay)
		{
			bWasWrongWay = true;
			OnWrongWay.Broadcast(true);
		}
	}
	else
	{
		if (bWasWrongWay)
		{
			bWasWrongWay = false;
			OnWrongWay.Broadcast(false);
		}
		WrongWayTimer = 0.0f;
	}
}

// ============================================================================
// Utility
// ============================================================================

FText UMGCheckpointSubsystem::FormatTime(float TimeSeconds) const
{
	if (TimeSeconds <= 0.0f)
	{
		return FText::FromString(TEXT("--:--.---"));
	}

	int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	float SecondsRemaining = TimeSeconds - (Minutes * 60.0f);
	int32 Seconds = FMath::FloorToInt(SecondsRemaining);
	int32 Milliseconds = FMath::FloorToInt((SecondsRemaining - Seconds) * 1000.0f);

	return FText::FromString(FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds));
}

FText UMGCheckpointSubsystem::FormatDelta(float DeltaSeconds) const
{
	if (FMath::Abs(DeltaSeconds) < 0.001f)
	{
		return FText::FromString(TEXT("+0.000"));
	}

	FString Sign = DeltaSeconds >= 0.0f ? TEXT("+") : TEXT("-");
	float AbsDelta = FMath::Abs(DeltaSeconds);

	int32 Seconds = FMath::FloorToInt(AbsDelta);
	int32 Milliseconds = FMath::FloorToInt((AbsDelta - Seconds) * 1000.0f);

	if (Seconds > 0)
	{
		return FText::FromString(FString::Printf(TEXT("%s%d.%03d"), *Sign, Seconds, Milliseconds));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("%s0.%03d"), *Sign, Milliseconds));
	}
}

FLinearColor UMGCheckpointSubsystem::GetDeltaColor(float DeltaSeconds) const
{
	if (DeltaSeconds < -0.5f)
	{
		// Significantly faster - green
		return FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);
	}
	else if (DeltaSeconds < 0.0f)
	{
		// Slightly faster - light green
		return FLinearColor(0.5f, 1.0f, 0.5f, 1.0f);
	}
	else if (DeltaSeconds < 0.5f)
	{
		// Slightly slower - yellow
		return FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	}
	else
	{
		// Significantly slower - red
		return FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
	}
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGCheckpointSubsystem::SaveCheckpointData()
{
	// Get save manager and update track best times
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			if (UMGSaveGame* SaveData = SaveManager->GetSaveDataMutable())
			{
				// Transfer best times to save game
				for (const auto& Pair : BestTimesRecords)
				{
					FName TrackName = FName(*Pair.Key);
					SaveData->TrackBestTimes.Add(TrackName, Pair.Value.BestLapTime);
				}
			}
		}
	}
}

void UMGCheckpointSubsystem::LoadCheckpointData()
{
	// Get save manager and load track best times
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			if (const UMGSaveGame* SaveData = SaveManager->GetCurrentSaveData())
			{
				// Transfer best times from save game
				for (const auto& Pair : SaveData->TrackBestTimes)
				{
					FString LayoutId = Pair.Key.ToString();
					FMGBestTimesRecord Record;
					Record.BestLapTime = Pair.Value;
					BestTimesRecords.Add(LayoutId, Record);
				}
			}
		}
	}
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGCheckpointSubsystem::TickRace(float DeltaTime)
{
	if (!bRaceActive || bRacePaused)
	{
		return;
	}

	// Update times
	ActiveState.TotalRaceTime += DeltaTime;
	ActiveState.CurrentLapTime += DeltaTime;
	ActiveState.CurrentSectorTime += DeltaTime;

	// Update time remaining
	if (ActiveState.bHasTimeLimit)
	{
		ActiveState.TimeRemaining -= DeltaTime;
		CheckTimeExpired();
	}
}

void UMGCheckpointSubsystem::ProcessCheckpointPass(int32 CheckpointIndex, FVector Velocity)
{
	if (CheckpointIndex < 0 || CheckpointIndex >= ActiveLayout.Checkpoints.Num())
	{
		return;
	}

	const FMGCheckpointDefinition& Checkpoint = ActiveLayout.Checkpoints[CheckpointIndex];

	// Create passage record
	FMGCheckpointPassage Passage;
	Passage.CheckpointId = Checkpoint.CheckpointId;
	Passage.CheckpointIndex = CheckpointIndex;
	Passage.PassageTime = ActiveState.TotalRaceTime;
	Passage.SplitTime = ActiveState.CurrentLapTime;
	Passage.Speed = Velocity.Size() / 100.0f; // Convert to m/s
	Passage.Timestamp = FDateTime::Now();

	// Calculate delta from best/target
	if (TargetSplitTimes.IsValidIndex(CheckpointIndex) && TargetSplitTimes[CheckpointIndex] > 0.0f)
	{
		Passage.DeltaFromTarget = Passage.SplitTime - TargetSplitTimes[CheckpointIndex];
	}

	// Calculate points
	int32 PointsEarned = Checkpoint.BonusPoints;

	// Speed bonus
	if (Checkpoint.SpeedBonusThreshold > 0.0f && Passage.Speed >= Checkpoint.SpeedBonusThreshold)
	{
		PointsEarned += Checkpoint.SpeedBonusPoints;
		Passage.bWasSpeedBonus = true;
	}

	Passage.PointsEarned = PointsEarned;
	ActiveState.TotalPoints += PointsEarned;

	// Add passage to current lap
	ActiveState.CurrentLapData.Passages.Add(Passage);

	// Update checkpoint count
	ActiveState.CheckpointsPassed++;

	// Handle time extension checkpoints
	if (Checkpoint.Type == EMGCheckpointType::TimeExtension && Checkpoint.TimeExtensionSeconds > 0.0f)
	{
		ActiveState.TimeRemaining += Checkpoint.TimeExtensionSeconds;
		OnTimeExtension.Broadcast(Checkpoint.TimeExtensionSeconds, ActiveState.TimeRemaining);
	}

	// Check for sector completion
	int32 CurrentSector = ActiveState.CurrentSector;
	int32 NewSector = GetSectorForCheckpoint(CheckpointIndex + 1);
	if (NewSector != CurrentSector)
	{
		ProcessSectorComplete();
		ActiveState.CurrentSector = NewSector;
	}

	// Advance to next checkpoint
	ActiveState.CurrentCheckpoint = CheckpointIndex + 1;

	// Check for lap completion
	if (ActiveState.CurrentCheckpoint >= ActiveLayout.Checkpoints.Num())
	{
		ProcessLapComplete();
	}
	else
	{
		// Broadcast checkpoint passed
		int32 CheckpointsRemaining = ActiveLayout.Checkpoints.Num() - ActiveState.CurrentCheckpoint;
		OnCheckpointPassed.Broadcast(Passage, CheckpointsRemaining, Passage.DeltaFromTarget);
	}
}

void UMGCheckpointSubsystem::ProcessLapComplete()
{
	// Finalize current lap data
	ActiveState.CurrentLapData.LapTime = ActiveState.CurrentLapTime;

	// Check if this is the best lap
	bool bIsBestLap = false;
	if (ActiveState.CurrentLapData.bIsValid)
	{
		if (ActiveState.BestLapTime <= 0.0f || ActiveState.CurrentLapTime < ActiveState.BestLapTime)
		{
			float OldBest = ActiveState.BestLapTime;
			ActiveState.BestLapTime = ActiveState.CurrentLapTime;
			ActiveState.CurrentLapData.bIsBestLap = true;
			bIsBestLap = true;

			OnNewBestLap.Broadcast(OldBest, ActiveState.BestLapTime);
		}
	}

	// Calculate delta from best
	if (ActiveState.BestLapTime > 0.0f)
	{
		ActiveState.CurrentLapData.DeltaFromBest = ActiveState.CurrentLapTime - ActiveState.BestLapTime;
	}

	// Store completed lap
	ActiveState.CompletedLaps.Add(ActiveState.CurrentLapData);

	// Check if race is finished
	if (ActiveState.CurrentLap >= ActiveLayout.TotalLaps)
	{
		// Race complete
		int32 LapsRemaining = 0;
		OnLapCompleted.Broadcast(ActiveState.CurrentLapData, LapsRemaining, bIsBestLap);
		StopRace();
		return;
	}

	// Start new lap
	int32 LapsRemaining = ActiveLayout.TotalLaps - ActiveState.CurrentLap;
	OnLapCompleted.Broadcast(ActiveState.CurrentLapData, LapsRemaining, bIsBestLap);

	ActiveState.CurrentLap++;
	ActiveState.CurrentCheckpoint = 0;
	ActiveState.CurrentLapTime = 0.0f;
	ActiveState.CurrentSector = 0;
	ActiveState.CurrentSectorTime = 0.0f;

	// Reset current lap data
	ActiveState.CurrentLapData = FMGLapData();
	ActiveState.CurrentLapData.LapNumber = ActiveState.CurrentLap;
	ActiveState.CurrentLapData.bIsValid = true;
}

void UMGCheckpointSubsystem::ProcessSectorComplete()
{
	// Store sector time
	ActiveState.CurrentLapData.SectorTimes.Add(ActiveState.CurrentSectorTime);

	// Check for best sector
	float BestSector = GetBestSectorTime(ActiveState.CurrentSector);
	if (BestSector <= 0.0f || ActiveState.CurrentSectorTime < BestSector)
	{
		OnNewBestSector.Broadcast(ActiveState.CurrentSector, ActiveState.CurrentSectorTime);
	}

	OnSectorCompleted.Broadcast(ActiveState.CurrentSector, ActiveState.CurrentSectorTime);

	// Reset sector time
	ActiveState.CurrentSectorTime = 0.0f;
}

void UMGCheckpointSubsystem::CheckTimeExpired()
{
	if (ActiveState.bHasTimeLimit && ActiveState.TimeRemaining <= 0.0f)
	{
		ActiveState.TimeRemaining = 0.0f;
		OnTimeExpired.Broadcast();
		StopRace();
	}
}

void UMGCheckpointSubsystem::UpdateBestTimes()
{
	if (!bLayoutLoaded)
	{
		return;
	}

	FString LayoutId = ActiveLayout.LayoutId;
	FMGBestTimesRecord* Record = BestTimesRecords.Find(LayoutId);

	if (!Record)
	{
		// Create new record
		FMGBestTimesRecord NewRecord;
		NewRecord.LayoutId = LayoutId;
		BestTimesRecords.Add(LayoutId, NewRecord);
		Record = BestTimesRecords.Find(LayoutId);
	}

	// Update best lap time
	if (ActiveState.BestLapTime > 0.0f &&
	    (Record->BestLapTime <= 0.0f || ActiveState.BestLapTime < Record->BestLapTime))
	{
		Record->BestLapTime = ActiveState.BestLapTime;
	}

	// Update best race time if we finished
	if (ActiveState.CurrentLap > ActiveLayout.TotalLaps &&
	    (Record->BestRaceTime <= 0.0f || ActiveState.TotalRaceTime < Record->BestRaceTime))
	{
		Record->BestRaceTime = ActiveState.TotalRaceTime;
	}

	// Update best sector times from best lap
	FMGLapData BestLap = GetBestLapData();
	if (BestLap.bIsValid && BestLap.SectorTimes.Num() > 0)
	{
		if (Record->BestSectorTimes.Num() < BestLap.SectorTimes.Num())
		{
			Record->BestSectorTimes.SetNum(BestLap.SectorTimes.Num());
		}

		for (int32 i = 0; i < BestLap.SectorTimes.Num(); i++)
		{
			if (Record->BestSectorTimes[i] <= 0.0f || BestLap.SectorTimes[i] < Record->BestSectorTimes[i])
			{
				Record->BestSectorTimes[i] = BestLap.SectorTimes[i];
			}
		}
	}

	// Update split times
	if (BestLap.bIsValid && BestLap.Passages.Num() > 0)
	{
		if (Record->BestSplitTimes.Num() < BestLap.Passages.Num())
		{
			Record->BestSplitTimes.SetNum(BestLap.Passages.Num());
		}

		for (int32 i = 0; i < BestLap.Passages.Num(); i++)
		{
			if (Record->BestSplitTimes[i] <= 0.0f || BestLap.Passages[i].SplitTime < Record->BestSplitTimes[i])
			{
				Record->BestSplitTimes[i] = BestLap.Passages[i].SplitTime;
			}
		}
	}

	Record->RecordDate = FDateTime::Now();
}

int32 UMGCheckpointSubsystem::GetSectorForCheckpoint(int32 CheckpointIndex) const
{
	if (!bLayoutLoaded || ActiveLayout.Sectors.Num() == 0)
	{
		return 0;
	}

	for (int32 i = 0; i < ActiveLayout.Sectors.Num(); i++)
	{
		const FMGSectorDefinition& Sector = ActiveLayout.Sectors[i];
		if (CheckpointIndex >= Sector.StartCheckpointIndex &&
		    CheckpointIndex <= Sector.EndCheckpointIndex)
		{
			return i;
		}
	}

	return 0;
}

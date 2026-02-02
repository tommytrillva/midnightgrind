// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/RaceTypes/MGDragRaceHandler.h"
#include "GameModes/MGRaceGameMode.h"
#include "Vehicle/MGVehiclePawn.h"

UMGDragRaceHandler::UMGDragRaceHandler()
{
	// Quarter mile by default
	TrackDistanceFeet = 1320.0f;
	bUseProTree = false;

	// Reaction thresholds (NHRA-style)
	PerfectReactionThreshold = 0.02f;
	GreatReactionThreshold = 0.05f;
	GoodReactionThreshold = 0.1f;

	// Standard interval distances
	IntervalDistances = { 60.0f, 330.0f, 660.0f, 1000.0f, 1320.0f };
}

void UMGDragRaceHandler::Initialize(AMGRaceGameMode* InGameMode)
{
	Super::Initialize(InGameMode);

	TreeState = EMGLaunchState::PreStage;
}

void UMGDragRaceHandler::Reset()
{
	Super::Reset();

	TreeState = EMGLaunchState::PreStage;
	TreeTimer = 0.0f;
	GreenLightTime = 0.0f;
	CurrentAmberLight = 0;
	RacerDragData.Empty();
}

void UMGDragRaceHandler::OnCountdownStarted()
{
	Super::OnCountdownStarted();

	// Initialize racer data
	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		TArray<FMGRacerData> Racers = GM->GetAllRacers();
		for (const FMGRacerData& Racer : Racers)
		{
			FMGDragRacerData DragData;
			DragData.LaunchState = EMGLaunchState::PreStage;
			RacerDragData.Add(Racer.RacerIndex, DragData);
		}
	}

	// Start Christmas tree when countdown begins
	StartChristmasTree();
}

void UMGDragRaceHandler::OnRaceStarted()
{
	Super::OnRaceStarted();

	// Green light!
	TreeState = EMGLaunchState::Green;

	AMGRaceGameMode* GM = GetGameMode();
	if (GM)
	{
		GreenLightTime = GM->GetRaceTime();
	}

	// Update all racers
	for (auto& Pair : RacerDragData)
	{
		if (Pair.Value.LaunchState != EMGLaunchState::RedLight)
		{
			Pair.Value.LaunchState = EMGLaunchState::Green;
			OnLaunchStateChanged.Broadcast(Pair.Key, EMGLaunchState::Green);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Drag Race: GREEN LIGHT!"));
}

void UMGDragRaceHandler::OnRaceTick(float DeltaTime)
{
	Super::OnRaceTick(DeltaTime);

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	// Update racer progress
	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	for (const FMGRacerData& Racer : Racers)
	{
		if (!Racer.bFinished && !Racer.bDNF)
		{
			UpdateRacerProgress(Racer.RacerIndex, DeltaTime);
		}
	}
}

void UMGDragRaceHandler::StartChristmasTree()
{
	TreeState = EMGLaunchState::Staged;
	TreeTimer = 0.0f;
	CurrentAmberLight = 0;

	UE_LOG(LogTemp, Log, TEXT("Drag Race: Christmas tree started (%s tree)"),
		bUseProTree ? TEXT("Pro") : TEXT("Sportsman"));
}

void UMGDragRaceHandler::OnRacerStaged(int32 RacerIndex, bool bFullyStaged)
{
	FMGDragRacerData* DragData = RacerDragData.Find(RacerIndex);
	if (DragData)
	{
		DragData->LaunchState = bFullyStaged ? EMGLaunchState::FullyStaged : EMGLaunchState::Staged;
		OnLaunchStateChanged.Broadcast(RacerIndex, DragData->LaunchState);
	}
}

float UMGDragRaceHandler::GetTimeToGreen() const
{
	if (TreeState != EMGLaunchState::TreeDropping)
	{
		return 0.0f;
	}

	float Interval = GetTreeInterval();
	int32 LightsRemaining = bUseProTree ? 1 : (3 - CurrentAmberLight);
	float TimeRemaining = (LightsRemaining * Interval) - TreeTimer;

	return FMath::Max(0.0f, TimeRemaining);
}

void UMGDragRaceHandler::OnLaunchInput(int32 RacerIndex)
{
	FMGDragRacerData* DragData = RacerDragData.Find(RacerIndex);
	if (!DragData)
	{
		return;
	}

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	// Check if jumped start
	if (TreeState != EMGLaunchState::Green)
	{
		// Red light!
		DragData->bRedLight = true;
		DragData->LaunchState = EMGLaunchState::RedLight;
		DragData->ReactionTime = -1.0f; // Negative indicates red light

		OnRedLight.Broadcast(RacerIndex);
		OnLaunchStateChanged.Broadcast(RacerIndex, EMGLaunchState::RedLight);

		UE_LOG(LogTemp, Log, TEXT("Drag Race: Racer %d RED LIGHT!"), RacerIndex);
		return;
	}

	// Calculate reaction time (time from green to launch)
	if (DragData->ReactionTime == 0.0f)
	{
		float CurrentTime = GM->GetRaceTime();
		DragData->ReactionTime = CurrentTime - GreenLightTime;
		DragData->LaunchQuality = GetLaunchQuality(DragData->ReactionTime);

		OnReactionTimeRecorded.Broadcast(RacerIndex, DragData->ReactionTime, DragData->LaunchQuality);

		UE_LOG(LogTemp, Log, TEXT("Drag Race: Racer %d reaction time %.4f (%s)"),
			RacerIndex, DragData->ReactionTime,
			*StaticEnum<EMGLaunchQuality>()->GetNameStringByValue(static_cast<int64>(DragData->LaunchQuality)));
	}
}

FMGDragRacerData UMGDragRaceHandler::GetRacerDragData(int32 RacerIndex) const
{
	const FMGDragRacerData* Data = RacerDragData.Find(RacerIndex);
	return Data ? *Data : FMGDragRacerData();
}

float UMGDragRaceHandler::GetReactionTime(int32 RacerIndex) const
{
	const FMGDragRacerData* Data = RacerDragData.Find(RacerIndex);
	return Data ? Data->ReactionTime : 0.0f;
}

float UMGDragRaceHandler::GetElapsedTime(int32 RacerIndex) const
{
	const FMGDragRacerData* Data = RacerDragData.Find(RacerIndex);
	if (Data && Data->bFinished)
	{
		return Data->QuarterMileTime;
	}

	// Return current elapsed time
	AMGRaceGameMode* GM = GetGameMode();
	if (GM && Data)
	{
		return GM->GetRaceTime() - GreenLightTime;
	}

	return 0.0f;
}

float UMGDragRaceHandler::GetCurrentDistance(int32 RacerIndex) const
{
	const FMGDragRacerData* Data = RacerDragData.Find(RacerIndex);
	return Data ? Data->CurrentDistance : 0.0f;
}

void UMGDragRaceHandler::OnShift(int32 RacerIndex, float RPMPercent)
{
	FMGDragRacerData* DragData = RacerDragData.Find(RacerIndex);
	if (!DragData)
	{
		return;
	}

	DragData->ShiftCount++;

	// Determine shift quality based on RPM
	EMGShiftQuality Quality;
	if (RPMPercent >= 0.98f && RPMPercent <= 1.0f)
	{
		Quality = EMGShiftQuality::Perfect;
		DragData->PerfectShifts++;
	}
	else if (RPMPercent >= 0.95f)
	{
		Quality = EMGShiftQuality::Good;
	}
	else if (RPMPercent >= 0.85f)
	{
		Quality = EMGShiftQuality::Early;
	}
	else if (RPMPercent < 0.85f)
	{
		Quality = EMGShiftQuality::Missed;
	}
	else
	{
		Quality = EMGShiftQuality::Late; // Over redline
	}

	OnShiftPerformed.Broadcast(RacerIndex, Quality);
}

EMGRaceCompletionResult UMGDragRaceHandler::CheckCompletionCondition(int32 RacerIndex)
{
	const FMGDragRacerData* DragData = RacerDragData.Find(RacerIndex);
	if (!DragData)
	{
		return EMGRaceCompletionResult::InProgress;
	}

	// Red light = disqualified
	if (DragData->bRedLight)
	{
		return EMGRaceCompletionResult::Disqualified;
	}

	// Crossed finish line
	if (DragData->bFinished)
	{
		return EMGRaceCompletionResult::Finished;
	}

	return EMGRaceCompletionResult::InProgress;
}

void UMGDragRaceHandler::CalculatePositions(TArray<int32>& OutPositions)
{
	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	TArray<FMGRacerData> Racers = GM->GetAllRacers();
	OutPositions.SetNum(Racers.Num());

	// Sort by: 1) Finished (by time), 2) Distance, 3) Red lights last
	TArray<TPair<int32, float>> RacerRanking;
	for (const FMGRacerData& Racer : Racers)
	{
		const FMGDragRacerData* DragData = RacerDragData.Find(Racer.RacerIndex);
		float Score;

		if (DragData)
		{
			if (DragData->bRedLight)
			{
				// Red light - last place
				Score = -1000000.0f;
			}
			else if (DragData->bFinished)
			{
				// Finished - sort by ET (lower is better, so invert)
				Score = 1000000.0f - DragData->QuarterMileTime;
			}
			else
			{
				// In progress - sort by distance
				Score = DragData->CurrentDistance;
			}
		}
		else
		{
			Score = 0.0f;
		}

		RacerRanking.Add(TPair<int32, float>(Racer.RacerIndex, Score));
	}

	RacerRanking.Sort([](const TPair<int32, float>& A, const TPair<int32, float>& B)
	{
		return A.Value > B.Value;
	});

	for (int32 i = 0; i < RacerRanking.Num(); ++i)
	{
		OutPositions[RacerRanking[i].Key] = i + 1;
	}
}

FText UMGDragRaceHandler::GetDisplayName() const
{
	return NSLOCTEXT("RaceType", "DragName", "Drag Race");
}

FText UMGDragRaceHandler::GetDescription() const
{
	return NSLOCTEXT("RaceType", "DragDesc", "Quarter-mile straight-line racing. Perfect your launch and shifts!");
}

FText UMGDragRaceHandler::GetProgressFormat() const
{
	return NSLOCTEXT("RaceType", "DragProgress", "{0} ft / {1} ft");
}

int64 UMGDragRaceHandler::CalculateCreditsForPosition(int32 Position, int32 TotalRacers) const
{
	int64 BaseCredits = Super::CalculateCreditsForPosition(Position, TotalRacers);

	// Drag races are quick - slightly reduced base but bonuses for good reaction
	return static_cast<int64>(BaseCredits * 0.7f);
}

void UMGDragRaceHandler::UpdateRacerProgress(int32 RacerIndex, float DeltaTime)
{
	FMGDragRacerData* DragData = RacerDragData.Find(RacerIndex);
	if (!DragData || DragData->bFinished || DragData->bRedLight)
	{
		return;
	}

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	FMGRacerData RacerData = GM->GetRacerData(RacerIndex);
	if (!RacerData.Vehicle.IsValid())
	{
		return;
	}

	AMGVehiclePawn* Vehicle = RacerData.Vehicle.Get();

	// Get distance traveled from start line
	// In a real implementation, this would measure from the staging position
	float OldDistance = DragData->CurrentDistance;

	// Calculate new distance (simplified - assumes straight line from start)
	FVector Velocity = Vehicle->GetVelocity();
	float DistanceDelta = CmToFeet(Velocity.Size() * DeltaTime);
	DragData->CurrentDistance += DistanceDelta;

	// Check interval crossings
	CheckIntervals(RacerIndex, OldDistance, DragData->CurrentDistance);

	// Check finish
	if (DragData->CurrentDistance >= TrackDistanceFeet && !DragData->bFinished)
	{
		DragData->bFinished = true;
		DragData->QuarterMileTime = GM->GetRaceTime() - GreenLightTime;
		DragData->TrapSpeed = CmsToMph(Velocity.Size());

		UE_LOG(LogTemp, Log, TEXT("Drag Race: Racer %d finished! ET: %.3f, Trap: %.1f mph"),
			RacerIndex, DragData->QuarterMileTime, DragData->TrapSpeed);
	}
}

void UMGDragRaceHandler::CheckIntervals(int32 RacerIndex, float OldDistance, float NewDistance)
{
	FMGDragRacerData* DragData = RacerDragData.Find(RacerIndex);
	if (!DragData)
	{
		return;
	}

	AMGRaceGameMode* GM = GetGameMode();
	if (!GM)
	{
		return;
	}

	FMGRacerData RacerData = GM->GetRacerData(RacerIndex);
	float CurrentTime = GM->GetRaceTime() - GreenLightTime;
	float CurrentSpeed = 0.0f;

	if (RacerData.Vehicle.IsValid())
	{
		CurrentSpeed = CmsToMph(RacerData.Vehicle->GetVelocity().Size());
	}

	// Check each interval
	for (float IntervalDist : IntervalDistances)
	{
		if (OldDistance < IntervalDist && NewDistance >= IntervalDist)
		{
			// Crossed this interval
			OnIntervalTime.Broadcast(RacerIndex, IntervalDist, CurrentTime);

			// Store specific interval times
			if (FMath::IsNearlyEqual(IntervalDist, 60.0f, 1.0f))
			{
				DragData->SixtyFootTime = CurrentTime;
			}
			else if (FMath::IsNearlyEqual(IntervalDist, 330.0f, 1.0f))
			{
				DragData->ThreeThirtyTime = CurrentTime;
			}
			else if (FMath::IsNearlyEqual(IntervalDist, 660.0f, 1.0f))
			{
				DragData->EighthMileTime = CurrentTime;
				DragData->EighthMileSpeed = CurrentSpeed;
			}
			else if (FMath::IsNearlyEqual(IntervalDist, 1000.0f, 1.0f))
			{
				DragData->ThousandFootTime = CurrentTime;
			}

			UE_LOG(LogTemp, Log, TEXT("Drag Race: Racer %d at %.0f ft: %.3fs (%.1f mph)"),
				RacerIndex, IntervalDist, CurrentTime, CurrentSpeed);
		}
	}
}

EMGLaunchQuality UMGDragRaceHandler::GetLaunchQuality(float ReactionTime) const
{
	if (ReactionTime < 0.0f)
	{
		return EMGLaunchQuality::RedLight;
	}
	else if (ReactionTime <= 0.001f)
	{
		return EMGLaunchQuality::Holeshot;
	}
	else if (ReactionTime <= PerfectReactionThreshold)
	{
		return EMGLaunchQuality::Perfect;
	}
	else if (ReactionTime <= GreatReactionThreshold)
	{
		return EMGLaunchQuality::Great;
	}
	else if (ReactionTime <= GoodReactionThreshold)
	{
		return EMGLaunchQuality::Good;
	}
	else if (ReactionTime <= 0.2f)
	{
		return EMGLaunchQuality::Average;
	}
	else
	{
		return EMGLaunchQuality::Poor;
	}
}

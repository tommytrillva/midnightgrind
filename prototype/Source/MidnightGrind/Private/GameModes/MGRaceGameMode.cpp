// Copyright Midnight Grind. All Rights Reserved.
// Updated Stage 52: MVP Game Entry Points - Player Controller Notification

#include "GameModes/MGRaceGameMode.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Core/MGPlayerController.h"
#include "UI/MGRaceHUDSubsystem.h"
#include "Track/MGCheckpoint.h"
#include "GameModes/MGRaceFlowManager.h"
#include "Fuel/MGFuelSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"

AMGRaceGameMode::AMGRaceGameMode()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set default pawn class to our vehicle
	DefaultPawnClass = AMGVehiclePawn::StaticClass();

	// Default race config
	RaceConfig.RaceType = EMGRaceType::Circuit;
	RaceConfig.LapCount = 3;
	RaceConfig.TimeOfDay = 0.0f; // MIDNIGHT!
}

void AMGRaceGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// Parse race options from URL if provided
	// e.g., ?laps=5&type=sprint&pinkslip=true
	if (!Options.IsEmpty())
	{
		FString LapsStr = UGameplayStatics::ParseOption(Options, TEXT("laps"));
		if (!LapsStr.IsEmpty())
		{
			RaceConfig.LapCount = FCString::Atoi(*LapsStr);
		}

		FString TypeStr = UGameplayStatics::ParseOption(Options, TEXT("type"));
		if (TypeStr.Equals(TEXT("sprint"), ESearchCase::IgnoreCase))
		{
			RaceConfig.RaceType = EMGRaceType::Sprint;
		}
		else if (TypeStr.Equals(TEXT("drift"), ESearchCase::IgnoreCase))
		{
			RaceConfig.RaceType = EMGRaceType::Drift;
		}

		if (UGameplayStatics::HasOption(Options, TEXT("pinkslip")))
		{
			RaceConfig.bPinkSlipRace = true;
			RaceConfig.RaceType = EMGRaceType::PinkSlip;
		}
	}
}

void AMGRaceGameMode::StartPlay()
{
	Super::StartPlay();

	// Initialize in pre-race state
	SetRaceState(EMGRaceState::PreRace);
}

void AMGRaceGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Only server controls race state
	if (!HasAuthority())
	{
		return;
	}

	// Early exit for idle states - disable tick when not needed
	if (CurrentRaceState == EMGRaceState::PreRace || CurrentRaceState == EMGRaceState::Finished)
	{
		return;
	}

	switch (CurrentRaceState)
	{
	case EMGRaceState::Countdown:
		UpdateCountdown(DeltaTime);
		break;

	case EMGRaceState::Racing:
		UpdateRaceTiming(DeltaTime);

		// Update positions periodically
		PositionUpdateAccumulator += DeltaTime;
		if (PositionUpdateAccumulator >= PositionUpdateRate)
		{
			UpdatePositions();
			PositionUpdateAccumulator = 0.0f;
		}

		CheckRaceComplete();
		break;

	default:
		break;
	}
}

void AMGRaceGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// Register player's pawn as a racer
	if (NewPlayer && NewPlayer->GetPawn())
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(NewPlayer->GetPawn()))
		{
			PlayerRacerIndex = RegisterRacer(Vehicle, false, FText::FromString(TEXT("Player")));
		}
	}
}

// ==========================================
// RACE CONFIGURATION
// ==========================================

void AMGRaceGameMode::SetRaceConfig(const FMGRaceConfig& Config)
{
	RaceConfig = Config;
}

// ==========================================
// RACE CONTROL
// ==========================================

void AMGRaceGameMode::StartCountdown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentRaceState != EMGRaceState::PreRace)
	{
		return;
	}

	// Freeze all vehicles
	FreezeAllVehicles(true);

	// Initialize countdown
	CountdownSeconds = CountdownDuration;
	CountdownAccumulator = 0.0f;

	SetRaceState(EMGRaceState::Countdown);

	// Broadcast first tick
	OnCountdownTick.Broadcast(CountdownSeconds);
}

void AMGRaceGameMode::AbortCountdown()
{
	if (CurrentRaceState == EMGRaceState::Countdown)
	{
		SetRaceState(EMGRaceState::PreRace);
		FreezeAllVehicles(false);
	}
}

void AMGRaceGameMode::PauseRace()
{
	if (CurrentRaceState == EMGRaceState::Racing)
	{
		SetRaceState(EMGRaceState::Paused);
		UGameplayStatics::SetGamePaused(GetWorld(), true);
	}
}

void AMGRaceGameMode::ResumeRace()
{
	if (CurrentRaceState == EMGRaceState::Paused)
	{
		SetRaceState(EMGRaceState::Racing);
		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}
}

void AMGRaceGameMode::EndRace()
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentRaceState == EMGRaceState::Racing || CurrentRaceState == EMGRaceState::Paused)
	{
		// Mark all unfinished racers as DNF
		for (FMGRacerData& Racer : Racers)
		{
			if (!Racer.bFinished)
			{
				Racer.bDNF = true;
			}
		}

		CalculateResults();
		SetRaceState(EMGRaceState::Finished);
		OnRaceFinished.Broadcast(RaceResults);

		// Notify all player controllers that race has ended
		NotifyPlayersRaceEnded();
	}
}

void AMGRaceGameMode::RestartRace()
{
	if (!HasAuthority())
	{
		return;
	}

	// Reset all racer data
	for (FMGRacerData& Racer : Racers)
	{
		Racer.CurrentLap = 0;
		Racer.CurrentCheckpoint = 0;
		Racer.Position = 0;
		Racer.TotalDistance = 0.0f;
		Racer.CurrentLapTime = 0.0f;
		Racer.BestLapTime = 0.0f;
		Racer.TotalTime = 0.0f;
		Racer.LapTimes.Empty();
		Racer.bFinished = false;
		Racer.FinishTime = 0.0f;
		Racer.bDNF = false;
		Racer.DriftScore = 0.0f;

		// Reset vehicle to start position
		if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
		{
			Vehicle->RespawnAtCheckpoint();
		}
	}

	RaceTime = 0.0f;
	FinishedCount = 0;

	SetRaceState(EMGRaceState::PreRace);
}

// ==========================================
// RACER MANAGEMENT
// ==========================================

int32 AMGRaceGameMode::RegisterRacer(AMGVehiclePawn* Vehicle, bool bIsAI, FText DisplayName)
{
	if (!HasAuthority())
	{
		return -1;
	}

	if (!Vehicle)
	{
		return -1;
	}

	FMGRacerData RacerData;
	RacerData.RacerIndex = Racers.Num();
	RacerData.Vehicle = Vehicle;
	RacerData.bIsAI = bIsAI;
	RacerData.DisplayName = DisplayName.IsEmpty() ? FText::FromString(FString::Printf(TEXT("Racer %d"), RacerData.RacerIndex + 1)) : DisplayName;

	Racers.Add(RacerData);

	return RacerData.RacerIndex;
}

void AMGRaceGameMode::UnregisterRacer(int32 RacerIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Racers.IsValidIndex(RacerIndex))
	{
		Racers.RemoveAt(RacerIndex);

		// Update indices
		for (int32 i = RacerIndex; i < Racers.Num(); ++i)
		{
			Racers[i].RacerIndex = i;
		}
	}
}

FMGRacerData AMGRaceGameMode::GetRacerData(int32 RacerIndex) const
{
	if (Racers.IsValidIndex(RacerIndex))
	{
		return Racers[RacerIndex];
	}
	return FMGRacerData();
}

// ==========================================
// CHECKPOINT/LAP TRACKING
// ==========================================

void AMGRaceGameMode::RegisterCheckpoint(AMGCheckpoint* Checkpoint, int32 CheckpointIndex)
{
	// Ensure array is large enough
	if (CheckpointIndex >= Checkpoints.Num())
	{
		Checkpoints.SetNum(CheckpointIndex + 1);
	}

	Checkpoints[CheckpointIndex] = Checkpoint;
}

void AMGRaceGameMode::OnCheckpointPassed(AMGVehiclePawn* Vehicle, int32 CheckpointIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	if (CurrentRaceState != EMGRaceState::Racing)
	{
		return;
	}

	int32 RacerIndex = GetRacerIndexForVehicle(Vehicle);
	if (!Racers.IsValidIndex(RacerIndex))
	{
		return;
	}

	FMGRacerData& Racer = Racers[RacerIndex];

	// Verify checkpoint order
	int32 ExpectedCheckpoint = Racer.CurrentCheckpoint;
	if (CheckpointIndex != ExpectedCheckpoint)
	{
		// Wrong checkpoint - might be going backwards or cutting
		return;
	}

	// Update checkpoint
	Racer.CurrentCheckpoint = (CheckpointIndex + 1) % Checkpoints.Num();

	// Update vehicle
	if (AMGVehiclePawn* VehiclePawn = Racer.Vehicle.Get())
	{
		VehiclePawn->RecordCheckpoint(CheckpointIndex);
	}

	// Check for lap completion (crossed start/finish)
	if (CheckpointIndex == 0 && Racer.CurrentLap > 0)
	{
		// This was the start/finish line - record lap
		Racer.LapTimes.Add(Racer.CurrentLapTime);

		// Update best lap
		if (Racer.BestLapTime <= 0.0f || Racer.CurrentLapTime < Racer.BestLapTime)
		{
			Racer.BestLapTime = Racer.CurrentLapTime;
		}

		OnLapCompleted.Broadcast(RacerIndex, Racer.CurrentLapTime);

		// Notify fuel subsystem of lap completion for fuel tracking
		if (UWorld* World = GetWorld())
		{
			if (UMGFuelSubsystem* FuelSubsystem = World->GetSubsystem<UMGFuelSubsystem>())
			{
				// Use vehicle name as the fuel system vehicle ID
				if (AMGVehiclePawn* VehiclePawn = Racer.Vehicle.Get())
				{
					FName VehicleID = FName(*VehiclePawn->GetName());
					FuelSubsystem->OnLapCompleted(VehicleID, Racer.CurrentLap);
				}
			}
		}

		// Reset lap timer
		Racer.CurrentLapTime = 0.0f;
	}

	// Increment lap on first checkpoint pass OR after completing a lap
	if (CheckpointIndex == 0)
	{
		Racer.CurrentLap++;

		// Update vehicle
		if (AMGVehiclePawn* VehiclePawn = Racer.Vehicle.Get())
		{
			VehiclePawn->SetCurrentLap(Racer.CurrentLap);
		}

		// Check if finished
		if (RaceConfig.RaceType == EMGRaceType::Circuit && Racer.CurrentLap > RaceConfig.LapCount)
		{
			// Race finished for this racer
			Racer.bFinished = true;
			Racer.FinishTime = Racer.TotalTime;
			FinishedCount++;

			OnRacerFinished.Broadcast(RacerIndex, FinishedCount);
		}
	}
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void AMGRaceGameMode::SetRaceState(EMGRaceState NewState)
{
	if (CurrentRaceState != NewState)
	{
		CurrentRaceState = NewState;
		OnRaceStateChanged.Broadcast(NewState);
	}
}

void AMGRaceGameMode::UpdateCountdown(float DeltaTime)
{
	CountdownAccumulator += DeltaTime;

	if (CountdownAccumulator >= 1.0f)
	{
		CountdownAccumulator = 0.0f;
		CountdownSeconds--;

		if (CountdownSeconds > 0)
		{
			OnCountdownTick.Broadcast(CountdownSeconds);
		}
		else
		{
			// GO!
			FreezeAllVehicles(false);
			SetRaceState(EMGRaceState::Racing);
			OnRaceStarted.Broadcast();

			// Initialize lap to 1 for all racers
			for (FMGRacerData& Racer : Racers)
			{
				Racer.CurrentLap = 1;
				if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
				{
					Vehicle->SetCurrentLap(1);
				}
			}

			// Notify all player controllers that race has started
			NotifyPlayersRaceStarted();
		}
	}
}

void AMGRaceGameMode::UpdateRaceTiming(float DeltaTime)
{
	RaceTime += DeltaTime;

	// Update each racer's timing and drift score
	for (FMGRacerData& Racer : Racers)
	{
		if (!Racer.bFinished && !Racer.bDNF)
		{
			Racer.CurrentLapTime += DeltaTime;
			Racer.TotalTime += DeltaTime;

			// Capture drift score from vehicle
			if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
			{
				FMGVehicleRuntimeState VehicleState = Vehicle->GetRuntimeState();
				Racer.DriftScore = VehicleState.DriftScore;
			}
		}
	}

	// Check time limit
	if (RaceConfig.TimeLimit > 0.0f && RaceTime >= RaceConfig.TimeLimit)
	{
		EndRace();
	}
}

void AMGRaceGameMode::UpdatePositions()
{
	// Calculate progress for each racer
	for (FMGRacerData& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
		{
			// Progress = (Lap * CheckpointCount + CurrentCheckpoint) + fractional progress to next checkpoint
			float Progress = (Racer.CurrentLap * Checkpoints.Num()) + Racer.CurrentCheckpoint;

			// Add fractional progress based on distance to next checkpoint
			int32 NextCheckpointIndex = (Racer.CurrentCheckpoint + 1) % Checkpoints.Num();
			if (Checkpoints.IsValidIndex(Racer.CurrentCheckpoint) && Checkpoints.IsValidIndex(NextCheckpointIndex))
			{
				AMGCheckpoint* CurrentCP = Checkpoints[Racer.CurrentCheckpoint].Get();
				AMGCheckpoint* NextCP = Checkpoints[NextCheckpointIndex].Get();

				if (CurrentCP && NextCP)
				{
					FVector VehiclePos = Vehicle->GetActorLocation();
					FVector CurrentCPPos = CurrentCP->GetActorLocation();
					FVector NextCPPos = NextCP->GetActorLocation();

					// Calculate total distance between checkpoints
					float TotalDist = FVector::Dist(CurrentCPPos, NextCPPos);
					if (TotalDist > 0.0f)
					{
						// Calculate distance vehicle has traveled from current checkpoint toward next
						float DistFromCurrent = FVector::Dist(CurrentCPPos, VehiclePos);

						// Use dot product to check if vehicle is actually progressing toward next checkpoint
						FVector ToNextCP = (NextCPPos - CurrentCPPos).GetSafeNormal();
						FVector ToVehicle = (VehiclePos - CurrentCPPos).GetSafeNormal();
						float DotProgress = FVector::DotProduct(ToNextCP, ToVehicle);

						// Only add progress if moving toward next checkpoint
						if (DotProgress > 0.0f)
						{
							float FractionalProgress = FMath::Clamp(DistFromCurrent / TotalDist, 0.0f, 0.99f);
							Progress += FractionalProgress;
						}
					}
				}
			}

			Racer.TotalDistance = Progress;
		}
	}

	// Sort by progress (descending) - finished racers first
	TArray<FMGRacerData*> SortedRacers;
	for (FMGRacerData& Racer : Racers)
	{
		SortedRacers.Add(&Racer);
	}

	SortedRacers.Sort([](const FMGRacerData* A, const FMGRacerData* B)
	{
		// Finished racers come first, sorted by finish time
		if (A->bFinished && B->bFinished)
		{
			return A->FinishTime < B->FinishTime;
		}
		if (A->bFinished) return true;
		if (B->bFinished) return false;

		// DNF racers come last
		if (A->bDNF && !B->bDNF) return false;
		if (!A->bDNF && B->bDNF) return true;

		// Sort by progress
		return A->TotalDistance > B->TotalDistance;
	});

	// Assign positions
	for (int32 i = 0; i < SortedRacers.Num(); ++i)
	{
		int32 NewPosition = i + 1;
		if (SortedRacers[i]->Position != NewPosition)
		{
			int32 OldPosition = SortedRacers[i]->Position;
			SortedRacers[i]->Position = NewPosition;

			// Update vehicle
			if (AMGVehiclePawn* Vehicle = SortedRacers[i]->Vehicle.Get())
			{
				Vehicle->SetRacePosition(NewPosition);
			}

			// Broadcast position change
			OnPositionChanged.Broadcast(SortedRacers[i]->RacerIndex, NewPosition);
		}
	}

	// Update HUD subsystem with player data
	UpdateHUDSubsystem();
}

void AMGRaceGameMode::CheckRaceComplete()
{
	// Check if all racers have finished
	if (FinishedCount >= Racers.Num())
	{
		CalculateResults();
		SetRaceState(EMGRaceState::Finished);
		OnRaceFinished.Broadcast(RaceResults);

		// Notify RaceFlowManager for reward processing
		NotifyRaceFlowManager();

		// Notify all player controllers that race has ended
		NotifyPlayersRaceEnded();
	}
}

void AMGRaceGameMode::CalculateResults()
{
	// Sort racers by final position
	TArray<FMGRacerData> SortedRacers = Racers;
	SortedRacers.Sort([](const FMGRacerData& A, const FMGRacerData& B)
	{
		return A.Position < B.Position;
	});

	// Build results
	RaceResults.Config = RaceConfig;
	RaceResults.RacerResults = SortedRacers;
	RaceResults.TotalRaceTime = RaceTime;

	// Find best lap
	RaceResults.BestLapTime = MAX_FLT;
	RaceResults.BestLapRacerIndex = -1;
	for (const FMGRacerData& Racer : Racers)
	{
		if (Racer.BestLapTime > 0.0f && Racer.BestLapTime < RaceResults.BestLapTime)
		{
			RaceResults.BestLapTime = Racer.BestLapTime;
			RaceResults.BestLapRacerIndex = Racer.RacerIndex;
		}
	}

	// Calculate player rewards
	if (Racers.IsValidIndex(PlayerRacerIndex))
	{
		const FMGRacerData& PlayerData = Racers[PlayerRacerIndex];
		RaceResults.bPlayerWon = (PlayerData.Position == 1);
		RaceResults.CreditsEarned = CalculateCreditsEarned(PlayerData.Position, RaceResults.bPlayerWon);

		// Reputation based on position and performance
		RaceResults.ReputationEarned = FMath::Max(0, 100 - (PlayerData.Position - 1) * 15);

		// Bonus for pink slip win
		if (RaceConfig.bPinkSlipRace && RaceResults.bPlayerWon)
		{
			RaceResults.ReputationEarned += 200;
		}
	}
}

int64 AMGRaceGameMode::CalculateCreditsEarned(int32 Position, bool bWon) const
{
	// Base credits by position
	static const int64 PositionCredits[] = {
		10000,  // 1st
		7500,   // 2nd
		5000,   // 3rd
		3500,   // 4th
		2500,   // 5th
		2000,   // 6th
		1500,   // 7th
		1000    // 8th+
	};

	int32 Index = FMath::Min(Position - 1, 7);
	int64 BaseCredits = PositionCredits[Index];

	// Multipliers
	float Multiplier = 1.0f;

	// Pink slip bonus
	if (RaceConfig.bPinkSlipRace && bWon)
	{
		Multiplier *= 3.0f;
	}

	// Difficulty bonus
	Multiplier += RaceConfig.AIDifficulty * 0.5f;

	// Lap bonus
	Multiplier += (RaceConfig.LapCount - 3) * 0.1f;

	return static_cast<int64>(BaseCredits * Multiplier);
}

int32 AMGRaceGameMode::GetRacerIndexForVehicle(AMGVehiclePawn* Vehicle) const
{
	for (int32 i = 0; i < Racers.Num(); ++i)
	{
		if (Racers[i].Vehicle.Get() == Vehicle)
		{
			return i;
		}
	}
	return -1;
}

void AMGRaceGameMode::FreezeAllVehicles(bool bFreeze)
{
	for (const FMGRacerData& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Racer.Vehicle.Get())
		{
			if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Vehicle->GetRootComponent()))
			{
				RootPrimitive->SetSimulatePhysics(!bFreeze);
			}
		}
	}
}

void AMGRaceGameMode::NotifyPlayersRaceStarted()
{
	// Notify all player controllers that race has started
	for (TActorIterator<AMGPlayerController> It(GetWorld()); It; ++It)
	{
		AMGPlayerController* PC = *It;
		if (PC)
		{
			PC->ClientOnRaceStarted();
		}
	}
}

void AMGRaceGameMode::NotifyPlayersRaceEnded()
{
	// Notify all player controllers that race has ended
	for (TActorIterator<AMGPlayerController> It(GetWorld()); It; ++It)
	{
		AMGPlayerController* PC = *It;
		if (PC)
		{
			PC->ClientOnRaceEnded();
		}
	}
}

void AMGRaceGameMode::UpdateHUDSubsystem()
{
	// Get HUD subsystem
	UMGRaceHUDSubsystem* HUDSubsystem = GetWorld()->GetSubsystem<UMGRaceHUDSubsystem>();
	if (!HUDSubsystem)
	{
		return;
	}

	// Update player's race status
	if (Racers.IsValidIndex(PlayerRacerIndex))
	{
		const FMGRacerData& PlayerData = Racers[PlayerRacerIndex];

		FMGRaceStatus Status;
		Status.CurrentPosition = PlayerData.Position;
		Status.TotalRacers = Racers.Num();
		Status.CurrentLap = PlayerData.CurrentLap;
		Status.TotalLaps = RaceConfig.LapCount;
		Status.CurrentLapTime = PlayerData.CurrentLapTime;
		Status.BestLapTime = PlayerData.BestLapTime;
		Status.TotalRaceTime = PlayerData.TotalTime;

		HUDSubsystem->UpdateRaceStatus(Status);
	}
}

void AMGRaceGameMode::NotifyRaceFlowManager()
{
	// Get RaceFlowManager from GameInstance and notify of race completion
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGRaceFlowManager* FlowManager = GI->GetSubsystem<UMGRaceFlowManager>())
		{
			FlowManager->OnRaceFinished(RaceResults);
		}
	}
}

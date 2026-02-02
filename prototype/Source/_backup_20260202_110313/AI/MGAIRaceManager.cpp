// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGAIRaceManager.h"
#include "AI/MGRacingAIController.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleData.h"
#include "Track/MGTrackSpline.h"
#include "Engine/World.h"

UMGAIRaceManager::UMGAIRaceManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	// Default configuration
	Configuration.AIControllerClass = AMGRacingAIController::StaticClass();
	Configuration.PositionUpdateRate = 10.0f;
	Configuration.RubberBandingConfig.bEnableCatchUp = true;
	Configuration.RubberBandingConfig.bEnableSlowDown = true;
}

void UMGAIRaceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bRaceActive || ActiveOpponents.Num() == 0)
	{
		return;
	}

	// Update positions at configured rate
	PositionUpdateTimer += DeltaTime;
	float UpdateInterval = 1.0f / Configuration.PositionUpdateRate;

	if (PositionUpdateTimer >= UpdateInterval)
	{
		PositionUpdateTimer = 0.0f;
		UpdatePositions();
	}
}

// ==========================================
// INITIALIZATION
// ==========================================

void UMGAIRaceManager::SetTrackSpline(AMGTrackSpline* InTrackSpline)
{
	TrackSpline = InTrackSpline;

	if (TrackSpline)
	{
		TrackLength = TrackSpline->GetTrackLength();
	}
}

void UMGAIRaceManager::SetConfiguration(const FMGAIRaceManagerConfig& InConfig)
{
	Configuration = InConfig;
}

void UMGAIRaceManager::SetRaceParameters(float InTrackLength, int32 InTotalLaps)
{
	TrackLength = InTrackLength;
	TotalLaps = InTotalLaps;
}

// ==========================================
// AI SPAWNING
// ==========================================

int32 UMGAIRaceManager::SpawnAIOpponent(const FMGAIOpponentConfig& Config, const FTransform& SpawnTransform)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return -1;
	}

	// Determine vehicle class to spawn
	TSubclassOf<AMGVehiclePawn> VehicleClass = Config.VehiclePawnClass;

	if (!VehicleClass)
	{
		// Try to get from vehicle model data
		if (Config.VehicleModel.IsValid())
		{
			UMGVehicleModelData* ModelData = Config.VehicleModel.LoadSynchronous();
			if (ModelData && ModelData->VehicleBlueprintClass.IsValid())
			{
				VehicleClass = Cast<UClass>(ModelData->VehicleBlueprintClass.LoadSynchronous());
			}
		}
	}

	if (!VehicleClass)
	{
		// Use default vehicle pawn
		VehicleClass = AMGVehiclePawn::StaticClass();
	}

	// Spawn the vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AMGVehiclePawn* SpawnedVehicle = World->SpawnActor<AMGVehiclePawn>(
		VehicleClass,
		SpawnTransform,
		SpawnParams
	);

	if (!SpawnedVehicle)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGAIRaceManager: Failed to spawn AI vehicle"));
		return -1;
	}

	// Spawn the AI controller
	TSubclassOf<AMGRacingAIController> ControllerClass = Configuration.AIControllerClass;
	if (!ControllerClass)
	{
		ControllerClass = AMGRacingAIController::StaticClass();
	}

	AMGRacingAIController* AIController = World->SpawnActor<AMGRacingAIController>(ControllerClass);
	if (!AIController)
	{
		SpawnedVehicle->Destroy();
		UE_LOG(LogTemp, Warning, TEXT("MGAIRaceManager: Failed to spawn AI controller"));
		return -1;
	}

	// Setup the AI controller
	if (TrackSpline && TrackSpline->RacingLineSpline)
	{
		AIController->SetRacingLine(TrackSpline->RacingLineSpline);
	}
	AIController->SetDriverProfile(Config.DriverProfile);
	AIController->SetRubberBandingConfig(Configuration.RubberBandingConfig);

	// Possess the vehicle
	AIController->Possess(SpawnedVehicle);

	// Create opponent entry
	FMGActiveAIOpponent NewOpponent;
	NewOpponent.OpponentId = GetNextOpponentId();
	NewOpponent.VehiclePawn = SpawnedVehicle;
	NewOpponent.AIController = AIController;
	NewOpponent.DriverProfile = Config.DriverProfile;
	NewOpponent.CurrentPosition = Config.GridPosition + 1; // 1-indexed

	ActiveOpponents.Add(NewOpponent);

	UE_LOG(LogTemp, Log, TEXT("MGAIRaceManager: Spawned AI opponent %d (%s)"),
		NewOpponent.OpponentId, *Config.DriverProfile.DriverName.ToString());

	return NewOpponent.OpponentId;
}

TArray<int32> UMGAIRaceManager::SpawnAIOpponents(const TArray<FMGAIOpponentConfig>& Configs, const TArray<FTransform>& SpawnTransforms)
{
	TArray<int32> SpawnedIds;

	int32 NumToSpawn = FMath::Min(Configs.Num(), SpawnTransforms.Num());

	for (int32 i = 0; i < NumToSpawn; ++i)
	{
		int32 Id = SpawnAIOpponent(Configs[i], SpawnTransforms[i]);
		if (Id >= 0)
		{
			SpawnedIds.Add(Id);
		}
	}

	return SpawnedIds;
}

TArray<int32> UMGAIRaceManager::GenerateAIField(int32 OpponentCount, EMGAIDifficulty BaseDifficulty, const TArray<FTransform>& SpawnTransforms)
{
	TArray<FMGAIOpponentConfig> Configs;

	// Generate varied opponents around base difficulty
	for (int32 i = 0; i < OpponentCount; ++i)
	{
		FMGAIOpponentConfig Config;

		// Vary difficulty slightly
		EMGAIDifficulty Difficulty = BaseDifficulty;
		float RandomFactor = FMath::FRand();
		if (RandomFactor < 0.2f && BaseDifficulty > EMGAIDifficulty::Rookie)
		{
			Difficulty = static_cast<EMGAIDifficulty>(static_cast<int32>(BaseDifficulty) - 1);
		}
		else if (RandomFactor > 0.8f && BaseDifficulty < EMGAIDifficulty::Legend)
		{
			Difficulty = static_cast<EMGAIDifficulty>(static_cast<int32>(BaseDifficulty) + 1);
		}

		Config.DriverProfile = FMGAIDriverProfile();
		Config.DriverProfile.GenerateFromDifficulty(Difficulty);

		// Randomize personality
		int32 PersonalityRoll = FMath::RandRange(0, 4);
		Config.DriverProfile.Personality = static_cast<EMGAIPersonality>(PersonalityRoll);

		// Add some variation to skills
		float SkillVariation = FMath::FRandRange(-10.0f, 10.0f);
		Config.DriverProfile.SkillRating = FMath::Clamp(Config.DriverProfile.SkillRating + SkillVariation, 0.0f, 100.0f);

		// Generate driver name
		static const TArray<FString> FirstNames = {
			TEXT("Alex"), TEXT("Jordan"), TEXT("Casey"), TEXT("Morgan"), TEXT("Riley"),
			TEXT("Taylor"), TEXT("Quinn"), TEXT("Avery"), TEXT("Jamie"), TEXT("Drew")
		};
		static const TArray<FString> LastNames = {
			TEXT("Speed"), TEXT("Blaze"), TEXT("Thunder"), TEXT("Storm"), TEXT("Phoenix"),
			TEXT("Nitro"), TEXT("Turbo"), TEXT("Drift"), TEXT("Flash"), TEXT("Bolt")
		};

		FString Name = FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)] + TEXT(" ") +
			LastNames[FMath::RandRange(0, LastNames.Num() - 1)];
		Config.DriverProfile.DriverName = FText::FromString(Name);

		Config.GridPosition = i + 1; // Player is position 0

		Configs.Add(Config);
	}

	return SpawnAIOpponents(Configs, SpawnTransforms);
}

void UMGAIRaceManager::RemoveAIOpponent(int32 OpponentId)
{
	for (int32 i = ActiveOpponents.Num() - 1; i >= 0; --i)
	{
		if (ActiveOpponents[i].OpponentId == OpponentId)
		{
			FMGActiveAIOpponent& Opponent = ActiveOpponents[i];

			if (Opponent.AIController)
			{
				Opponent.AIController->StopRacing();
				Opponent.AIController->UnPossess();
				Opponent.AIController->Destroy();
			}

			if (Opponent.VehiclePawn)
			{
				Opponent.VehiclePawn->Destroy();
			}

			ActiveOpponents.RemoveAt(i);
			return;
		}
	}
}

void UMGAIRaceManager::RemoveAllAIOpponents()
{
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.AIController)
		{
			Opponent.AIController->StopRacing();
			Opponent.AIController->UnPossess();
			Opponent.AIController->Destroy();
		}

		if (Opponent.VehiclePawn)
		{
			Opponent.VehiclePawn->Destroy();
		}
	}

	ActiveOpponents.Empty();
}

// ==========================================
// RACE CONTROL
// ==========================================

void UMGAIRaceManager::InitializeForRace()
{
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		// Reset AI state
		if (Opponent.AIController)
		{
			Opponent.AIController->StopRacing();
			Opponent.AIController->SetAIEnabled(true);
		}

		Opponent.CurrentLap = 0;
		Opponent.TrackDistance = 0.0f;
		Opponent.TotalRaceDistance = 0.0f;
		Opponent.bFinished = false;
		Opponent.FinishTime = 0.0f;
	}

	bRaceActive = false;
}

void UMGAIRaceManager::StartRacing()
{
	bRaceActive = true;

	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.AIController)
		{
			Opponent.AIController->StartRacing();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("MGAIRaceManager: Started racing with %d AI opponents"), ActiveOpponents.Num());
}

void UMGAIRaceManager::StopRacing()
{
	bRaceActive = false;

	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.AIController)
		{
			Opponent.AIController->StopRacing();
		}
	}
}

void UMGAIRaceManager::SetAllPaused(bool bPaused)
{
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.AIController)
		{
			Opponent.AIController->SetAIEnabled(!bPaused);
		}
	}
}

void UMGAIRaceManager::OnAILapCompleted(int32 OpponentId)
{
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.OpponentId == OpponentId)
		{
			Opponent.CurrentLap++;
			return;
		}
	}
}

void UMGAIRaceManager::OnAIFinished(int32 OpponentId, float FinishTime)
{
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.OpponentId == OpponentId)
		{
			Opponent.bFinished = true;
			Opponent.FinishTime = FinishTime;

			if (Opponent.AIController)
			{
				Opponent.AIController->StopRacing();
			}

			OnAIOpponentFinished.Broadcast(OpponentId, FinishTime);
			return;
		}
	}
}

// ==========================================
// QUERIES
// ==========================================

TArray<FMGActiveAIOpponent> UMGAIRaceManager::GetAllOpponents() const
{
	return ActiveOpponents;
}

bool UMGAIRaceManager::GetOpponent(int32 OpponentId, FMGActiveAIOpponent& OutOpponent) const
{
	for (const FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (Opponent.OpponentId == OpponentId)
		{
			OutOpponent = Opponent;
			return true;
		}
	}
	return false;
}

TArray<FMGActiveAIOpponent> UMGAIRaceManager::GetOpponentsByPosition() const
{
	TArray<FMGActiveAIOpponent> Sorted = ActiveOpponents;

	Sorted.Sort([](const FMGActiveAIOpponent& A, const FMGActiveAIOpponent& B)
	{
		// Finished racers come first, sorted by finish time
		if (A.bFinished && B.bFinished)
		{
			return A.FinishTime < B.FinishTime;
		}
		if (A.bFinished) return true;
		if (B.bFinished) return false;

		// Otherwise sort by total race distance (higher = ahead)
		return A.TotalRaceDistance > B.TotalRaceDistance;
	});

	return Sorted;
}

void UMGAIRaceManager::SetPlayerVehicle(AMGVehiclePawn* InPlayerVehicle, int32 InPlayerLap, float InPlayerTrackDistance)
{
	PlayerVehicle = InPlayerVehicle;
	PlayerLap = InPlayerLap;
	PlayerTrackDistance = InPlayerTrackDistance;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGAIRaceManager::UpdatePositions()
{
	// Update track distances for all opponents
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		if (!Opponent.bFinished)
		{
			UpdateOpponentTrackDistance(Opponent);
		}
	}

	// Calculate player's total distance
	float PlayerTotalDistance = CalculateTotalRaceDistance(PlayerLap, PlayerTrackDistance);

	// Sort by total race distance
	TArray<FMGActiveAIOpponent*> SortedOpponents;
	for (FMGActiveAIOpponent& Opponent : ActiveOpponents)
	{
		SortedOpponents.Add(&Opponent);
	}

	SortedOpponents.Sort([](const FMGActiveAIOpponent* A, const FMGActiveAIOpponent* B)
	{
		if (A->bFinished && B->bFinished)
		{
			return A->FinishTime < B->FinishTime;
		}
		if (A->bFinished) return true;
		if (B->bFinished) return false;
		return A->TotalRaceDistance > B->TotalRaceDistance;
	});

	// Find where player fits in
	int32 PlayerPosition = 1;
	if (PlayerVehicle)
	{
		for (int32 i = 0; i < SortedOpponents.Num(); ++i)
		{
			if (!SortedOpponents[i]->bFinished && SortedOpponents[i]->TotalRaceDistance > PlayerTotalDistance)
			{
				PlayerPosition++;
			}
		}
	}

	// Find the leader's total distance (could be player or AI)
	float LeaderTotalDistance = PlayerTotalDistance;
	for (const FMGActiveAIOpponent* Opponent : SortedOpponents)
	{
		if (!Opponent->bFinished && Opponent->TotalRaceDistance > LeaderTotalDistance)
		{
			LeaderTotalDistance = Opponent->TotalRaceDistance;
		}
	}

	// Assign positions and update AI controllers
	int32 Position = 1;
	float PreviousDistance = FLT_MAX;
	int32 PlayerInserted = 0;
	int32 TotalRacers = SortedOpponents.Num() + (PlayerVehicle ? 1 : 0);

	for (FMGActiveAIOpponent* Opponent : SortedOpponents)
	{
		// Insert player position
		if (!PlayerInserted && PlayerVehicle && !Opponent->bFinished && PlayerTotalDistance > Opponent->TotalRaceDistance)
		{
			Position++;
			PlayerInserted = 1;
		}

		int32 OldPosition = Opponent->CurrentPosition;
		Opponent->CurrentPosition = Position;

		// Broadcast position change
		if (OldPosition != Position)
		{
			OnAIPositionChanged.Broadcast(Opponent->OpponentId, OldPosition, Position);
		}

		// Update AI controller with race info
		if (Opponent->AIController)
		{
			// Set race position
			Opponent->AIController->SetRacePosition(Position, TotalRacers);

			// Calculate and set distance to leader
			// Positive = behind leader, Negative = ahead of leader (i.e., is the leader)
			float DistanceToLeader = LeaderTotalDistance - Opponent->TotalRaceDistance;
			Opponent->AIController->SetDistanceToLeader(DistanceToLeader);
		}

		PreviousDistance = Opponent->TotalRaceDistance;
		Position++;
	}
}

void UMGAIRaceManager::UpdateOpponentTrackDistance(FMGActiveAIOpponent& Opponent)
{
	if (!Opponent.VehiclePawn || !TrackSpline)
	{
		return;
	}

	FVector VehicleLocation = Opponent.VehiclePawn->GetActorLocation();
	Opponent.TrackDistance = TrackSpline->GetClosestDistanceOnTrack(VehicleLocation);
	Opponent.TotalRaceDistance = CalculateTotalRaceDistance(Opponent.CurrentLap, Opponent.TrackDistance);
}

float UMGAIRaceManager::CalculateTotalRaceDistance(int32 Lap, float CurrentTrackDistance) const
{
	return (Lap * TrackLength) + CurrentTrackDistance;
}

int32 UMGAIRaceManager::GetNextOpponentId()
{
	return NextOpponentId++;
}

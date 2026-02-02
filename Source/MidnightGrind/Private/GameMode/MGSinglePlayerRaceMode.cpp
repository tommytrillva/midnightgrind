// Copyright Midnight Grind. All Rights Reserved.

#include "GameMode/MGSinglePlayerRaceMode.h"
#include "AI/MGAIRacerController.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Progression/MGPlayerProgression.h"
#include "AI/MGRacingLineGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"

AMGSinglePlayerRaceMode::AMGSinglePlayerRaceMode()
{
	PrimaryActorTick.bCanEverTick = true;

	// Race defaults
	bRaceActive = false;
	bCountdownActive = false;
	CountdownRemaining = 3.0f;
	RaceTimeElapsed = 0.0f;
	NumLaps = 3;

	// Progression defaults
	FirstPlaceCash = 5000;
	SecondPlaceCash = 3000;
	ThirdPlaceCash = 2000;
	CompletionCash = 1000;

	// AI personalities (varied driving styles)
	AIPersonalities = {
		TEXT("Aggressive"),      // Takes risks, late braking
		TEXT("Smooth"),          // Consistent, optimal lines
		TEXT("Defensive"),       // Protects position
		TEXT("Unpredictable"),   // Varies style
		TEXT("Technical")        // Perfect cornering
	};
}

void AMGSinglePlayerRaceMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Fast init - skipping heavy multiplayer subsystems"));

	// Create player progression
	PlayerProgression = NewObject<UMGPlayerProgression>(this);
	if (PlayerProgression)
	{
		PlayerProgression->Initialize();
	}
}

void AMGSinglePlayerRaceMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Race mode ready - spawn AI when player is ready"));
}

void AMGSinglePlayerRaceMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Player joined - spawning AI opponents"));

	// Spawn AI opponents after player is ready
	SpawnAIOpponents(5);

	// Start countdown after brief delay
	FTimerHandle CountdownTimer;
	GetWorldTimerManager().SetTimer(CountdownTimer, this, &AMGSinglePlayerRaceMode::StartRaceCountdown, 2.0f, false);
}

void AMGSinglePlayerRaceMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update countdown
	if (bCountdownActive)
	{
		CountdownRemaining -= DeltaTime;
		if (CountdownRemaining <= 0.0f)
		{
			bCountdownActive = false;
			StartRace();
		}
	}

	// Update race time
	if (bRaceActive)
	{
		RaceTimeElapsed += DeltaTime;
		UpdateRaceStandings(DeltaTime);
	}
}

// ============================================
// RACE MANAGEMENT
// ============================================

void AMGSinglePlayerRaceMode::StartRaceCountdown()
{
	if (bCountdownActive || bRaceActive)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Starting countdown: 3... 2... 1..."));

	bCountdownActive = true;
	CountdownRemaining = 3.0f;
	OnRaceCountdownStart.Broadcast(CountdownRemaining);
}

void AMGSinglePlayerRaceMode::StartRace()
{
	if (bRaceActive)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[SinglePlayerRace] GO GO GO! Race started!"));

	bRaceActive = true;
	RaceTimeElapsed = 0.0f;
	OnRaceStart.Broadcast();

	// Enable AI controllers
	for (AMGAIRacerController* AIRacer : AIRacers)
	{
		if (AIRacer)
		{
			AIRacer->StartRacing();
		}
	}

	// TODO: Enable player input if it was locked during countdown
}

void AMGSinglePlayerRaceMode::FinishRace(AMGVehiclePawn* Vehicle, float RaceTime)
{
	if (!Vehicle || !bRaceActive)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Vehicle finished! Time: %.2fs"), RaceTime);

	// Calculate position
	TArray<AMGVehiclePawn*> Standings = GetRaceStandings();
	int32 Position = Standings.Find(Vehicle) + 1;

	// Check if player finished
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->GetPawn() == Vehicle)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SinglePlayerRace] PLAYER FINISHED in position %d!"), Position);
		OnPlayerFinish.Broadcast(Position, RaceTime);
		AwardRaceRewards(Position, RaceTime);
	}

	// Check if race is complete (all vehicles finished)
	bool bAllFinished = true;
	for (AMGVehiclePawn* V : AIVehicles)
	{
		if (V && !V->HasFinishedRace())
		{
			bAllFinished = false;
			break;
		}
	}

	if (bAllFinished)
	{
		bRaceActive = false;
		OnRaceFinish.Broadcast(Standings[0], RaceTime);
		UE_LOG(LogTemp, Warning, TEXT("[SinglePlayerRace] Race complete! Winner: %s"), *Standings[0]->GetName());
	}
}

// ============================================
// AI OPPONENTS
// ============================================

void AMGSinglePlayerRaceMode::SpawnAIOpponents(int32 NumOpponents)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("[SinglePlayerRace] No world - cannot spawn AI"));
		return;
	}

	// Clear existing AI
	AIRacers.Empty();
	AIVehicles.Empty();

	UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Spawning %d AI opponents"), NumOpponents);

	for (int32 i = 0; i < NumOpponents; ++i)
	{
		AMGAIRacerController* AIRacer = SpawnAIRacer(i);
		if (AIRacer)
		{
			AIRacers.Add(AIRacer);
			if (AMGVehiclePawn* AIVehicle = Cast<AMGVehiclePawn>(AIRacer->GetPawn()))
			{
				AIVehicles.Add(AIVehicle);
				UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace]   AI #%d: %s personality"), i + 1, *AIPersonalities[i % AIPersonalities.Num()]);
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[SinglePlayerRace] Successfully spawned %d AI racers!"), AIRacers.Num());
}

AMGAIRacerController* AMGSinglePlayerRaceMode::SpawnAIRacer(int32 Index)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	// Get spawn location (TODO: use proper spawn points from track)
	FVector SpawnLocation = FVector(Index * 500.0f, 0.0f, 100.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// Select vehicle class
	TSubclassOf<AMGVehiclePawn> VehicleClass = nullptr;
	if (AIVehicleClasses.Num() > 0)
	{
		VehicleClass = AIVehicleClasses[Index % AIVehicleClasses.Num()];
	}
	else
	{
		// Fallback to default vehicle class
		VehicleClass = AMGVehiclePawn::StaticClass();
	}

	// Spawn AI controller
	AMGAIRacerController* AIController = World->SpawnActor<AMGAIRacerController>(
		AMGAIRacerController::StaticClass(),
		SpawnLocation,
		SpawnRotation
	);

	if (!AIController)
	{
		UE_LOG(LogTemp, Error, TEXT("[SinglePlayerRace] Failed to spawn AI controller %d"), Index);
		return nullptr;
	}

	// Spawn AI vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = AIController;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AMGVehiclePawn* AIVehicle = World->SpawnActor<AMGVehiclePawn>(
		VehicleClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (!AIVehicle)
	{
		UE_LOG(LogTemp, Error, TEXT("[SinglePlayerRace] Failed to spawn AI vehicle %d"), Index);
		AIController->Destroy();
		return nullptr;
	}

	// Possess vehicle
	AIController->Possess(AIVehicle);

	// Set AI personality
	FString Personality = AIPersonalities[Index % AIPersonalities.Num()];
	AIController->SetPersonality(Personality);

	// Configure AI difficulty (scale based on index for variety)
	float DifficultyMultiplier = 0.8f + (Index * 0.05f); // 0.8 to 1.0 range
	AIController->SetDifficultyMultiplier(DifficultyMultiplier);

	// Enable skill-based catch-up (NOT rubber banding!)
	AIController->SetSkillBasedCatchUpEnabled(true);

	return AIController;
}

TArray<AMGVehiclePawn*> AMGSinglePlayerRaceMode::GetRaceStandings() const
{
	TArray<AMGVehiclePawn*> AllVehicles = AIVehicles;

	// Add player vehicle
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		if (AMGVehiclePawn* PlayerVehicle = Cast<AMGVehiclePawn>(PC->GetPawn()))
		{
			AllVehicles.Add(PlayerVehicle);
		}
	}

	// Sort by track progress
	AllVehicles.Sort([this](const AMGVehiclePawn& A, const AMGVehiclePawn& B)
	{
		float ProgressA = CalculateTrackProgress(const_cast<AMGVehiclePawn*>(&A));
		float ProgressB = CalculateTrackProgress(const_cast<AMGVehiclePawn*>(&B));
		return ProgressA > ProgressB;
	});

	return AllVehicles;
}

void AMGSinglePlayerRaceMode::UpdateRaceStandings(float InDeltaTime)
{
	// Update standings periodically (not every frame for performance)
	static float TimeSinceLastUpdate = 0.0f;
	TimeSinceLastUpdate += InDeltaTime;

	if (TimeSinceLastUpdate >= 0.5f) // Update twice per second
	{
		TimeSinceLastUpdate = 0.0f;
		TArray<AMGVehiclePawn*> Standings = GetRaceStandings();
		
		// TODO: Broadcast standings update event for UI
	}
}

float AMGSinglePlayerRaceMode::CalculateTrackProgress(AMGVehiclePawn* Vehicle) const
{
	if (!Vehicle)
	{
		return 0.0f;
	}

	// TODO: Implement proper track progress calculation
	// For now, use simple distance from start
	float CurrentLap = Vehicle->GetCurrentLap();
	float LapProgress = Vehicle->GetLapProgress();
	
	return (CurrentLap * 1.0f) + (LapProgress * 0.01f);
}

// ============================================
// PROGRESSION
// ============================================

void AMGSinglePlayerRaceMode::AwardRaceRewards(int32 Position, float RaceTime)
{
	if (!PlayerProgression)
	{
		UE_LOG(LogTemp, Error, TEXT("[SinglePlayerRace] No player progression - cannot award rewards"));
		return;
	}

	int32 CashReward = CompletionCash;

	// Position bonuses
	switch (Position)
	{
	case 1:
		CashReward += FirstPlaceCash;
		UE_LOG(LogTemp, Warning, TEXT("[SinglePlayerRace] 1ST PLACE! +$%d"), FirstPlaceCash);
		break;
	case 2:
		CashReward += SecondPlaceCash;
		UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] 2nd place. +$%d"), SecondPlaceCash);
		break;
	case 3:
		CashReward += ThirdPlaceCash;
		UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] 3rd place. +$%d"), ThirdPlaceCash);
		break;
	default:
		UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Finished in position %d"), Position);
		break;
	}

	// Time bonuses (under 2 minutes gets bonus)
	if (RaceTime < 120.0f)
	{
		int32 TimeBonus = FMath::RoundToInt((120.0f - RaceTime) * 10.0f);
		CashReward += TimeBonus;
		UE_LOG(LogTemp, Log, TEXT("[SinglePlayerRace] Fast time bonus! +$%d"), TimeBonus);
	}

	// Award cash
	PlayerProgression->AddCash(CashReward);
	PlayerProgression->AddExperience(Position <= 3 ? 200 : 100);

	UE_LOG(LogTemp, Warning, TEXT("[SinglePlayerRace] Total rewards: $%d cash, +XP"), CashReward);

	// TODO: Check for unlocks
	// TODO: Save progression
}

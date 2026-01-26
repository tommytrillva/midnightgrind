// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Race Flow Subsystem - MVP Game Flow Orchestration

#include "Race/MGRaceFlowSubsystem.h"
#include "Core/MGGameStateSubsystem.h"
#include "RaceDirector/MGRaceDirectorSubsystem.h"
#include "Vehicle/MGVehicleSpawnSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "GameModes/MGRaceGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGRaceFlow, Log, All);

// ==========================================
// Track and Vehicle data (MVP hardcoded, later from data assets)
// ==========================================

namespace MGRaceFlowDefaults
{
	// Available tracks for MVP
	const TArray<FName> AvailableTracks = {
		FName("Track_Downtown"),
		FName("Track_Highway"),
		FName("Track_Industrial"),
		FName("Track_Mountain"),
		FName("Track_Airport"),
		FName("Track_Docks")
	};

	// Track display names
	const TMap<FName, FString> TrackNames = {
		{ FName("Track_Downtown"), TEXT("Downtown Circuit") },
		{ FName("Track_Highway"), TEXT("Highway Sprint") },
		{ FName("Track_Industrial"), TEXT("Industrial Zone") },
		{ FName("Track_Mountain"), TEXT("Mountain Pass") },
		{ FName("Track_Airport"), TEXT("Airport Runway") },
		{ FName("Track_Docks"), TEXT("Dockside Drift") }
	};

	// Track level names (for loading)
	const TMap<FName, FName> TrackLevels = {
		{ FName("Track_Downtown"), FName("LVL_Downtown") },
		{ FName("Track_Highway"), FName("LVL_Highway") },
		{ FName("Track_Industrial"), FName("LVL_Industrial") },
		{ FName("Track_Mountain"), FName("LVL_Mountain") },
		{ FName("Track_Airport"), FName("LVL_Airport") },
		{ FName("Track_Docks"), FName("LVL_Docks") }
	};

	// AI racer names
	const TArray<FString> AINames = {
		TEXT("Shadow"),
		TEXT("Nitro"),
		TEXT("Blaze"),
		TEXT("Phantom"),
		TEXT("Viper"),
		TEXT("Storm"),
		TEXT("Thunder"),
		TEXT("Midnight"),
		TEXT("Ghost"),
		TEXT("Demon"),
		TEXT("Apex"),
		TEXT("Chrome")
	};

	// Starter/common AI vehicles (class D-B)
	const TArray<FName> AIVehiclesLow = {
		FName("Vehicle_240SX"),
		FName("Vehicle_Civic"),
		FName("Vehicle_MX5"),
		FName("Vehicle_86"),
		FName("Vehicle_350Z"),
		FName("Vehicle_Mustang")
	};

	// Mid-tier AI vehicles (class B-A)
	const TArray<FName> AIVehiclesMid = {
		FName("Vehicle_Supra"),
		FName("Vehicle_RX7"),
		FName("Vehicle_Skyline"),
		FName("Vehicle_Evo"),
		FName("Vehicle_STI"),
		FName("Vehicle_M3")
	};

	// High-tier AI vehicles (class A-S)
	const TArray<FName> AIVehiclesHigh = {
		FName("Vehicle_GTR"),
		FName("Vehicle_Porsche"),
		FName("Vehicle_Ferrari"),
		FName("Vehicle_Lambo"),
		FName("Vehicle_McLaren")
	};

	// Position reward multipliers
	const TArray<float> PositionMultipliers = {
		1.0f,   // 1st
		0.7f,   // 2nd
		0.5f,   // 3rd
		0.35f,  // 4th
		0.25f,  // 5th
		0.15f,  // 6th
		0.10f,  // 7th
		0.05f   // 8th+
	};

	// XP per position
	const TArray<int32> PositionXP = {
		250,    // 1st
		175,    // 2nd
		125,    // 3rd
		100,    // 4th
		75,     // 5th
		50,     // 6th
		35,     // 7th
		25      // 8th+
	};
}

// ==========================================
// CONSTRUCTOR / INITIALIZATION
// ==========================================

UMGRaceFlowSubsystem::UMGRaceFlowSubsystem()
{
}

void UMGRaceFlowSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogMGRaceFlow, Log, TEXT("Race Flow Subsystem initialized"));

	// Cache subsystems on first tick (they may not exist yet)
	CacheSubsystems();
}

void UMGRaceFlowSubsystem::Deinitialize()
{
	// Cleanup any active race
	if (IsRaceActive())
	{
		UE_LOG(LogMGRaceFlow, Warning, TEXT("Deinitializing with active race - aborting"));
		AbortRace();
	}

	Super::Deinitialize();
}

void UMGRaceFlowSubsystem::CacheSubsystems()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		GameStateSubsystem = GI->GetSubsystem<UMGGameStateSubsystem>();
		RaceDirectorSubsystem = GI->GetSubsystem<UMGRaceDirectorSubsystem>();
		EconomySubsystem = GI->GetSubsystem<UMGEconomySubsystem>();
		GarageSubsystem = GI->GetSubsystem<UMGGarageSubsystem>();
	}
}

// ==========================================
// RACE FLOW CONTROL
// ==========================================

bool UMGRaceFlowSubsystem::StartRace(const FMGRaceSetupRequest& Request)
{
	if (!CanStartRace())
	{
		UE_LOG(LogMGRaceFlow, Warning, TEXT("Cannot start race - flow state: %d"), static_cast<int32>(CurrentState));
		return false;
	}

	// Validate setup
	FString Error;
	if (!ValidateSetup(Request, Error))
	{
		HandleError(Error);
		return false;
	}

	// Store setup
	CurrentSetup = Request;

	// Generate AI if not provided
	if (CurrentAIOpponents.Num() == 0 && Request.AICount > 0)
	{
		CurrentAIOpponents = GenerateAIOpponents(
			Request.AICount,
			Request.AIDifficulty,
			Request.PlayerVehicleID
		);
	}

	UE_LOG(LogMGRaceFlow, Log, TEXT("Starting race: Track=%s, Type=%s, Laps=%d, AI=%d"),
		*Request.TrackID.ToString(),
		*Request.RaceType.ToString(),
		Request.LapCount,
		Request.AICount);

	// Begin setup phase
	SetFlowState(EMGRaceFlowState::Setup);
	ExecuteSetup();

	return true;
}

bool UMGRaceFlowSubsystem::StartQuickRace(FName TrackID, FName VehicleID)
{
	FMGRaceSetupRequest Request;
	Request.TrackID = TrackID;
	Request.PlayerVehicleID = VehicleID;
	Request.RaceType = FName("Circuit");
	Request.LapCount = 3;
	Request.AICount = 7;
	Request.AIDifficulty = 0.5f;
	Request.TimeOfDay = 0.0f; // Midnight
	Request.Weather = 0.0f;   // Clear
	Request.BaseCashReward = 5000;
	Request.BaseRepReward = 100;

	return StartRace(Request);
}

void UMGRaceFlowSubsystem::AbortRace()
{
	if (CurrentState == EMGRaceFlowState::Idle)
	{
		return;
	}

	UE_LOG(LogMGRaceFlow, Log, TEXT("Aborting race"));

	// Clear result to indicate abort
	LastResult = FMGRaceFlowResult();
	LastResult.bRaceCompleted = false;

	// Stop race director
	if (RaceDirectorSubsystem.IsValid())
	{
		RaceDirectorSubsystem->EndRace();
	}

	// Return to garage
	SetFlowState(EMGRaceFlowState::Returning);
	ExecuteReturn();
}

void UMGRaceFlowSubsystem::RestartRace()
{
	if (CurrentState == EMGRaceFlowState::Idle)
	{
		return;
	}

	UE_LOG(LogMGRaceFlow, Log, TEXT("Restarting race"));

	// Stop current race
	if (RaceDirectorSubsystem.IsValid())
	{
		RaceDirectorSubsystem->ResetRace();
	}

	// Re-run setup with same config
	SetFlowState(EMGRaceFlowState::Setup);
	ExecuteSetup();
}

void UMGRaceFlowSubsystem::ContinueToGarage()
{
	if (CurrentState != EMGRaceFlowState::Results)
	{
		UE_LOG(LogMGRaceFlow, Warning, TEXT("ContinueToGarage called in wrong state: %d"), static_cast<int32>(CurrentState));
		return;
	}

	SetFlowState(EMGRaceFlowState::Returning);
	ExecuteReturn();
}

void UMGRaceFlowSubsystem::ContinueToNextRace()
{
	if (CurrentState != EMGRaceFlowState::Results)
	{
		UE_LOG(LogMGRaceFlow, Warning, TEXT("ContinueToNextRace called in wrong state: %d"), static_cast<int32>(CurrentState));
		return;
	}

	// Restart with same setup
	RestartRace();
}

// ==========================================
// STATE QUERIES
// ==========================================

bool UMGRaceFlowSubsystem::IsRaceActive() const
{
	return CurrentState != EMGRaceFlowState::Idle && CurrentState != EMGRaceFlowState::Error;
}

bool UMGRaceFlowSubsystem::CanStartRace() const
{
	return CurrentState == EMGRaceFlowState::Idle || CurrentState == EMGRaceFlowState::Error;
}

// ==========================================
// TRACK DATA
// ==========================================

TArray<FName> UMGRaceFlowSubsystem::GetAvailableTracks() const
{
	return MGRaceFlowDefaults::AvailableTracks;
}

FText UMGRaceFlowSubsystem::GetTrackDisplayName(FName TrackID) const
{
	if (const FString* Name = MGRaceFlowDefaults::TrackNames.Find(TrackID))
	{
		return FText::FromString(*Name);
	}
	return FText::FromName(TrackID);
}

UTexture2D* UMGRaceFlowSubsystem::GetTrackPreview(FName TrackID) const
{
	// MVP: Return nullptr, UI will use placeholder
	// Post-MVP: Load track preview images from data assets
	return nullptr;
}

bool UMGRaceFlowSubsystem::IsTrackUnlocked(FName TrackID) const
{
	// MVP: All tracks unlocked for initial playtesting
	// Post-MVP: Check REP tier and progression requirements
	return true;
}

FName UMGRaceFlowSubsystem::GetTrackLevelName(FName TrackID) const
{
	if (const FName* Level = MGRaceFlowDefaults::TrackLevels.Find(TrackID))
	{
		return *Level;
	}
	return TrackID; // Fallback to track ID as level name
}

// ==========================================
// AI SETUP
// ==========================================

TArray<FMGAIRacerSetup> UMGRaceFlowSubsystem::GenerateAIOpponents(int32 Count, float Difficulty, FName PlayerVehicleClass)
{
	TArray<FMGAIRacerSetup> Opponents;

	// Select appropriate vehicle pool based on difficulty
	const TArray<FName>* VehiclePool = &MGRaceFlowDefaults::AIVehiclesLow;
	if (Difficulty > 0.7f)
	{
		VehiclePool = &MGRaceFlowDefaults::AIVehiclesHigh;
	}
	else if (Difficulty > 0.4f)
	{
		VehiclePool = &MGRaceFlowDefaults::AIVehiclesMid;
	}

	// Generate racers
	for (int32 i = 0; i < Count && i < MGRaceFlowDefaults::AINames.Num(); ++i)
	{
		FMGAIRacerSetup Racer;

		// Assign name
		Racer.DisplayName = MGRaceFlowDefaults::AINames[i];

		// Assign vehicle (cycle through pool)
		int32 VehicleIndex = i % VehiclePool->Num();
		Racer.VehicleID = (*VehiclePool)[VehicleIndex];

		// Calculate skill based on difficulty with some variance
		float Variance = FMath::FRandRange(-0.1f, 0.1f);
		Racer.SkillLevel = FMath::Clamp(Difficulty + Variance, 0.1f, 1.0f);

		// Aggression varies
		Racer.Aggression = FMath::FRandRange(0.3f, 0.8f);

		// One racer is a rival
		if (i == 0)
		{
			Racer.bIsRival = true;
			Racer.SkillLevel = FMath::Min(Racer.SkillLevel + 0.1f, 1.0f);
			Racer.Aggression = FMath::Min(Racer.Aggression + 0.2f, 1.0f);
		}

		Opponents.Add(Racer);
	}

	return Opponents;
}

void UMGRaceFlowSubsystem::SetAIOpponents(const TArray<FMGAIRacerSetup>& Opponents)
{
	CurrentAIOpponents = Opponents;
}

// ==========================================
// QUICK RACE PRESETS
// ==========================================

FMGRaceSetupRequest UMGRaceFlowSubsystem::GetTestRaceSetup()
{
	FMGRaceSetupRequest Request;
	Request.TrackID = FName("Track_Downtown");
	Request.PlayerVehicleID = FName("Vehicle_240SX");
	Request.RaceType = FName("Circuit");
	Request.LapCount = 2;
	Request.AICount = 3;
	Request.AIDifficulty = 0.3f;
	Request.TimeOfDay = 0.0f;
	Request.Weather = 0.0f;
	Request.BaseCashReward = 3000;
	Request.BaseRepReward = 50;
	return Request;
}

FMGRaceSetupRequest UMGRaceFlowSubsystem::GetSprintRacePreset(FName TrackID, FName VehicleID)
{
	FMGRaceSetupRequest Request;
	Request.TrackID = TrackID;
	Request.PlayerVehicleID = VehicleID;
	Request.RaceType = FName("Sprint");
	Request.LapCount = 1;
	Request.AICount = 5;
	Request.AIDifficulty = 0.5f;
	Request.BaseCashReward = 3500;
	Request.BaseRepReward = 75;
	return Request;
}

FMGRaceSetupRequest UMGRaceFlowSubsystem::GetCircuitRacePreset(FName TrackID, FName VehicleID, int32 Laps)
{
	FMGRaceSetupRequest Request;
	Request.TrackID = TrackID;
	Request.PlayerVehicleID = VehicleID;
	Request.RaceType = FName("Circuit");
	Request.LapCount = Laps;
	Request.AICount = 7;
	Request.AIDifficulty = 0.5f;
	Request.BaseCashReward = 5000;
	Request.BaseRepReward = 100;
	return Request;
}

FMGRaceSetupRequest UMGRaceFlowSubsystem::GetDriftRacePreset(FName TrackID, FName VehicleID)
{
	FMGRaceSetupRequest Request;
	Request.TrackID = TrackID;
	Request.PlayerVehicleID = VehicleID;
	Request.RaceType = FName("Drift");
	Request.LapCount = 2;
	Request.AICount = 3;
	Request.AIDifficulty = 0.5f;
	Request.BaseCashReward = 4000;
	Request.BaseRepReward = 100;
	return Request;
}

FMGRaceSetupRequest UMGRaceFlowSubsystem::GetDragRacePreset(FName VehicleID)
{
	FMGRaceSetupRequest Request;
	Request.TrackID = FName("Track_Airport");
	Request.PlayerVehicleID = VehicleID;
	Request.RaceType = FName("Drag");
	Request.LapCount = 1;
	Request.AICount = 1;
	Request.AIDifficulty = 0.5f;
	Request.BaseCashReward = 2500;
	Request.BaseRepReward = 50;
	return Request;
}

// ==========================================
// INTERNAL FLOW
// ==========================================

void UMGRaceFlowSubsystem::SetFlowState(EMGRaceFlowState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	EMGRaceFlowState OldState = CurrentState;
	CurrentState = NewState;

	UE_LOG(LogMGRaceFlow, Log, TEXT("Flow state: %d -> %d"), static_cast<int32>(OldState), static_cast<int32>(NewState));

	OnFlowStateChanged.Broadcast(NewState);
}

void UMGRaceFlowSubsystem::ExecuteSetup()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Executing race setup"));

	// Ensure subsystems are cached
	CacheSubsystems();

	// Initialize race director
	if (RaceDirectorSubsystem.IsValid())
	{
		// Get track length (hardcoded for MVP, should come from track data)
		float TrackLength = 5000.0f; // 5km default

		RaceDirectorSubsystem->InitializeRace(CurrentSetup.LapCount, TrackLength);

		// Set difficulty
		RaceDirectorSubsystem->SetDifficultyPreset(FMath::RoundToInt(CurrentSetup.AIDifficulty * 4)); // 0-4 presets
	}

	// Mark setup complete
	OnRaceSetupComplete.Broadcast(true);

	// Begin loading
	SetFlowState(EMGRaceFlowState::Loading);
	ExecuteLoading();
}

void UMGRaceFlowSubsystem::ExecuteLoading()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Loading track: %s"), *CurrentSetup.TrackID.ToString());

	LoadingProgress = 0.0f;
	OnRaceLoadProgress.Broadcast(0.0f);

	// Tell game state subsystem to load the track
	if (GameStateSubsystem.IsValid())
	{
		FName LevelName = GetTrackLevelName(CurrentSetup.TrackID);
		GameStateSubsystem->StartRaceLoading(LevelName);
	}
	else
	{
		// Fallback: direct level load
		FName LevelName = GetTrackLevelName(CurrentSetup.TrackID);
		UGameplayStatics::OpenLevel(GetGameInstance(), LevelName);
	}

	// For MVP, simulate loading completion
	// In full implementation, we'd bind to level streaming events
	LoadingProgress = 1.0f;
	OnRaceLoadProgress.Broadcast(1.0f);
	OnLevelLoaded();
}

void UMGRaceFlowSubsystem::OnLevelLoaded()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Track loaded"));

	// Proceed to pre-race
	SetFlowState(EMGRaceFlowState::PreRace);
	ExecutePreRace();
}

void UMGRaceFlowSubsystem::ExecutePreRace()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Pre-race phase"));

	// Get world for vehicle spawning
	UWorld* World = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		World = GI->GetWorld();
	}

	// Spawn vehicles using spawn subsystem
	if (World)
	{
		if (UMGVehicleSpawnSubsystem* SpawnSubsystem = World->GetSubsystem<UMGVehicleSpawnSubsystem>())
		{
			// Build AI spawn requests
			TArray<FMGVehicleSpawnRequest> AIRequests;
			for (const FMGAIRacerSetup& AI : CurrentAIOpponents)
			{
				FMGVehicleSpawnRequest Request;
				Request.VehicleID = AI.VehicleID;
				Request.bIsAI = true;
				Request.DisplayName = AI.DisplayName;
				Request.AISkill = AI.SkillLevel;
				AIRequests.Add(Request);
			}

			// Spawn all vehicles
			bool bSpawned = SpawnSubsystem->SpawnRaceVehicles(CurrentSetup.PlayerVehicleID, AIRequests);
			if (bSpawned)
			{
				// Possess player vehicle
				APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
				if (PC)
				{
					SpawnSubsystem->PossessPlayerVehicle(PC);
				}
			}
			else
			{
				UE_LOG(LogMGRaceFlow, Warning, TEXT("Failed to spawn vehicles"));
			}
		}
		else
		{
			UE_LOG(LogMGRaceFlow, Warning, TEXT("Vehicle spawn subsystem not available"));
		}
	}

	// Register with race director for timing/positions
	if (RaceDirectorSubsystem.IsValid())
	{
		// Register player
		FGuid PlayerID = RaceDirectorSubsystem->RegisterRacer(
			TEXT("Player"),
			true,  // bIsPlayer
			1      // Start position
		);

		// Register AI opponents
		int32 StartPos = 2;
		for (const FMGAIRacerSetup& AI : CurrentAIOpponents)
		{
			FGuid AIID = RaceDirectorSubsystem->RegisterRacer(
				AI.DisplayName,
				false, // Not player
				StartPos++
			);

			if (AI.bIsRival)
			{
				RaceDirectorSubsystem->DesignateRival(AIID, true);
			}

			RaceDirectorSubsystem->SetRacerAggression(AIID, AI.Aggression);
		}
	}

	// Update game state
	if (GameStateSubsystem.IsValid())
	{
		GameStateSubsystem->BeginPreRace();
	}

	// Get and configure race game mode
	if (World)
	{
		CachedRaceGameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode());
		if (CachedRaceGameMode.IsValid())
		{
			// Configure game mode with race parameters
			FMGRaceConfig Config = ConvertSetupToConfig(CurrentSetup);
			CachedRaceGameMode->SetRaceConfig(Config);

			// Bind to game mode events
			BindRaceGameModeEvents();

			UE_LOG(LogMGRaceFlow, Log, TEXT("Configured race game mode: %d laps, AI difficulty %.2f"),
				Config.LapCount, Config.AIDifficulty);
		}
		else
		{
			UE_LOG(LogMGRaceFlow, Warning, TEXT("No race game mode found - using fallback"));
		}
	}

	// Start countdown
	SetFlowState(EMGRaceFlowState::Countdown);
	ExecuteCountdown();
}

void UMGRaceFlowSubsystem::ExecuteCountdown()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Countdown phase"));

	// Start countdown via race game mode
	if (CachedRaceGameMode.IsValid())
	{
		CachedRaceGameMode->StartCountdown();
		// Race will transition to Racing when game mode broadcasts OnRaceStarted
	}
	else
	{
		// Fallback: no game mode, proceed directly
		UE_LOG(LogMGRaceFlow, Warning, TEXT("No game mode - skipping countdown"));
		SetFlowState(EMGRaceFlowState::Racing);
		ExecuteRacing();
	}
}

void UMGRaceFlowSubsystem::ExecuteRacing()
{
	// Note: This is typically called via HandleRaceStarted() when game mode broadcasts OnRaceStarted
	// This direct call path is a fallback when no game mode is available

	UE_LOG(LogMGRaceFlow, Log, TEXT("Racing phase started (fallback path)"));

	// Race runs until game mode signals completion
	// This is triggered by HandleRaceFinished -> OnRaceGameModeEnd
}

void UMGRaceFlowSubsystem::OnRaceGameModeStart()
{
	// Called when race game mode signals race has started (after countdown)
	if (CurrentState != EMGRaceFlowState::Racing)
	{
		SetFlowState(EMGRaceFlowState::Racing);
	}
}

void UMGRaceFlowSubsystem::OnRaceGameModeEnd()
{
	// Called when race game mode signals race has ended
	UE_LOG(LogMGRaceFlow, Log, TEXT("Race game mode signaled end"));

	SetFlowState(EMGRaceFlowState::Cooldown);
	ExecuteCooldown();
}

void UMGRaceFlowSubsystem::ExecuteCooldown()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Cooldown phase"));

	// End race director
	if (RaceDirectorSubsystem.IsValid())
	{
		RaceDirectorSubsystem->EndRace();
	}

	// Short cooldown for celebrations/replays
	// For MVP, proceed directly to results

	SetFlowState(EMGRaceFlowState::Results);
	ExecuteResults();
}

void UMGRaceFlowSubsystem::ExecuteResults()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Results phase"));

	// Update game state
	if (GameStateSubsystem.IsValid())
	{
		GameStateSubsystem->EndRace();
	}

	// Build result from race director stats
	LastResult = FMGRaceFlowResult();
	LastResult.bRaceCompleted = true;

	if (RaceDirectorSubsystem.IsValid())
	{
		FMGRaceStatistics Stats = RaceDirectorSubsystem->GetRaceStatistics();
		FMGRacerState PlayerState = RaceDirectorSubsystem->GetPlayerState();

		LastResult.bPlayerFinished = PlayerState.bHasFinished;
		LastResult.PlayerPosition = PlayerState.CurrentPosition;
		LastResult.TotalRacers = Stats.TotalRacers;
		LastResult.PlayerTotalTime = PlayerState.FinishTime;
		LastResult.bPlayerWon = (PlayerState.CurrentPosition == 1);

		// Get finish order
		TArray<FGuid> FinishOrder = RaceDirectorSubsystem->GetFinishOrder();
		for (const FGuid& ID : FinishOrder)
		{
			// Convert to FName for storage
			LastResult.FinishOrder.Add(FName(*ID.ToString()));
		}
	}
	else
	{
		// MVP fallback: simulate a result
		LastResult.bPlayerFinished = true;
		LastResult.PlayerPosition = FMath::RandRange(1, 4);
		LastResult.TotalRacers = CurrentSetup.AICount + 1;
		LastResult.PlayerTotalTime = 180.0f + FMath::FRandRange(0.0f, 30.0f);
		LastResult.bPlayerWon = (LastResult.PlayerPosition == 1);
	}

	// Calculate and apply rewards
	CalculateRewards(LastResult);

	// Broadcast result
	OnRaceFinished.Broadcast(LastResult);

	// Process rewards
	SetFlowState(EMGRaceFlowState::ProcessingRewards);
	ExecuteRewardProcessing();
}

void UMGRaceFlowSubsystem::CalculateRewards(FMGRaceFlowResult& Result)
{
	// Get position multiplier
	int32 PosIndex = FMath::Clamp(Result.PlayerPosition - 1, 0, MGRaceFlowDefaults::PositionMultipliers.Num() - 1);
	float PosMult = MGRaceFlowDefaults::PositionMultipliers[PosIndex];

	// Calculate cash
	Result.CashEarned = FMath::RoundToInt64(CurrentSetup.BaseCashReward * PosMult);

	// Win bonus
	if (Result.bPlayerWon)
	{
		Result.CashEarned = FMath::RoundToInt64(Result.CashEarned * 1.25f);
	}

	// Calculate reputation
	Result.ReputationEarned = FMath::RoundToInt(CurrentSetup.BaseRepReward * PosMult);

	// Calculate XP
	int32 XPIndex = FMath::Clamp(Result.PlayerPosition - 1, 0, MGRaceFlowDefaults::PositionXP.Num() - 1);
	Result.XPEarned = MGRaceFlowDefaults::PositionXP[XPIndex];

	// Difficulty bonus
	Result.CashEarned = FMath::RoundToInt64(Result.CashEarned * (1.0f + CurrentSetup.AIDifficulty * 0.5f));
	Result.ReputationEarned = FMath::RoundToInt(Result.ReputationEarned * (1.0f + CurrentSetup.AIDifficulty * 0.3f));

	// Pink slip handling
	if (CurrentSetup.bIsPinkSlip)
	{
		if (Result.bPlayerWon)
		{
			Result.PinkSlipWonVehicleID = CurrentSetup.PinkSlipVehicleID;
		}
		else
		{
			Result.PinkSlipLostVehicleID = CurrentSetup.PlayerVehicleID;
		}
	}

	UE_LOG(LogMGRaceFlow, Log, TEXT("Rewards: Position=%d, Cash=%lld, Rep=%d, XP=%d"),
		Result.PlayerPosition,
		Result.CashEarned,
		Result.ReputationEarned,
		Result.XPEarned);
}

void UMGRaceFlowSubsystem::ExecuteRewardProcessing()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Processing rewards"));

	// Apply rewards
	ApplyRewards(LastResult);

	// Broadcast
	OnRewardsProcessed.Broadcast(LastResult);

	// MVP: Set input mode to menu so player can see results
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			FInputModeUIOnly InputMode;
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}

	// MVP: Auto-continue to garage after a short delay
	// In full game, player would press Continue on results screen
	if (UWorld* World = GetGameInstance()->GetWorld())
	{
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUFunction(this, FName("ContinueToGarage"));
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 5.0f, false);

		UE_LOG(LogMGRaceFlow, Log, TEXT("Auto-returning to garage in 5 seconds..."));
	}
}

void UMGRaceFlowSubsystem::ApplyRewards(const FMGRaceFlowResult& Result)
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Applying rewards - Cash: %lld, Rep: %d, XP: %d"),
		Result.CashEarned,
		Result.ReputationEarned,
		Result.XPEarned);

	// Award race winnings through Economy subsystem
	if (EconomySubsystem.IsValid() && Result.CashEarned > 0)
	{
		EconomySubsystem->AwardRaceWinnings(Result.CashEarned, CurrentSetup.TrackID);
		UE_LOG(LogMGRaceFlow, Log, TEXT("Awarded %lld credits via EconomySubsystem"), Result.CashEarned);
	}

	// Pink slip vehicle transfer
	if (!Result.PinkSlipWonVehicleID.IsNone())
	{
		UE_LOG(LogMGRaceFlow, Log, TEXT("Won vehicle via pink slip: %s"), *Result.PinkSlipWonVehicleID.ToString());

		// Add vehicle to player garage
		if (GarageSubsystem.IsValid())
		{
			FGuid NewVehicleId;
			FMGGarageResult GarageResult = GarageSubsystem->AddVehicleByID(Result.PinkSlipWonVehicleID, NewVehicleId);
			if (GarageResult.bSuccess)
			{
				UE_LOG(LogMGRaceFlow, Log, TEXT("Added pink slip vehicle to garage: %s"), *NewVehicleId.ToString());
			}
			else
			{
				UE_LOG(LogMGRaceFlow, Warning, TEXT("Failed to add pink slip vehicle: %s"), *GarageResult.ErrorMessage.ToString());
			}
		}

		// Record pink slip win in economy
		if (EconomySubsystem.IsValid())
		{
			EconomySubsystem->ProcessPinkSlipWin(0, Result.PinkSlipWonVehicleID);
		}
	}
	else if (!Result.PinkSlipLostVehicleID.IsNone())
	{
		UE_LOG(LogMGRaceFlow, Warning, TEXT("Lost vehicle via pink slip: %s"), *Result.PinkSlipLostVehicleID.ToString());

		// Remove vehicle from player garage
		if (GarageSubsystem.IsValid())
		{
			// Find and remove the vehicle with matching model ID
			TArray<FMGOwnedVehicle> Vehicles = GarageSubsystem->GetAllVehicles();
			for (const FMGOwnedVehicle& Vehicle : Vehicles)
			{
				// Match by vehicle ID (checking soft pointer path contains the ID)
				if (Vehicle.VehicleModelData.GetAssetName().Contains(Result.PinkSlipLostVehicleID.ToString()))
				{
					FMGGarageResult GarageResult = GarageSubsystem->RemoveVehicle(Vehicle.VehicleId);
					if (GarageResult.bSuccess)
					{
						UE_LOG(LogMGRaceFlow, Log, TEXT("Removed pink slip lost vehicle from garage"));
					}
					else
					{
						UE_LOG(LogMGRaceFlow, Warning, TEXT("Failed to remove pink slip vehicle: %s"), *GarageResult.ErrorMessage.ToString());
					}
					break;
				}
			}
		}

		// Record pink slip loss in economy
		if (EconomySubsystem.IsValid())
		{
			EconomySubsystem->ProcessPinkSlipLoss(0, Result.PinkSlipLostVehicleID);
		}
	}
}

void UMGRaceFlowSubsystem::ExecuteReturn()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Returning to garage"));

	// Unbind from game mode events
	UnbindRaceGameModeEvents();
	CachedRaceGameMode = nullptr;

	// Clear AI opponents
	CurrentAIOpponents.Empty();

	// Tell game state to go to garage
	if (GameStateSubsystem.IsValid())
	{
		GameStateSubsystem->GoToGarage();
	}

	// Load garage level
	UGameplayStatics::OpenLevel(GetGameInstance(), FName("L_Garage"));

	// Reset to idle
	SetFlowState(EMGRaceFlowState::Idle);
}

void UMGRaceFlowSubsystem::HandleError(const FString& Error)
{
	UE_LOG(LogMGRaceFlow, Error, TEXT("Race flow error: %s"), *Error);

	ErrorMessage = Error;
	SetFlowState(EMGRaceFlowState::Error);
	OnRaceError.Broadcast(Error);
}

bool UMGRaceFlowSubsystem::ValidateSetup(const FMGRaceSetupRequest& Request, FString& OutError)
{
	if (Request.TrackID.IsNone())
	{
		OutError = TEXT("No track specified");
		return false;
	}

	if (Request.PlayerVehicleID.IsNone())
	{
		OutError = TEXT("No player vehicle specified");
		return false;
	}

	if (Request.LapCount < 1)
	{
		OutError = TEXT("Invalid lap count");
		return false;
	}

	if (Request.AICount < 0 || Request.AICount > 15)
	{
		OutError = TEXT("Invalid AI count (0-15)");
		return false;
	}

	return true;
}

// ==========================================
// GAME MODE INTEGRATION
// ==========================================

void UMGRaceFlowSubsystem::BindRaceGameModeEvents()
{
	if (!CachedRaceGameMode.IsValid())
	{
		return;
	}

	// Bind to race started (after countdown)
	CachedRaceGameMode->OnRaceStarted.AddDynamic(this, &UMGRaceFlowSubsystem::HandleRaceStarted);

	// Bind to race finished
	CachedRaceGameMode->OnRaceFinished.AddDynamic(this, &UMGRaceFlowSubsystem::HandleRaceFinished);

	UE_LOG(LogMGRaceFlow, Log, TEXT("Bound to race game mode events"));
}

void UMGRaceFlowSubsystem::UnbindRaceGameModeEvents()
{
	if (!CachedRaceGameMode.IsValid())
	{
		return;
	}

	CachedRaceGameMode->OnRaceStarted.RemoveDynamic(this, &UMGRaceFlowSubsystem::HandleRaceStarted);
	CachedRaceGameMode->OnRaceFinished.RemoveDynamic(this, &UMGRaceFlowSubsystem::HandleRaceFinished);
}

FMGRaceConfig UMGRaceFlowSubsystem::ConvertSetupToConfig(const FMGRaceSetupRequest& Setup) const
{
	FMGRaceConfig Config;

	// Convert race type name to enum
	if (Setup.RaceType == FName("Circuit"))
	{
		Config.RaceType = EMGRaceType::Circuit;
	}
	else if (Setup.RaceType == FName("Sprint"))
	{
		Config.RaceType = EMGRaceType::Sprint;
	}
	else if (Setup.RaceType == FName("Drift"))
	{
		Config.RaceType = EMGRaceType::Drift;
	}
	else if (Setup.RaceType == FName("Drag"))
	{
		Config.RaceType = EMGRaceType::Drag;
	}
	else if (Setup.RaceType == FName("TimeTrial"))
	{
		Config.RaceType = EMGRaceType::TimeTrial;
	}
	else
	{
		Config.RaceType = EMGRaceType::Circuit;
	}

	Config.LapCount = Setup.LapCount;
	Config.AIDifficulty = Setup.AIDifficulty;
	Config.MaxRacers = Setup.AICount + 1; // +1 for player
	Config.bPinkSlipRace = Setup.bIsPinkSlip;
	Config.TimeOfDay = Setup.TimeOfDay;
	Config.Weather = Setup.Weather;
	Config.TrackName = Setup.TrackID;

	return Config;
}

void UMGRaceFlowSubsystem::HandleRaceStarted()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Race game mode signaled race started (GO!)"));

	// Transition to racing state
	if (CurrentState == EMGRaceFlowState::Countdown)
	{
		SetFlowState(EMGRaceFlowState::Racing);

		// Start race director
		if (RaceDirectorSubsystem.IsValid())
		{
			RaceDirectorSubsystem->StartRace();
		}

		// Update game state
		if (GameStateSubsystem.IsValid())
		{
			GameStateSubsystem->StartRacing();
		}

		// Broadcast
		OnRaceStarted.Broadcast();
	}
}

void UMGRaceFlowSubsystem::HandleRaceFinished(const FMGRaceResults& Results)
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Race game mode signaled race finished"));

	// Call our existing end handler
	OnRaceGameModeEnd();
}

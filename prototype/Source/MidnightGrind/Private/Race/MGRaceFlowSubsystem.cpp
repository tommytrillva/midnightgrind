// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Race Flow Subsystem - MVP Game Flow Orchestration

#include "Race/MGRaceFlowSubsystem.h"
#include "Core/MGGameStateSubsystem.h"
#include "RaceDirector/MGRaceDirectorSubsystem.h"
#include "Vehicle/MGVehicleSpawnSubsystem.h"
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
	// TODO: Load from data asset
	return nullptr;
}

bool UMGRaceFlowSubsystem::IsTrackUnlocked(FName TrackID) const
{
	// MVP: All tracks unlocked
	// TODO: Check progression
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

	// Start countdown after pre-race intro
	// For MVP, skip directly to countdown
	SetFlowState(EMGRaceFlowState::Countdown);
	ExecuteCountdown();
}

void UMGRaceFlowSubsystem::ExecuteCountdown()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Countdown phase"));

	// Countdown is handled by RaceGameMode
	// For MVP, proceed to racing state
	// Full implementation would wait for countdown events

	SetFlowState(EMGRaceFlowState::Racing);
	ExecuteRacing();
}

void UMGRaceFlowSubsystem::ExecuteRacing()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Racing phase started"));

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

	// Broadcast race started
	OnRaceStarted.Broadcast();

	// Race runs until game mode signals completion
	// This is triggered by OnRaceGameModeEnd
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

	// Stay in results state until user continues
}

void UMGRaceFlowSubsystem::ApplyRewards(const FMGRaceFlowResult& Result)
{
	// MVP: Just log rewards
	// TODO: Apply to progression subsystem

	UE_LOG(LogMGRaceFlow, Log, TEXT("Applied rewards - Cash: %lld, Rep: %d, XP: %d"),
		Result.CashEarned,
		Result.ReputationEarned,
		Result.XPEarned);

	// Pink slip vehicle transfer
	if (!Result.PinkSlipWonVehicleID.IsNone())
	{
		UE_LOG(LogMGRaceFlow, Log, TEXT("Won vehicle via pink slip: %s"), *Result.PinkSlipWonVehicleID.ToString());
		// TODO: Add vehicle to player garage
	}
	else if (!Result.PinkSlipLostVehicleID.IsNone())
	{
		UE_LOG(LogMGRaceFlow, Warning, TEXT("Lost vehicle via pink slip: %s"), *Result.PinkSlipLostVehicleID.ToString());
		// TODO: Remove vehicle from player garage
	}
}

void UMGRaceFlowSubsystem::ExecuteReturn()
{
	UE_LOG(LogMGRaceFlow, Log, TEXT("Returning to garage"));

	// Clear AI opponents
	CurrentAIOpponents.Empty();

	// Tell game state to go to garage
	if (GameStateSubsystem.IsValid())
	{
		GameStateSubsystem->GoToGarage();
	}

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

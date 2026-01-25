// Copyright Midnight Grind. All Rights Reserved.

#include "DevTools/MGDevCommands.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleMovementComponent.h"
#include "Vehicle/MGVehicleFactory.h"
#include "GameModes/MGRaceGameMode.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Track/MGTrackSubsystem.h"
#include "Track/MGSpawnPointActor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UMGDevCommands::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MG Dev Commands initialized. Use 'MG.' prefix for commands."));
}

void UMGDevCommands::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// VEHICLE COMMANDS
// ==========================================

void UMGDevCommands::SpawnVehicle(EMGVehiclePreset Preset)
{
	LogCommand(FString::Printf(TEXT("SpawnVehicle(%d)"), (int32)Preset));

	UWorld* World = GetWorld();
	if (!World) return;

	// Create vehicle data
	FMGVehicleData VehicleData = UMGVehicleFactory::CreateVehicleFromPreset(Preset);

	// Find spawn point
	FTransform SpawnTransform = FTransform::Identity;

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(World, AMGSpawnPointActor::StaticClass(), SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		SpawnTransform = SpawnPoints[0]->GetActorTransform();
	}
	else
	{
		// Use player start if no spawn points
		AActor* PlayerStart = UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass());
		if (PlayerStart)
		{
			SpawnTransform = PlayerStart->GetActorTransform();
		}
	}

	// Spawn vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	// Would spawn the vehicle pawn here - requires a blueprint class reference
	// For now, just log what would happen
	UE_LOG(LogTemp, Log, TEXT("Would spawn vehicle: %s (PI: %.0f) at location %s"),
		*VehicleData.DisplayName, VehicleData.Stats.PerformanceIndex, *SpawnTransform.GetLocation().ToString());
}

void UMGDevCommands::SpawnAI(int32 Count)
{
	LogCommand(FString::Printf(TEXT("SpawnAI(%d)"), Count));

	UWorld* World = GetWorld();
	if (!World) return;

	// Get player vehicle for matching
	AMGVehiclePawn* PlayerVehicle = GetPlayerVehicle();
	FMGVehicleData PlayerData;
	if (PlayerVehicle)
	{
		PlayerData = PlayerVehicle->GetVehicleConfiguration();
	}
	else
	{
		// Create default if no player
		PlayerData = UMGVehicleFactory::CreateVehicleFromPreset(EMGVehiclePreset::JDM_Mid);
	}

	// Find spawn points
	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(World, AMGSpawnPointActor::StaticClass(), SpawnPoints);

	for (int32 i = 0; i < Count; ++i)
	{
		// Create AI vehicle matched to player
		float Difficulty = FMath::FRandRange(0.3f, 0.7f);
		FMGVehicleData AIData = UMGVehicleFactory::CreateAIOpponent(PlayerData, Difficulty);

		FTransform SpawnTransform = FTransform::Identity;
		if (SpawnPoints.IsValidIndex(i + 1)) // +1 because 0 is for player
		{
			SpawnTransform = SpawnPoints[i + 1]->GetActorTransform();
		}
		else
		{
			// Offset from first spawn point
			SpawnTransform.SetLocation(FVector(0, (i + 1) * 300.0f, 0));
		}

		UE_LOG(LogTemp, Log, TEXT("Would spawn AI %d: %s (PI: %.0f)"),
			i + 1, *AIData.DisplayName, AIData.Stats.PerformanceIndex);
	}
}

void UMGDevCommands::DespawnAllAI()
{
	LogCommand(TEXT("DespawnAllAI"));

	// Would iterate and destroy all AI vehicles
	UE_LOG(LogTemp, Log, TEXT("Despawning all AI vehicles..."));
}

void UMGDevCommands::TeleportToStart()
{
	LogCommand(TEXT("TeleportToStart"));

	AMGVehiclePawn* Vehicle = GetPlayerVehicle();
	if (!Vehicle) return;

	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(World, AMGSpawnPointActor::StaticClass(), SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		Vehicle->SetActorTransform(SpawnPoints[0]->GetActorTransform());
		UE_LOG(LogTemp, Log, TEXT("Teleported to start"));
	}
}

void UMGDevCommands::ResetVehicle()
{
	LogCommand(TEXT("ResetVehicle"));

	AMGVehiclePawn* Vehicle = GetPlayerVehicle();
	if (Vehicle)
	{
		Vehicle->RespawnAtCheckpoint();
		UE_LOG(LogTemp, Log, TEXT("Vehicle reset"));
	}
}

// ==========================================
// RACE COMMANDS
// ==========================================

void UMGDevCommands::StartRace()
{
	LogCommand(TEXT("StartRace"));

	UWorld* World = GetWorld();
	if (!World) return;

	if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode()))
	{
		GameMode->StartCountdown();
		UE_LOG(LogTemp, Log, TEXT("Race countdown started"));
	}
}

void UMGDevCommands::FinishRace()
{
	LogCommand(TEXT("FinishRace"));

	UWorld* World = GetWorld();
	if (!World) return;

	if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode()))
	{
		GameMode->EndRace();
		UE_LOG(LogTemp, Log, TEXT("Race finished"));
	}
}

void UMGDevCommands::RestartRace()
{
	LogCommand(TEXT("RestartRace"));

	UWorld* World = GetWorld();
	if (!World) return;

	if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode()))
	{
		GameMode->RestartRace();
		UE_LOG(LogTemp, Log, TEXT("Race restarted"));
	}
}

void UMGDevCommands::SetLap(int32 LapNumber)
{
	LogCommand(FString::Printf(TEXT("SetLap(%d)"), LapNumber));

	AMGVehiclePawn* Vehicle = GetPlayerVehicle();
	if (Vehicle)
	{
		Vehicle->SetCurrentLap(LapNumber);
		UE_LOG(LogTemp, Log, TEXT("Set lap to %d"), LapNumber);
	}
}

void UMGDevCommands::SetPosition(int32 Position)
{
	LogCommand(FString::Printf(TEXT("SetPosition(%d)"), Position));

	AMGVehiclePawn* Vehicle = GetPlayerVehicle();
	if (Vehicle)
	{
		Vehicle->SetRacePosition(Position);
		UE_LOG(LogTemp, Log, TEXT("Set position to %d"), Position);
	}
}

void UMGDevCommands::SkipToResults()
{
	LogCommand(TEXT("SkipToResults"));

	// Force race end and show results
	FinishRace();
}

// ==========================================
// ECONOMY COMMANDS
// ==========================================

void UMGDevCommands::AddCredits(int32 Amount)
{
	LogCommand(FString::Printf(TEXT("AddCredits(%d)"), Amount));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGEconomySubsystem* Economy = GI->GetSubsystem<UMGEconomySubsystem>())
		{
			Economy->AddCredits(Amount, EMGTransactionType::Other, FText::FromString(TEXT("Dev Command")));
			UE_LOG(LogTemp, Log, TEXT("Added %d credits. New balance: %lld"), Amount, Economy->GetCredits());
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Could not add credits - EconomySubsystem not available"));
}

void UMGDevCommands::AddXP(int32 Amount)
{
	LogCommand(FString::Printf(TEXT("AddXP(%d)"), Amount));

	// Would add to progression subsystem
	UE_LOG(LogTemp, Log, TEXT("Added %d XP"), Amount);
}

void UMGDevCommands::SetLevel(int32 Level)
{
	LogCommand(FString::Printf(TEXT("SetLevel(%d)"), Level));

	// Would set level in progression subsystem
	UE_LOG(LogTemp, Log, TEXT("Set level to %d"), Level);
}

void UMGDevCommands::UnlockAllVehicles()
{
	LogCommand(TEXT("UnlockAllVehicles"));

	// Would unlock all vehicles in garage subsystem
	UE_LOG(LogTemp, Log, TEXT("All vehicles unlocked"));
}

// ==========================================
// CHEAT COMMANDS
// ==========================================

void UMGDevCommands::GodMode()
{
	LogCommand(TEXT("GodMode"));

	bGodMode = !bGodMode;
	UE_LOG(LogTemp, Log, TEXT("God Mode: %s"), bGodMode ? TEXT("ON") : TEXT("OFF"));
}

void UMGDevCommands::UnlimitedNitrous()
{
	LogCommand(TEXT("UnlimitedNitrous"));

	bUnlimitedNitrous = !bUnlimitedNitrous;
	UE_LOG(LogTemp, Log, TEXT("Unlimited Nitrous: %s"), bUnlimitedNitrous ? TEXT("ON") : TEXT("OFF"));
}

void UMGDevCommands::SuperSpeed()
{
	LogCommand(TEXT("SuperSpeed"));

	bSuperSpeed = !bSuperSpeed;
	UE_LOG(LogTemp, Log, TEXT("Super Speed: %s"), bSuperSpeed ? TEXT("ON") : TEXT("OFF"));
}

void UMGDevCommands::TimeScale(float Scale)
{
	LogCommand(FString::Printf(TEXT("TimeScale(%.2f)"), Scale));

	UWorld* World = GetWorld();
	if (World)
	{
		UGameplayStatics::SetGlobalTimeDilation(World, FMath::Clamp(Scale, 0.1f, 10.0f));
		UE_LOG(LogTemp, Log, TEXT("Time scale set to %.2f"), Scale);
	}
}

void UMGDevCommands::FreezeAI()
{
	LogCommand(TEXT("FreezeAI"));

	bAIFrozen = !bAIFrozen;
	UE_LOG(LogTemp, Log, TEXT("AI Frozen: %s"), bAIFrozen ? TEXT("ON") : TEXT("OFF"));
}

// ==========================================
// DEBUG COMMANDS
// ==========================================

void UMGDevCommands::ShowDebug()
{
	LogCommand(TEXT("ShowDebug"));

	bShowDebug = !bShowDebug;
	UE_LOG(LogTemp, Log, TEXT("Debug Display: %s"), bShowDebug ? TEXT("ON") : TEXT("OFF"));
}

void UMGDevCommands::ShowCheckpoints()
{
	LogCommand(TEXT("ShowCheckpoints"));

	bShowCheckpoints = !bShowCheckpoints;
	UE_LOG(LogTemp, Log, TEXT("Checkpoint Visualization: %s"), bShowCheckpoints ? TEXT("ON") : TEXT("OFF"));
}

void UMGDevCommands::ShowRacingLine()
{
	LogCommand(TEXT("ShowRacingLine"));

	bShowRacingLine = !bShowRacingLine;
	UE_LOG(LogTemp, Log, TEXT("Racing Line: %s"), bShowRacingLine ? TEXT("ON") : TEXT("OFF"));
}

void UMGDevCommands::PrintRaceState()
{
	LogCommand(TEXT("PrintRaceState"));

	UWorld* World = GetWorld();
	if (!World) return;

	if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode()))
	{
		UE_LOG(LogTemp, Log, TEXT("=== RACE STATE ==="));
		UE_LOG(LogTemp, Log, TEXT("Phase: %d"), (int32)GameMode->GetRacePhase());
		UE_LOG(LogTemp, Log, TEXT("Race Time: %.2f"), GameMode->GetRaceTime());
		// Would print more state info
	}
}

void UMGDevCommands::PrintVehicleStats()
{
	LogCommand(TEXT("PrintVehicleStats"));

	AMGVehiclePawn* Vehicle = GetPlayerVehicle();
	if (!Vehicle) return;

	FMGVehicleData Config = Vehicle->GetVehicleConfiguration();
	FMGVehicleRuntimeState State = Vehicle->GetRuntimeState();

	UE_LOG(LogTemp, Log, TEXT("=== VEHICLE STATS ==="));
	UE_LOG(LogTemp, Log, TEXT("Name: %s"), *Config.DisplayName);
	UE_LOG(LogTemp, Log, TEXT("PI: %.0f (%s)"), Config.Stats.PerformanceIndex,
		*UEnum::GetValueAsString(Config.Stats.PerformanceClass));
	UE_LOG(LogTemp, Log, TEXT("HP: %.0f | Torque: %.0f"), Config.Stats.Horsepower, Config.Stats.Torque);
	UE_LOG(LogTemp, Log, TEXT("Weight: %.0f kg"), Config.Stats.WeightKG);
	UE_LOG(LogTemp, Log, TEXT("Current Speed: %.1f MPH / %.1f KPH"), State.SpeedMPH, State.SpeedKPH);
	UE_LOG(LogTemp, Log, TEXT("RPM: %.0f / %d (%.1f%%)", State.RPM, Config.Stats.Redline, State.RPMPercent * 100.0f));
	UE_LOG(LogTemp, Log, TEXT("Gear: %d | Drifting: %s"), State.CurrentGear, State.bIsDrifting ? TEXT("YES") : TEXT("NO"));
}

// ==========================================
// QUICK TEST
// ==========================================

void UMGDevCommands::QuickRace(int32 AICount, int32 Laps)
{
	LogCommand(FString::Printf(TEXT("QuickRace(%d, %d)"), AICount, Laps));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGRaceFlowSubsystem* RaceFlow = GI->GetSubsystem<UMGRaceFlowSubsystem>())
		{
			// Create race setup
			FMGRaceSetupRequest Setup;
			Setup.TrackID = FName("Track_Downtown");
			Setup.RaceType = FName("Circuit");
			Setup.PlayerVehicleID = FName("Vehicle_SakuraGTR");
			Setup.LapCount = Laps;
			Setup.AICount = AICount;
			Setup.AIDifficulty = 0.5f;
			Setup.BaseCashReward = 5000;
			Setup.BaseRepReward = 100;

			if (RaceFlow->StartRace(Setup))
			{
				UE_LOG(LogTemp, Log, TEXT("Quick Race started: %d AI, %d Laps on %s"),
					AICount, Laps, *Setup.TrackID.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to start Quick Race"));
			}
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Could not start Quick Race - RaceFlowSubsystem not available"));
}

void UMGDevCommands::QuickTimeTrial(int32 Laps)
{
	LogCommand(FString::Printf(TEXT("QuickTimeTrial(%d)"), Laps));

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGRaceFlowSubsystem* RaceFlow = GI->GetSubsystem<UMGRaceFlowSubsystem>())
		{
			// Create time trial setup (no AI)
			FMGRaceSetupRequest Setup;
			Setup.TrackID = FName("Track_Downtown");
			Setup.RaceType = FName("TimeTrial");
			Setup.PlayerVehicleID = FName("Vehicle_SakuraGTR");
			Setup.LapCount = Laps;
			Setup.AICount = 0;
			Setup.BaseCashReward = 3000;
			Setup.BaseRepReward = 50;

			if (RaceFlow->StartRace(Setup))
			{
				UE_LOG(LogTemp, Log, TEXT("Quick Time Trial started: %d Laps on %s"),
					Laps, *Setup.TrackID.ToString());
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to start Quick Time Trial"));
			}
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Could not start Quick Time Trial - RaceFlowSubsystem not available"));
}

// ==========================================
// HELPERS
// ==========================================

AMGVehiclePawn* UMGDevCommands::GetPlayerVehicle() const
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return nullptr;

	return Cast<AMGVehiclePawn>(PC->GetPawn());
}

void UMGDevCommands::LogCommand(const FString& Command)
{
	UE_LOG(LogTemp, Log, TEXT("[MG.%s]"), *Command);
}

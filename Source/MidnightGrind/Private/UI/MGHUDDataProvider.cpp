// Copyright Midnight Grind. All Rights Reserved.

#include "UI/MGHUDDataProvider.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Race/MGRaceFlowSubsystem.h"
#include "Scoring/MGScoringSubsystem.h"
#include "Police/MGPoliceSubsystem.h"
#include "Drift/MGDriftSubsystem.h"

void UMGHUDDataProvider::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start update timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGHUDDataProvider::OnTick,
			UpdateInterval,
			true
		);
	}
}

void UMGHUDDataProvider::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
	}

	Super::Deinitialize();
}

// ==========================================
// FORMATTING
// ==========================================

FText UMGHUDDataProvider::GetFormattedSpeed() const
{
	int32 SpeedInt = FMath::RoundToInt(CachedHUDData.Speed);
	FString SpeedString = FString::Printf(TEXT("%d"), SpeedInt);

	FString UnitString = (SpeedDisplayMode == EMGSpeedDisplayMode::KPH) ? TEXT("KM/H") : TEXT("MPH");

	return FText::FromString(FString::Printf(TEXT("%s %s"), *SpeedString, *UnitString));
}

FText UMGHUDDataProvider::GetFormattedLapTime(float TimeSeconds) const
{
	int32 Minutes = FMath::FloorToInt(TimeSeconds / 60.0f);
	float Seconds = FMath::Fmod(TimeSeconds, 60.0f);
	int32 WholeSeconds = FMath::FloorToInt(Seconds);
	int32 Milliseconds = FMath::RoundToInt((Seconds - WholeSeconds) * 1000.0f);

	return FText::FromString(FString::Printf(TEXT("%d:%02d.%03d"), Minutes, WholeSeconds, Milliseconds));
}

FText UMGHUDDataProvider::GetFormattedPosition(int32 Position) const
{
	FString Suffix;
	int32 LastDigit = Position % 10;
	int32 LastTwoDigits = Position % 100;

	if (LastTwoDigits >= 11 && LastTwoDigits <= 13)
	{
		Suffix = TEXT("th");
	}
	else
	{
		switch (LastDigit)
		{
		case 1: Suffix = TEXT("st"); break;
		case 2: Suffix = TEXT("nd"); break;
		case 3: Suffix = TEXT("rd"); break;
		default: Suffix = TEXT("th"); break;
		}
	}

	return FText::FromString(FString::Printf(TEXT("%d%s"), Position, *Suffix));
}

FText UMGHUDDataProvider::GetFormattedGap(float GapSeconds) const
{
	if (FMath::Abs(GapSeconds) < 0.001f)
	{
		return FText::FromString(TEXT("--:--"));
	}

	bool bBehind = GapSeconds > 0.0f;
	float AbsGap = FMath::Abs(GapSeconds);

	FString GapString;
	if (AbsGap < 60.0f)
	{
		GapString = FString::Printf(TEXT("%.2f"), AbsGap);
	}
	else
	{
		int32 Minutes = FMath::FloorToInt(AbsGap / 60.0f);
		float Seconds = FMath::Fmod(AbsGap, 60.0f);
		GapString = FString::Printf(TEXT("%d:%05.2f"), Minutes, Seconds);
	}

	return FText::FromString(FString::Printf(TEXT("%s%s"), bBehind ? TEXT("+") : TEXT("-"), *GapString));
}

FText UMGHUDDataProvider::GetFormattedGear(int32 Gear) const
{
	if (Gear == 0)
	{
		return FText::FromString(TEXT("N"));
	}
	else if (Gear < 0)
	{
		return FText::FromString(TEXT("R"));
	}
	else
	{
		return FText::FromString(FString::Printf(TEXT("%d"), Gear));
	}
}

// ==========================================
// VEHICLE BINDING
// ==========================================

void UMGHUDDataProvider::SetPlayerVehicle(AMGVehiclePawn* Vehicle)
{
	PlayerVehicle = Vehicle;
}

void UMGHUDDataProvider::ClearPlayerVehicle()
{
	PlayerVehicle.Reset();
}

// ==========================================
// SETTINGS
// ==========================================

void UMGHUDDataProvider::SetSpeedDisplayMode(EMGSpeedDisplayMode Mode)
{
	SpeedDisplayMode = Mode;
}

void UMGHUDDataProvider::SetUpdateRate(float UpdatesPerSecond)
{
	UpdateInterval = 1.0f / FMath::Max(UpdatesPerSecond, 1.0f);

	// Restart timer with new interval
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickTimer);
		World->GetTimerManager().SetTimer(
			TickTimer,
			this,
			&UMGHUDDataProvider::OnTick,
			UpdateInterval,
			true
		);
	}
}

// ==========================================
// COUNTDOWN
// ==========================================

void UMGHUDDataProvider::StartCountdown(int32 StartValue)
{
	CachedHUDData.bCountdownActive = true;
	CachedHUDData.CountdownValue = StartValue;
	CachedHUDData.bShowGo = false;
}

void UMGHUDDataProvider::ShowGo()
{
	CachedHUDData.CountdownValue = 0;
	CachedHUDData.bShowGo = true;
}

void UMGHUDDataProvider::EndCountdown()
{
	CachedHUDData.bCountdownActive = false;
	CachedHUDData.bShowGo = false;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGHUDDataProvider::OnTick()
{
	UpdateHUDData();
	UpdateMinimapData();
}

void UMGHUDDataProvider::UpdateHUDData()
{
	FMGRaceHUDData NewData = CachedHUDData;

	// Gather data from various sources
	GatherVehicleData(NewData);
	GatherRaceData(NewData);
	GatherScoringData(NewData);
	GatherPoliceData(NewData);

	// Detect changes and broadcast events
	if (NewData.Position != CachedHUDData.Position && PreviousPosition != 0)
	{
		OnPositionChanged.Broadcast(CachedHUDData.Position, NewData.Position);
	}

	if (NewData.CurrentLap != CachedHUDData.CurrentLap && PreviousLap != 0)
	{
		OnLapCompleted.Broadcast(CachedHUDData.CurrentLap, CachedHUDData.CurrentLapTime);

		// Check for best lap
		if (CachedHUDData.CurrentLapTime > 0 &&
			(CachedHUDData.BestLapTime == 0 || CachedHUDData.CurrentLapTime < CachedHUDData.BestLapTime))
		{
			OnBestLap.Broadcast();
		}

		// Check for final lap
		if (NewData.CurrentLap == NewData.TotalLaps)
		{
			OnFinalLap.Broadcast();
		}
	}

	PreviousPosition = NewData.Position;
	PreviousLap = NewData.CurrentLap;

	CachedHUDData = NewData;
	OnHUDDataUpdated.Broadcast(CachedHUDData);
}

void UMGHUDDataProvider::UpdateMinimapData()
{
	FMGMinimapData NewData;

	if (PlayerVehicle.IsValid())
	{
		FVector PlayerPos = PlayerVehicle->GetActorLocation();
		NewData.PlayerPosition = FVector2D(PlayerPos.X, PlayerPos.Y);
		NewData.PlayerRotation = PlayerVehicle->GetActorRotation().Yaw;
	}

	// Gather other racers from race flow subsystem
	UWorld* World = GetWorld();
	if (World)
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			// Get other racer positions from Race Flow
			if (UMGRaceFlowSubsystem* RaceFlow = GameInstance->GetSubsystem<UMGRaceFlowSubsystem>())
			{
				TArray<FVector> RacerPositions = RaceFlow->GetAllRacerPositions();
				for (const FVector& Pos : RacerPositions)
				{
					// Skip if this is the player position (close to player)
					if (PlayerVehicle.IsValid())
					{
						float DistToPlayer = FVector::Dist2D(Pos, PlayerVehicle->GetActorLocation());
						if (DistToPlayer < 100.0f)
						{
							continue;
						}
					}
					NewData.OtherRacerPositions.Add(FVector2D(Pos.X, Pos.Y));
				}

				// Get checkpoint positions
				TArray<FVector> CheckpointPositions = RaceFlow->GetCheckpointPositions();
				for (const FVector& Pos : CheckpointPositions)
				{
					NewData.CheckpointPositions.Add(FVector2D(Pos.X, Pos.Y));
				}
			}

			// Get police positions from Police Subsystem
			if (UMGPoliceSubsystem* Police = GameInstance->GetSubsystem<UMGPoliceSubsystem>())
			{
				TArray<FVector> PolicePositions = Police->GetActivePolicePositions();
				for (const FVector& Pos : PolicePositions)
				{
					NewData.PolicePositions.Add(FVector2D(Pos.X, Pos.Y));
				}
			}
		}
	}

	CachedMinimapData = NewData;
	OnMinimapDataUpdated.Broadcast(CachedMinimapData);
}

void UMGHUDDataProvider::GatherVehicleData(FMGRaceHUDData& Data)
{
	if (!PlayerVehicle.IsValid())
	{
		return;
	}

	// Get speed from vehicle
	float SpeedCmPerSec = PlayerVehicle->GetVelocity().Size();
	Data.Speed = ConvertSpeed(SpeedCmPerSec);
	Data.SpeedMode = SpeedDisplayMode;

	// Get vehicle movement component for detailed data
	if (UMGVehicleMovementComponent* VehicleMovement = PlayerVehicle->FindComponentByClass<UMGVehicleMovementComponent>())
	{
		Data.CurrentGear = VehicleMovement->GetCurrentGear();
		Data.MaxGear = VehicleMovement->GetNumGears();
		Data.EngineRPM = VehicleMovement->GetEngineRPM();
		Data.MaxRPM = VehicleMovement->GetMaxRPM();
		Data.RPMNormalized = Data.MaxRPM > 0.0f ? Data.EngineRPM / Data.MaxRPM : 0.0f;
		Data.bInRedline = Data.RPMNormalized > 0.9f;

		// NOS data from vehicle movement
		Data.NOSAmount = VehicleMovement->GetNitroAmount();
		Data.bNOSActive = VehicleMovement->IsNitroActive();
	}
}

void UMGHUDDataProvider::GatherRaceData(FMGRaceHUDData& Data)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// Get race data from Race Flow Subsystem
	if (UMGRaceFlowSubsystem* RaceFlow = GameInstance->GetSubsystem<UMGRaceFlowSubsystem>())
	{
		Data.Position = RaceFlow->GetPlayerPosition();
		Data.TotalRacers = RaceFlow->GetTotalParticipants();
		Data.CurrentLap = RaceFlow->GetCurrentLap();
		Data.TotalLaps = RaceFlow->GetTotalLaps();
		Data.CurrentLapTime = RaceFlow->GetCurrentLapTime();
		Data.BestLapTime = RaceFlow->GetBestLapTime();
		Data.TotalRaceTime = RaceFlow->GetTotalRaceTime();
		Data.RaceProgress = RaceFlow->GetRaceProgress();

		// Get gap times if available
		Data.GapToLeader = RaceFlow->GetGapToLeader();
		Data.GapToNext = RaceFlow->GetGapToNextRacer();

		// Get distance to next checkpoint
		if (PlayerVehicle.IsValid())
		{
			Data.DistanceToCheckpoint = RaceFlow->GetDistanceToNextCheckpoint(PlayerVehicle->GetActorLocation());
		}
	}
}

void UMGHUDDataProvider::GatherScoringData(FMGRaceHUDData& Data)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// Get scoring data from Scoring Subsystem
	if (UMGScoringSubsystem* Scoring = GameInstance->GetSubsystem<UMGScoringSubsystem>())
	{
		Data.TotalScore = Scoring->GetTotalScore();
		Data.DriftScore = Scoring->GetDriftScore();
		Data.DriftMultiplier = Scoring->GetDriftMultiplier();
		Data.DriftCombo = Scoring->GetDriftCombo();
	}

	// Get drift state from Drift Subsystem
	if (UMGDriftSubsystem* Drift = GameInstance->GetSubsystem<UMGDriftSubsystem>())
	{
		Data.bIsDrifting = Drift->IsDrifting();
		Data.DriftAngle = Drift->GetCurrentDriftAngle();
	}
}

void UMGHUDDataProvider::GatherPoliceData(FMGRaceHUDData& Data)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	// Get police data from Police Subsystem
	if (UMGPoliceSubsystem* Police = GameInstance->GetSubsystem<UMGPoliceSubsystem>())
	{
		Data.HeatLevel = Police->GetCurrentHeatLevel();
		Data.bInPursuit = Police->IsInPursuit();
		Data.CooldownProgress = Police->GetCooldownProgress();
	}
}

float UMGHUDDataProvider::ConvertSpeed(float SpeedCmPerSec) const
{
	// Convert cm/s to km/h or mph
	float SpeedKmh = SpeedCmPerSec * 0.036f; // cm/s to km/h

	if (SpeedDisplayMode == EMGSpeedDisplayMode::MPH)
	{
		return SpeedKmh * 0.621371f; // km/h to mph
	}

	return SpeedKmh;
}

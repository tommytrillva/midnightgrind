// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGAIRacerSubsystem.h"
#include "AI/MGAIDriverProfile.h"
#include "AI/MGAIRacerController.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UMGAIRacerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default controller class
	if (!AIControllerClass)
	{
		AIControllerClass = AMGAIRacerController::StaticClass();
	}
}

void UMGAIRacerSubsystem::Deinitialize()
{
	ClearAllRacers();
	Super::Deinitialize();
}

void UMGAIRacerSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Cleanup any destroyed racers
	CleanupDestroyedRacers();
}

// ==========================================
// CONFIGURATION
// ==========================================

void UMGAIRacerSubsystem::SetDriverRoster(UMGAIDriverRoster* Roster)
{
	DriverRoster = Roster;
}

void UMGAIRacerSubsystem::SetVehicleClass(TSubclassOf<APawn> VehicleClass)
{
	AIVehicleClass = VehicleClass;
}

void UMGAIRacerSubsystem::SetRacingLine(const TArray<FMGAIRacingLinePoint>& RacingLine)
{
	RacingLinePoints = RacingLine;
}

void UMGAIRacerSubsystem::SetSpawnTransforms(const TArray<FTransform>& Transforms)
{
	GridSpawnTransforms = Transforms;
}

// ==========================================
// SPAWNING
// ==========================================

void UMGAIRacerSubsystem::SpawnAIRacers(const FMGAISpawnConfig& Config)
{
	// Clear existing racers
	ClearAllRacers();

	CurrentConfig = Config;
	bAllSpawned = false;

	// Select drivers
	TArray<UMGAIDriverProfile*> SelectedDrivers = SelectDriversForRace(Config);

	// Assign grid positions
	AssignGridPositions(SelectedDrivers);

	// Spawn each racer
	for (int32 i = 0; i < SelectedDrivers.Num(); i++)
	{
		UMGAIDriverProfile* Driver = SelectedDrivers[i];
		int32 GridPosition = i + 2; // Position 1 is for player

		FTransform SpawnTransform = GetSpawnTransformForPosition(GridPosition);
		FMGAIRacerInfo RacerInfo = SpawnSingleRacer(Driver, SpawnTransform, GridPosition);

		if (RacerInfo.Controller)
		{
			// Configure based on config
			RacerInfo.Controller->SetDifficultyMultiplier(Config.DifficultyModifier);

			// Use skill-based catch-up (NOT rubber banding - per GDD Pillar 5)
			// Legacy bEnableRubberBanding is mapped to the new system
			bool bEnableCatchUp = Config.bEnableSkillBasedCatchUp || Config.bEnableRubberBanding;
			RacerInfo.Controller->SetSkillBasedCatchUpEnabled(bEnableCatchUp);

			// Set racing line
			if (RacingLinePoints.Num() > 0)
			{
				RacerInfo.Controller->SetRacingLine(RacingLinePoints);
			}

			ActiveRacers.Add(RacerInfo);
			OnAIRacerSpawned.Broadcast(RacerInfo);
		}
	}

	bAllSpawned = true;
	OnAllAIRacersSpawned.Broadcast();
}

FMGAIRacerInfo UMGAIRacerSubsystem::SpawnSingleRacer(UMGAIDriverProfile* Profile, const FTransform& SpawnTransform, int32 GridPosition)
{
	FMGAIRacerInfo Info;
	Info.Profile = Profile;
	Info.GridPosition = GridPosition;
	Info.RacePosition = GridPosition;

	UWorld* World = GetWorld();
	if (!World || !AIVehicleClass || !AIControllerClass)
	{
		return Info;
	}

	// Spawn vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* Vehicle = World->SpawnActor<APawn>(AIVehicleClass, SpawnTransform, SpawnParams);
	if (!Vehicle)
	{
		return Info;
	}

	Info.Vehicle = Vehicle;
	Info.VehicleID = SelectVehicleForDriver(Profile, CurrentConfig);

	// Spawn controller
	AMGAIRacerController* Controller = World->SpawnActor<AMGAIRacerController>(AIControllerClass);
	if (!Controller)
	{
		Vehicle->Destroy();
		Info.Vehicle = nullptr;
		return Info;
	}

	Info.Controller = Controller;
	Info.bIsActive = true;

	// Configure controller
	Controller->SetDriverProfile(Profile);
	Controller->Possess(Vehicle);

	// Apply profile difficulty modifier
	if (Profile)
	{
		Profile->ApplyDifficultyModifier(CurrentConfig.DifficultyModifier);
	}

	return Info;
}

void UMGAIRacerSubsystem::ClearAllRacers()
{
	for (FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller)
		{
			if (Racer.Vehicle)
			{
				Racer.Controller->UnPossess();
				Racer.Vehicle->Destroy();
			}
			Racer.Controller->Destroy();

			OnAIRacerRemoved.Broadcast(Racer);
		}
	}

	ActiveRacers.Empty();
	bAllSpawned = false;

	OnAllAIRacersCleared.Broadcast();
}

void UMGAIRacerSubsystem::RemoveRacer(AMGAIRacerController* Controller)
{
	for (int32 i = 0; i < ActiveRacers.Num(); i++)
	{
		if (ActiveRacers[i].Controller == Controller)
		{
			FMGAIRacerInfo Racer = ActiveRacers[i];

			if (Racer.Vehicle)
			{
				Controller->UnPossess();
				Racer.Vehicle->Destroy();
			}
			Controller->Destroy();

			ActiveRacers.RemoveAt(i);
			OnAIRacerRemoved.Broadcast(Racer);
			return;
		}
	}
}

// ==========================================
// CONTROL
// ==========================================

void UMGAIRacerSubsystem::StartAllRacing()
{
	for (FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller && Racer.bIsActive)
		{
			Racer.Controller->StartRacing();
		}
	}

	bAIPaused = false;
}

void UMGAIRacerSubsystem::StopAllRacing()
{
	for (FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller)
		{
			Racer.Controller->StopRacing();
		}
	}
}

void UMGAIRacerSubsystem::PauseAllAI(bool bPause)
{
	bAIPaused = bPause;

	for (FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller)
		{
			Racer.Controller->ForceState(bPause ? EMGAIDrivingState::Waiting : EMGAIDrivingState::Racing);
		}
	}
}

void UMGAIRacerSubsystem::SetAllDifficulty(float DifficultyMultiplier)
{
	for (FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller)
		{
			Racer.Controller->SetDifficultyMultiplier(DifficultyMultiplier);
		}
	}
}

void UMGAIRacerSubsystem::SetAllSkillBasedCatchUp(bool bEnabled)
{
	for (FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller)
		{
			Racer.Controller->SetSkillBasedCatchUpEnabled(bEnabled);
		}
	}
}

void UMGAIRacerSubsystem::SetAllRubberBanding(bool bEnabled, float Strength)
{
	// Note: Strength parameter is deprecated - skill-based catch-up uses
	// fixed behavior based on race situation (per GDD Pillar 5: Unified Challenge)
	SetAllSkillBasedCatchUp(bEnabled);
}

// ==========================================
// QUERIES
// ==========================================

FMGAIRacerInfo UMGAIRacerSubsystem::GetRacerByIndex(int32 Index) const
{
	if (Index >= 0 && Index < ActiveRacers.Num())
	{
		return ActiveRacers[Index];
	}
	return FMGAIRacerInfo();
}

FMGAIRacerInfo UMGAIRacerSubsystem::GetRacerByController(AMGAIRacerController* Controller) const
{
	for (const FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller == Controller)
		{
			return Racer;
		}
	}
	return FMGAIRacerInfo();
}

FMGAIRacerInfo UMGAIRacerSubsystem::GetRacerInPosition(int32 Position) const
{
	for (const FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.RacePosition == Position)
		{
			return Racer;
		}
	}
	return FMGAIRacerInfo();
}

bool UMGAIRacerSubsystem::IsAnyRacing() const
{
	for (const FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.Controller && Racer.bIsActive)
		{
			EMGAIDrivingState State = Racer.Controller->GetDrivingState();
			if (State != EMGAIDrivingState::Waiting && State != EMGAIDrivingState::Finished)
			{
				return true;
			}
		}
	}
	return false;
}

// ==========================================
// POSITION TRACKING
// ==========================================

void UMGAIRacerSubsystem::UpdateRacePositions(const TArray<int32>& Positions)
{
	// Positions array maps racer index to race position
	for (int32 i = 0; i < FMath::Min(Positions.Num(), ActiveRacers.Num()); i++)
	{
		ActiveRacers[i].RacePosition = Positions[i];
	}
}

FMGAIRacerInfo UMGAIRacerSubsystem::GetLeadingAI() const
{
	FMGAIRacerInfo Leading;
	int32 BestPosition = INT_MAX;

	for (const FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.bIsActive && Racer.RacePosition < BestPosition)
		{
			BestPosition = Racer.RacePosition;
			Leading = Racer;
		}
	}

	return Leading;
}

FMGAIRacerInfo UMGAIRacerSubsystem::GetClosestToPlayer() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return FMGAIRacerInfo();
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC || !PC->GetPawn())
	{
		return FMGAIRacerInfo();
	}

	FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
	FMGAIRacerInfo Closest;
	float ClosestDist = FLT_MAX;

	for (const FMGAIRacerInfo& Racer : ActiveRacers)
	{
		if (Racer.bIsActive && Racer.Vehicle)
		{
			float Dist = FVector::Dist(PlayerLocation, Racer.Vehicle->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				Closest = Racer;
			}
		}
	}

	return Closest;
}

// ==========================================
// INTERNAL
// ==========================================

TArray<UMGAIDriverProfile*> UMGAIRacerSubsystem::SelectDriversForRace(const FMGAISpawnConfig& Config)
{
	TArray<UMGAIDriverProfile*> Selected;

	// Add required drivers first
	for (UMGAIDriverProfile* Required : Config.RequiredDrivers)
	{
		if (Required && Selected.Num() < Config.RacerCount)
		{
			Selected.Add(Required);
		}
	}

	// Fill remaining with random drivers from roster
	if (DriverRoster)
	{
		int32 Remaining = Config.RacerCount - Selected.Num();
		if (Remaining > 0)
		{
			TArray<UMGAIDriverProfile*> Random = DriverRoster->GetRandomDrivers(
				Remaining,
				Config.MinSkill,
				Config.MaxSkill
			);

			// Avoid duplicates
			for (UMGAIDriverProfile* Driver : Random)
			{
				if (!Selected.Contains(Driver))
				{
					Selected.Add(Driver);
				}
			}
		}
	}

	// If still not enough, create placeholder profiles
	while (Selected.Num() < Config.RacerCount)
	{
		// Would create default profiles here
		Selected.Add(nullptr);
	}

	return Selected;
}

void UMGAIRacerSubsystem::AssignGridPositions(TArray<UMGAIDriverProfile*>& Drivers)
{
	// Shuffle for random grid (could also use skill-based ordering)
	for (int32 i = Drivers.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		Drivers.Swap(i, j);
	}
}

FName UMGAIRacerSubsystem::SelectVehicleForDriver(UMGAIDriverProfile* Driver, const FMGAISpawnConfig& Config)
{
	if (Driver && !Driver->PreferredVehicle.IsNone())
	{
		// Check if class matches restriction
		if (Config.VehicleClassRestriction.IsNone() ||
			Driver->PreferredVehicleClass == Config.VehicleClassRestriction)
		{
			return Driver->PreferredVehicle;
		}
	}

	// Return default vehicle for class
	// Would query vehicle content system here
	return FName("DefaultVehicle");
}

FTransform UMGAIRacerSubsystem::GetSpawnTransformForPosition(int32 GridPosition) const
{
	if (GridPosition > 0 && GridPosition <= GridSpawnTransforms.Num())
	{
		return GridSpawnTransforms[GridPosition - 1];
	}

	// Generate default transform if not provided
	// Standard 2-wide grid pattern
	int32 Row = (GridPosition - 1) / 2;
	int32 Col = (GridPosition - 1) % 2;

	FVector Location = FVector(-Row * 800.0f, Col * 400.0f - 200.0f, 0.0f);
	FRotator Rotation = FRotator::ZeroRotator;

	return FTransform(Rotation, Location);
}

void UMGAIRacerSubsystem::CleanupDestroyedRacers()
{
	for (int32 i = ActiveRacers.Num() - 1; i >= 0; i--)
	{
		FMGAIRacerInfo& Racer = ActiveRacers[i];

		// Check if vehicle was destroyed
		if (!IsValid(Racer.Vehicle) || !IsValid(Racer.Controller))
		{
			Racer.bIsActive = false;

			// Clean up remaining references
			if (IsValid(Racer.Controller))
			{
				Racer.Controller->Destroy();
			}
			if (IsValid(Racer.Vehicle))
			{
				Racer.Vehicle->Destroy();
			}

			OnAIRacerRemoved.Broadcast(Racer);
			ActiveRacers.RemoveAt(i);
		}
	}
}

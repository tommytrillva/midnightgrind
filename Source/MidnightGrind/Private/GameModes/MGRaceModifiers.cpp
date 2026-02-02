// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/MGRaceModifiers.h"
#include "GameModes/MGRaceGameMode.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MG_VHCL_MovementComponent.h"
#include "GameFramework/Controller.h"
#include "Engine/World.h"

// ==========================================
// BASE MODIFIER
// ==========================================

UMGRaceModifier::UMGRaceModifier()
{
	ModifierID = NAME_None;
	DisplayName = FText::FromString(TEXT("Modifier"));
	Description = FText::GetEmpty();
	ShortDescription = FText::GetEmpty();
	Icon = nullptr;
	Category = EMGModifierCategory::Rules;
	Severity = EMGModifierSeverity::Moderate;
	bAllowedInRanked = false;
	bAllowedInMultiplayer = true;
	XPMultiplier = 1.0f;
	CashMultiplier = 1.0f;
}

void UMGRaceModifier::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	CachedGameMode = GameMode;
	bIsActive = true;
}

void UMGRaceModifier::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	bIsActive = false;
	CachedGameMode = nullptr;
}

void UMGRaceModifier::OnTick_Implementation(float MGDeltaTime)
{
	// Base implementation does nothing
}

void UMGRaceModifier::OnRaceStarted_Implementation()
{
	// Base implementation does nothing
}

void UMGRaceModifier::OnRaceEnded_Implementation()
{
	// Base implementation does nothing
}

void UMGRaceModifier::OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller)
{
	// Base implementation does nothing
}

void UMGRaceModifier::OnLapCompleted_Implementation(AController* Controller, int32 LapNumber)
{
	// Base implementation does nothing
}

bool UMGRaceModifier::IsCompatibleWith(const UMGRaceModifier* Other) const
{
	if (!Other)
	{
		return true;
	}

	// Check if either modifier lists the other as incompatible
	if (IncompatibleModifiers.Contains(Other->ModifierID))
	{
		return false;
	}

	if (Other->IncompatibleModifiers.Contains(ModifierID))
	{
		return false;
	}

	return true;
}

// ==========================================
// NO NOS MODIFIER
// ==========================================

UMGModifier_NoNOS::UMGModifier_NoNOS()
{
	ModifierID = TEXT("NoNOS");
	DisplayName = FText::FromString(TEXT("No Nitrous"));
	Description = FText::FromString(TEXT("Nitrous oxide is disabled for all vehicles. Pure driving skill only."));
	ShortDescription = FText::FromString(TEXT("NOS Disabled"));
	Category = EMGModifierCategory::Challenge;
	Severity = EMGModifierSeverity::Moderate;
	XPMultiplier = 1.1f;
	IncompatibleModifiers.Add(TEXT("UnlimitedNOS"));
}

void UMGModifier_NoNOS::OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller)
{
	Super::OnVehicleSpawned_Implementation(Vehicle, Controller);

	if (Vehicle)
	{
		// Disable NOS on vehicle
		// Vehicle->SetNOSEnabled(false);
	}
}

// ==========================================
// UNLIMITED NOS MODIFIER
// ==========================================

UMGModifier_UnlimitedNOS::UMGModifier_UnlimitedNOS()
{
	ModifierID = TEXT("UnlimitedNOS");
	DisplayName = FText::FromString(TEXT("Unlimited Nitrous"));
	Description = FText::FromString(TEXT("Nitrous never runs out. Boost to your heart's content!"));
	ShortDescription = FText::FromString(TEXT("Infinite NOS"));
	Category = EMGModifierCategory::Party;
	Severity = EMGModifierSeverity::Major;
	XPMultiplier = 0.8f;
	CashMultiplier = 0.8f;
	IncompatibleModifiers.Add(TEXT("NoNOS"));
}

void UMGModifier_UnlimitedNOS::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	if (!CachedGameMode.IsValid())
	{
		return;
	}

	// Refill NOS for all vehicles
	TArray<FMGRacerState> Racers = CachedGameMode->GetAllRacers();
	for (const FMGRacerState& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(Racer.Vehicle))
		{
			// Vehicle->SetNOSAmount(Vehicle->GetMaxNOSAmount());
		}
	}
}

// ==========================================
// GHOST MODE MODIFIER
// ==========================================

UMGModifier_GhostMode::UMGModifier_GhostMode()
{
	ModifierID = TEXT("GhostMode");
	DisplayName = FText::FromString(TEXT("Ghost Mode"));
	Description = FText::FromString(TEXT("Vehicles pass through each other. No collisions between racers."));
	ShortDescription = FText::FromString(TEXT("No Collisions"));
	Category = EMGModifierCategory::Physics;
	Severity = EMGModifierSeverity::Major;
	XPMultiplier = 0.9f;
	bAllowedInRanked = false;
	IncompatibleModifiers.Add(TEXT("OneHitKO"));
}

void UMGModifier_GhostMode::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Set all existing vehicles to ghost
	TArray<FMGRacerState> Racers = GameMode->GetAllRacers();
	for (const FMGRacerState& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(Racer.Vehicle))
		{
			// Vehicle->SetGhostMode(true);
		}
	}
}

void UMGModifier_GhostMode::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	// Disable ghost mode on all vehicles
	TArray<FMGRacerState> Racers = GameMode->GetAllRacers();
	for (const FMGRacerState& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(Racer.Vehicle))
		{
			// Vehicle->SetGhostMode(false);
		}
	}

	Super::OnDeactivated_Implementation(GameMode);
}

void UMGModifier_GhostMode::OnVehicleSpawned_Implementation(AMGVehiclePawn* Vehicle, AController* Controller)
{
	Super::OnVehicleSpawned_Implementation(Vehicle, Controller);

	if (Vehicle)
	{
		// Vehicle->SetGhostMode(true);
	}
}

// ==========================================
// CATCH UP MODIFIER
// ==========================================

UMGModifier_CatchUp::UMGModifier_CatchUp()
{
	ModifierID = TEXT("CatchUp");
	DisplayName = FText::FromString(TEXT("Catch Up"));
	Description = FText::FromString(TEXT("Trailing racers get speed boost, leaders are slowed. Keeps the pack close!"));
	ShortDescription = FText::FromString(TEXT("Rubber Banding"));
	Category = EMGModifierCategory::Difficulty;
	Severity = EMGModifierSeverity::Moderate;
	MaxSpeedBoost = 1.15f;
	MaxSpeedReduction = 0.95f;
}

void UMGModifier_CatchUp::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	if (!CachedGameMode.IsValid())
	{
		return;
	}

	TArray<FMGRacerState> Racers = CachedGameMode->GetAllRacers();
	int32 NumRacers = Racers.Num();

	if (NumRacers < 2)
	{
		return;
	}

	for (const FMGRacerState& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(Racer.Vehicle))
		{
			// Calculate speed multiplier based on position
			// Position 1 (first) gets reduction, last place gets boost
			float PositionFactor = (float)(Racer.Position - 1) / (float)(NumRacers - 1);
			float SpeedMult = FMath::Lerp(MaxSpeedReduction, MaxSpeedBoost, PositionFactor);

			// Apply to vehicle
			if (UMGVehicleMovementComponent* Movement = Vehicle->GetVehicleMovementComponent())
			{
				Movement->SetSpeedMultiplier(SpeedMult);
			}
		}
	}
}

// ==========================================
// SLIPSTREAM BOOST MODIFIER
// ==========================================

UMGModifier_SlipstreamBoost::UMGModifier_SlipstreamBoost()
{
	ModifierID = TEXT("SlipstreamBoost");
	DisplayName = FText::FromString(TEXT("Super Slipstream"));
	Description = FText::FromString(TEXT("Drafting behind other vehicles provides massive speed boost."));
	ShortDescription = FText::FromString(TEXT("2x Draft"));
	Category = EMGModifierCategory::Speed;
	Severity = EMGModifierSeverity::Moderate;
	SlipstreamMultiplier = 2.0f;
}

void UMGModifier_SlipstreamBoost::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Modify global slipstream settings
	// Would interact with a slipstream subsystem
}

void UMGModifier_SlipstreamBoost::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	// Reset slipstream settings

	Super::OnDeactivated_Implementation(GameMode);
}

// ==========================================
// ONE HIT KO MODIFIER
// ==========================================

UMGModifier_OneHitKO::UMGModifier_OneHitKO()
{
	ModifierID = TEXT("OneHitKO");
	DisplayName = FText::FromString(TEXT("One Hit KO"));
	Description = FText::FromString(TEXT("Any significant collision eliminates the vehicle from the race."));
	ShortDescription = FText::FromString(TEXT("Fragile"));
	Category = EMGModifierCategory::Challenge;
	Severity = EMGModifierSeverity::Extreme;
	XPMultiplier = 1.5f;
	CashMultiplier = 1.3f;
	bAllowedInRanked = false;
	MinKOImpactForce = 50.0f;
	IncompatibleModifiers.Add(TEXT("GhostMode"));
}

void UMGModifier_OneHitKO::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Bind to collision events on all vehicles
	TArray<FMGRacerState> Racers = GameMode->GetAllRacers();
	for (const FMGRacerState& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(Racer.Vehicle))
		{
			// Vehicle->OnCollision.AddDynamic(this, &UMGModifier_OneHitKO::OnVehicleCollision);
		}
	}
}

void UMGModifier_OneHitKO::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	// Unbind from collision events

	Super::OnDeactivated_Implementation(GameMode);
}

void UMGModifier_OneHitKO::OnVehicleCollision(AMGVehiclePawn* Vehicle, AActor* OtherActor, float ImpactForce)
{
	if (ImpactForce >= MinKOImpactForce && Vehicle)
	{
		// Eliminate vehicle
		// Vehicle->Eliminate();
	}
}

// ==========================================
// ELIMINATION MODIFIER
// ==========================================

UMGModifier_Elimination::UMGModifier_Elimination()
{
	ModifierID = TEXT("Elimination");
	DisplayName = FText::FromString(TEXT("Elimination"));
	Description = FText::FromString(TEXT("Last place is eliminated at the end of each lap. Survive to win!"));
	ShortDescription = FText::FromString(TEXT("Last Out"));
	Category = EMGModifierCategory::Rules;
	Severity = EMGModifierSeverity::Major;
	XPMultiplier = 1.25f;
	CashMultiplier = 1.2f;
}

void UMGModifier_Elimination::OnLapCompleted_Implementation(AController* Controller, int32 LapNumber)
{
	Super::OnLapCompleted_Implementation(Controller, LapNumber);

	if (!CachedGameMode.IsValid())
	{
		return;
	}

	// Find last place racer and eliminate them
	TArray<FMGRacerState> Racers = CachedGameMode->GetAllRacers();
	int32 LastPosition = 0;
	AController* LastPlaceController = nullptr;

	for (const FMGRacerState& Racer : Racers)
	{
		if (!EliminatedRacers.Contains(Racer.Controller) && Racer.Position > LastPosition)
		{
			LastPosition = Racer.Position;
			LastPlaceController = Racer.Controller;
		}
	}

	if (LastPlaceController && !EliminatedRacers.Contains(LastPlaceController))
	{
		EliminatedRacers.Add(LastPlaceController);
		// Eliminate the racer
		// CachedGameMode->EliminateRacer(LastPlaceController);
	}
}

void UMGModifier_Elimination::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	// Could add periodic elimination check for time-based elimination variant
}

// ==========================================
// MIRROR MODE MODIFIER
// ==========================================

UMGModifier_MirrorMode::UMGModifier_MirrorMode()
{
	ModifierID = TEXT("MirrorMode");
	DisplayName = FText::FromString(TEXT("Mirror Mode"));
	Description = FText::FromString(TEXT("The track layout is mirrored. Left becomes right!"));
	ShortDescription = FText::FromString(TEXT("Mirrored"));
	Category = EMGModifierCategory::Visual;
	Severity = EMGModifierSeverity::Moderate;
	XPMultiplier = 1.1f;
}

void UMGModifier_MirrorMode::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Mirror the track (scale X by -1)
	// Would interact with track subsystem
}

void UMGModifier_MirrorMode::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	// Unmirror the track

	Super::OnDeactivated_Implementation(GameMode);
}

// ==========================================
// TIME ATTACK MODIFIER
// ==========================================

UMGModifier_TimeAttack::UMGModifier_TimeAttack()
{
	ModifierID = TEXT("TimeAttack");
	DisplayName = FText::FromString(TEXT("Time Attack"));
	Description = FText::FromString(TEXT("Race against the clock! Checkpoints add time. Run out and you're done."));
	ShortDescription = FText::FromString(TEXT("vs Clock"));
	Category = EMGModifierCategory::Rules;
	Severity = EMGModifierSeverity::Major;
	StartingTime = 60.0f;
	TimePerCheckpoint = 10.0f;
}

void UMGModifier_TimeAttack::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Initialize time for all racers
	TArray<FMGRacerState> Racers = GameMode->GetAllRacers();
	for (const FMGRacerState& Racer : Racers)
	{
		RacerTimes.Add(Racer.Controller, StartingTime);
	}
}

void UMGModifier_TimeAttack::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	// Decrement time for all racers
	TArray<AController*> ToEliminate;

	for (auto& Pair : RacerTimes)
	{
		Pair.Value -= DeltaTime;
		if (Pair.Value <= 0.0f)
		{
			ToEliminate.Add(Pair.Key);
		}
	}

	// Eliminate racers who ran out of time
	for (AController* Controller : ToEliminate)
	{
		RacerTimes.Remove(Controller);
		// CachedGameMode->EliminateRacer(Controller);
	}
}

// ==========================================
// NIGHT VISION MODIFIER
// ==========================================

UMGModifier_NightVision::UMGModifier_NightVision()
{
	ModifierID = TEXT("NightVision");
	DisplayName = FText::FromString(TEXT("Blackout"));
	Description = FText::FromString(TEXT("Race in near total darkness with limited visibility. Headlights only!"));
	ShortDescription = FText::FromString(TEXT("Dark"));
	Category = EMGModifierCategory::Visual;
	Severity = EMGModifierSeverity::Major;
	XPMultiplier = 1.2f;
	VisibilityDistance = 50.0f;
}

void UMGModifier_NightVision::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Apply heavy fog and darkness
	// Would interact with weather subsystem
}

void UMGModifier_NightVision::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	// Remove darkness effect

	Super::OnDeactivated_Implementation(GameMode);
}

// ==========================================
// TRAFFIC MODIFIER
// ==========================================

UMGModifier_Traffic::UMGModifier_Traffic()
{
	ModifierID = TEXT("Traffic");
	DisplayName = FText::FromString(TEXT("Traffic"));
	Description = FText::FromString(TEXT("Civilian traffic vehicles populate the track. Don't crash!"));
	ShortDescription = FText::FromString(TEXT("Traffic"));
	Category = EMGModifierCategory::Challenge;
	Severity = EMGModifierSeverity::Moderate;
	XPMultiplier = 1.15f;
	CashMultiplier = 1.1f;
	TrafficDensity = 5.0f;
}

void UMGModifier_Traffic::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	// Spawn traffic vehicles along track
	// Would use traffic spawning system
}

void UMGModifier_Traffic::OnDeactivated_Implementation(AMGRaceGameMode* GameMode)
{
	// Despawn all traffic
	for (AActor* Traffic : SpawnedTraffic)
	{
		if (Traffic)
		{
			Traffic->Destroy();
		}
	}
	SpawnedTraffic.Empty();

	Super::OnDeactivated_Implementation(GameMode);
}

void UMGModifier_Traffic::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	// Update traffic AI
}

// ==========================================
// DRIFT ONLY MODIFIER
// ==========================================

UMGModifier_DriftOnly::UMGModifier_DriftOnly()
{
	ModifierID = TEXT("DriftOnly");
	DisplayName = FText::FromString(TEXT("Drift Only"));
	Description = FText::FromString(TEXT("You can only gain position while actively drifting. Style matters!"));
	ShortDescription = FText::FromString(TEXT("Drift 2 Pass"));
	Category = EMGModifierCategory::Rules;
	Severity = EMGModifierSeverity::Major;
	XPMultiplier = 1.3f;
	MinDriftAngle = 15.0f;
}

void UMGModifier_DriftOnly::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	if (!CachedGameMode.IsValid())
	{
		return;
	}

	// Check each vehicle - if not drifting and trying to pass, slow them down
	TArray<FMGRacerState> Racers = CachedGameMode->GetAllRacers();
	for (const FMGRacerState& Racer : Racers)
	{
		if (AMGVehiclePawn* Vehicle = Cast<AMGVehiclePawn>(Racer.Vehicle))
		{
			UMGVehicleMovementComponent* Movement = Vehicle->GetVehicleMovementComponent();
			if (Movement)
			{
				FMGDriftState DriftState = Movement->GetDriftState();
				bool bIsDrifting = FMath::Abs(DriftState.DriftAngle) >= MinDriftAngle;

				// If not drifting, apply speed limiter when near other vehicles
				// This is simplified - full implementation would check proximity to others
			}
		}
	}
}

// ==========================================
// RANDOM EVENTS MODIFIER
// ==========================================

UMGModifier_RandomEvents::UMGModifier_RandomEvents()
{
	ModifierID = TEXT("RandomEvents");
	DisplayName = FText::FromString(TEXT("Chaos Mode"));
	Description = FText::FromString(TEXT("Random events occur throughout the race. Expect the unexpected!"));
	ShortDescription = FText::FromString(TEXT("Chaos"));
	Category = EMGModifierCategory::Party;
	Severity = EMGModifierSeverity::Major;
	XPMultiplier = 1.1f;
	bAllowedInRanked = false;
	MinEventInterval = 15.0f;
	MaxEventInterval = 45.0f;
}

void UMGModifier_RandomEvents::OnActivated_Implementation(AMGRaceGameMode* GameMode)
{
	Super::OnActivated_Implementation(GameMode);

	NextEventTime = FMath::RandRange(MinEventInterval, MaxEventInterval);
	EventTimer = 0.0f;
}

void UMGModifier_RandomEvents::OnTick_Implementation(float MGDeltaTime)
{
	Super::OnTick_Implementation(DeltaTime);

	EventTimer += DeltaTime;

	if (EventTimer >= NextEventTime)
	{
		TriggerRandomEvent();
		EventTimer = 0.0f;
		NextEventTime = FMath::RandRange(MinEventInterval, MaxEventInterval);
	}
}

void UMGModifier_RandomEvents::TriggerRandomEvent()
{
	// Random event types:
	// - Speed boost for random racer
	// - Temporary slowdown for leader
	// - Reverse controls for 5 seconds
	// - Everyone gets NOS
	// - Fog rolls in
	// - Position shuffle

	int32 EventType = FMath::RandRange(0, 5);

	switch (EventType)
	{
	case 0:
		// Speed boost
		break;
	case 1:
		// Leader slowdown
		break;
	case 2:
		// NOS for all
		break;
	case 3:
		// Weather change
		break;
	case 4:
		// Position shuffle (swap two random racers)
		break;
	case 5:
		// Shortcut opens temporarily
		break;
	}
}

// ==========================================
// MODIFIER MANAGER
// ==========================================

UMGRaceModifierManager::UMGRaceModifierManager()
{
	// Register default modifiers
	RegisteredModifiers.Add(UMGModifier_NoNOS::StaticClass());
	RegisteredModifiers.Add(UMGModifier_UnlimitedNOS::StaticClass());
	RegisteredModifiers.Add(UMGModifier_GhostMode::StaticClass());
	RegisteredModifiers.Add(UMGModifier_CatchUp::StaticClass());
	RegisteredModifiers.Add(UMGModifier_SlipstreamBoost::StaticClass());
	RegisteredModifiers.Add(UMGModifier_OneHitKO::StaticClass());
	RegisteredModifiers.Add(UMGModifier_Elimination::StaticClass());
	RegisteredModifiers.Add(UMGModifier_MirrorMode::StaticClass());
	RegisteredModifiers.Add(UMGModifier_TimeAttack::StaticClass());
	RegisteredModifiers.Add(UMGModifier_NightVision::StaticClass());
	RegisteredModifiers.Add(UMGModifier_Traffic::StaticClass());
	RegisteredModifiers.Add(UMGModifier_DriftOnly::StaticClass());
	RegisteredModifiers.Add(UMGModifier_RandomEvents::StaticClass());
}

void UMGRaceModifierManager::Initialize(AMGRaceGameMode* GameMode)
{
	GameModeRef = GameMode;
}

bool UMGRaceModifierManager::ActivateModifier(FName ModifierID)
{
	if (!GameModeRef.IsValid())
	{
		return false;
	}

	if (ActiveModifiers.Contains(ModifierID))
	{
		return false; // Already active
	}

	if (!CanActivateModifier(ModifierID))
	{
		return false;
	}

	// Find and create modifier
	for (TSubclassOf<UMGRaceModifier> ModClass : RegisteredModifiers)
	{
		if (ModClass)
		{
			UMGRaceModifier* DefaultMod = ModClass->GetDefaultObject<UMGRaceModifier>();
			if (DefaultMod && DefaultMod->ModifierID == ModifierID)
			{
				UMGRaceModifier* NewMod = CreateModifier(ModClass);
				if (NewMod)
				{
					ActiveModifiers.Add(ModifierID, NewMod);
					NewMod->OnActivated(GameModeRef.Get());
					return true;
				}
			}
		}
	}

	return false;
}

bool UMGRaceModifierManager::DeactivateModifier(FName ModifierID)
{
	UMGRaceModifier* Modifier = nullptr;
	if (ActiveModifiers.RemoveAndCopyValue(ModifierID, Modifier))
	{
		if (Modifier && GameModeRef.IsValid())
		{
			Modifier->OnDeactivated(GameModeRef.Get());
		}
		return true;
	}
	return false;
}

void UMGRaceModifierManager::DeactivateAllModifiers()
{
	TArray<FName> ModifierIDs;
	ActiveModifiers.GetKeys(ModifierIDs);

	for (FName ID : ModifierIDs)
	{
		DeactivateModifier(ID);
	}
}

bool UMGRaceModifierManager::IsModifierActive(FName ModifierID) const
{
	return ActiveModifiers.Contains(ModifierID);
}

TArray<UMGRaceModifier*> UMGRaceModifierManager::GetActiveModifiers() const
{
	TArray<UMGRaceModifier*> Result;
	ActiveModifiers.GenerateValueArray(Result);
	return Result;
}

TArray<TSubclassOf<UMGRaceModifier>> UMGRaceModifierManager::GetAvailableModifiers() const
{
	return RegisteredModifiers;
}

float UMGRaceModifierManager::GetTotalXPMultiplier() const
{
	float Total = 1.0f;
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Total *= Pair.Value->XPMultiplier;
		}
	}
	return Total;
}

float UMGRaceModifierManager::GetTotalCashMultiplier() const
{
	float Total = 1.0f;
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Total *= Pair.Value->CashMultiplier;
		}
	}
	return Total;
}

bool UMGRaceModifierManager::CanActivateModifier(FName ModifierID) const
{
	// Find the modifier to check
	UMGRaceModifier* ModToCheck = nullptr;
	for (TSubclassOf<UMGRaceModifier> ModClass : RegisteredModifiers)
	{
		if (ModClass)
		{
			UMGRaceModifier* DefaultMod = ModClass->GetDefaultObject<UMGRaceModifier>();
			if (DefaultMod && DefaultMod->ModifierID == ModifierID)
			{
				ModToCheck = DefaultMod;
				break;
			}
		}
	}

	if (!ModToCheck)
	{
		return false;
	}

	// Check compatibility with all active modifiers
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value && !ModToCheck->IsCompatibleWith(Pair.Value))
		{
			return false;
		}
	}

	return true;
}

void UMGRaceModifierManager::TickModifiers(float MGDeltaTime)
{
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->OnTick(DeltaTime);
		}
	}
}

void UMGRaceModifierManager::NotifyRaceStarted()
{
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->OnRaceStarted();
		}
	}
}

void UMGRaceModifierManager::NotifyRaceEnded()
{
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->OnRaceEnded();
		}
	}
}

void UMGRaceModifierManager::NotifyVehicleSpawned(AMGVehiclePawn* Vehicle, AController* Controller)
{
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->OnVehicleSpawned(Vehicle, Controller);
		}
	}
}

void UMGRaceModifierManager::NotifyLapCompleted(AController* Controller, int32 LapNumber)
{
	for (const auto& Pair : ActiveModifiers)
	{
		if (Pair.Value)
		{
			Pair.Value->OnLapCompleted(Controller, LapNumber);
		}
	}
}

UMGRaceModifier* UMGRaceModifierManager::CreateModifier(TSubclassOf<UMGRaceModifier> ModifierClass)
{
	if (!ModifierClass)
	{
		return nullptr;
	}

	return NewObject<UMGRaceModifier>(this, ModifierClass);
}

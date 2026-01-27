// Copyright Midnight Grind. All Rights Reserved.

#include "WorldEvents/MGWorldEventsSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGWorldEventsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SpawnSettings = FMGWorldEventSpawnSettings();

	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGWorldEventsSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			EventUpdateHandle,
			[WeakThis]() { if (WeakThis.IsValid()) WeakThis->UpdateEvents(1.0f); },
			1.0f,
			true
		);
	}
}

void UMGWorldEventsSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(EventUpdateHandle);
	}
	Super::Deinitialize();
}

TArray<FMGWorldEvent> UMGWorldEventsSubsystem::GetActiveEvents() const
{
	TArray<FMGWorldEvent> Result;
	for (const FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.State != EMGWorldEventState::Expired &&
			Event.State != EMGWorldEventState::Completed &&
			Event.State != EMGWorldEventState::Failed)
		{
			Result.Add(Event);
		}
	}
	return Result;
}

TArray<FMGWorldEvent> UMGWorldEventsSubsystem::GetNearbyEvents(FVector Location, float Radius) const
{
	TArray<FMGWorldEvent> Result;
	for (const FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.State == EMGWorldEventState::Active ||
			Event.State == EMGWorldEventState::Pending)
		{
			float Distance = FVector::Dist(Location, Event.Location);
			if (Distance <= Radius)
			{
				Result.Add(Event);
			}
		}
	}
	return Result;
}

FMGWorldEvent UMGWorldEventsSubsystem::GetEvent(const FString& EventID) const
{
	for (const FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.EventID == EventID)
		{
			return Event;
		}
	}
	return FMGWorldEvent();
}

bool UMGWorldEventsSubsystem::HasActiveEventOfType(EMGWorldEventType Type) const
{
	for (const FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.Type == Type &&
			(Event.State == EMGWorldEventState::Active ||
			 Event.State == EMGWorldEventState::PlayerEngaged))
		{
			return true;
		}
	}
	return false;
}

bool UMGWorldEventsSubsystem::JoinEvent(const FString& EventID)
{
	for (FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.EventID == EventID)
		{
			if (Event.State == EMGWorldEventState::Active ||
				Event.State == EMGWorldEventState::Pending)
			{
				Event.State = EMGWorldEventState::PlayerEngaged;
				OnWorldEventStateChanged.Broadcast(Event, Event.State);
				return true;
			}
		}
	}
	return false;
}

void UMGWorldEventsSubsystem::LeaveEvent(const FString& EventID)
{
	for (FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.EventID == EventID)
		{
			if (Event.State == EMGWorldEventState::PlayerEngaged)
			{
				Event.State = EMGWorldEventState::Active;
				OnWorldEventStateChanged.Broadcast(Event, Event.State);
			}
			break;
		}
	}
}

void UMGWorldEventsSubsystem::CompleteEvent(const FString& EventID, bool bSuccess)
{
	for (FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.EventID == EventID)
		{
			Event.State = bSuccess ? EMGWorldEventState::Completed : EMGWorldEventState::Failed;
			OnWorldEventStateChanged.Broadcast(Event, Event.State);
			break;
		}
	}
}

void UMGWorldEventsSubsystem::SpawnEvent(EMGWorldEventType Type, FVector Location)
{
	if (GetActiveEvents().Num() >= SpawnSettings.MaxConcurrentEvents)
		return;

	FMGWorldEvent NewEvent = GenerateRandomEvent(Type, Location);
	NewEvent.State = EMGWorldEventState::Active;

	ActiveEvents.Add(NewEvent);
	OnWorldEventSpawned.Broadcast(NewEvent);
}

void UMGWorldEventsSubsystem::ForceSpawnNearPlayer(EMGWorldEventType Type)
{
	// Would get actual player location
	FVector SpawnLocation = LastPlayerLocation + FVector(
		FMath::RandRange(-500.0f, 500.0f),
		FMath::RandRange(-500.0f, 500.0f),
		0.0f
	);

	SpawnEvent(Type, SpawnLocation);
}

void UMGWorldEventsSubsystem::SetSpawnSettings(const FMGWorldEventSpawnSettings& Settings)
{
	SpawnSettings = Settings;
}

void UMGWorldEventsSubsystem::JoinStreetMeet(const FString& MeetID)
{
	for (FMGStreetMeet& Meet : ActiveStreetMeets)
	{
		if (Meet.MeetID == MeetID)
		{
			if (Meet.CurrentAttendees < Meet.MaxAttendees)
			{
				Meet.CurrentAttendees++;
			}
			break;
		}
	}
}

void UMGWorldEventsSubsystem::LeaveStreetMeet(const FString& MeetID)
{
	for (FMGStreetMeet& Meet : ActiveStreetMeets)
	{
		if (Meet.MeetID == MeetID)
		{
			Meet.CurrentAttendees = FMath::Max(0, Meet.CurrentAttendees - 1);
			break;
		}
	}
}

void UMGWorldEventsSubsystem::TriggerPoliceChase(int32 InitialHeat)
{
	CurrentPoliceEncounter = FMGPoliceEncounter();
	CurrentPoliceEncounter.EncounterID = FGuid::NewGuid().ToString();
	CurrentPoliceEncounter.HeatLevel = FMath::Clamp(InitialHeat, 1, 5);
	CurrentPoliceEncounter.PursuitUnits = InitialHeat;
	CurrentPoliceEncounter.EscapeProgress = 0.0f;

	OnPoliceEncounterStarted.Broadcast(CurrentPoliceEncounter);
}

void UMGWorldEventsSubsystem::IncreaseHeat(int32 Amount)
{
	if (!IsInPoliceChase())
		return;

	CurrentPoliceEncounter.HeatLevel = FMath::Clamp(
		CurrentPoliceEncounter.HeatLevel + Amount, 1, 5);

	// Add pursuit units based on heat
	CurrentPoliceEncounter.PursuitUnits = CurrentPoliceEncounter.HeatLevel * 2;

	// High heat triggers special response
	if (CurrentPoliceEncounter.HeatLevel >= 4)
	{
		CurrentPoliceEncounter.bHelicopterDeployed = true;
	}
	if (CurrentPoliceEncounter.HeatLevel >= 3)
	{
		CurrentPoliceEncounter.bRoadblocksActive = true;
	}
}

void UMGWorldEventsSubsystem::UpdateEscapeProgress(float Progress)
{
	if (!IsInPoliceChase())
		return;

	CurrentPoliceEncounter.EscapeProgress = FMath::Clamp(Progress, 0.0f, 1.0f);

	if (CurrentPoliceEncounter.EscapeProgress >= 1.0f)
	{
		// Escaped!
		CurrentPoliceEncounter.PursuitUnits = 0;
		OnPoliceEscaped.Broadcast();
	}
}

void UMGWorldEventsSubsystem::UpdateEvents(float DeltaTime)
{
	// Update event timers
	for (FMGWorldEvent& Event : ActiveEvents)
	{
		if (Event.State == EMGWorldEventState::Active ||
			Event.State == EMGWorldEventState::Pending)
		{
			Event.TimeRemaining -= DeltaTime;

			if (Event.TimeRemaining <= 0.0f)
			{
				Event.State = EMGWorldEventState::Expired;
				OnWorldEventStateChanged.Broadcast(Event, Event.State);
			}
		}
	}

	// Update street meets
	for (FMGStreetMeet& Meet : ActiveStreetMeets)
	{
		Meet.TimeUntilDispersal -= DeltaTime;
	}

	// Remove dispersed meets
	ActiveStreetMeets.RemoveAll([](const FMGStreetMeet& Meet)
	{
		return Meet.TimeUntilDispersal <= 0.0f;
	});

	// Update police chase
	UpdatePoliceChase(DeltaTime);

	// Try to spawn new events
	TimeSinceLastSpawn += DeltaTime;
	if (TimeSinceLastSpawn >= SpawnSettings.EventSpawnCooldown)
	{
		TrySpawnRandomEvent();
		TimeSinceLastSpawn = 0.0f;
	}

	// Cleanup old events
	CleanupExpiredEvents();
}

void UMGWorldEventsSubsystem::UpdatePoliceChase(float DeltaTime)
{
	if (!IsInPoliceChase())
		return;

	CurrentPoliceEncounter.TimeInPursuit += DeltaTime;

	// Natural escape progress when out of sight (would check actual visibility)
	// This is placeholder logic
	float EscapeGain = 0.01f * DeltaTime; // 1% per second when hidden
	CurrentPoliceEncounter.EscapeProgress = FMath::Min(
		1.0f, CurrentPoliceEncounter.EscapeProgress + EscapeGain);
}

void UMGWorldEventsSubsystem::TrySpawnRandomEvent()
{
	if (GetActiveEvents().Num() >= SpawnSettings.MaxConcurrentEvents)
		return;

	float Roll = FMath::FRand();

	EMGWorldEventType Type = EMGWorldEventType::StreetRace;

	if (Roll < SpawnSettings.PoliceSpawnChance)
	{
		// Don't auto-spawn police, they're triggered by player actions
		return;
	}
	else if (Roll < SpawnSettings.PoliceSpawnChance + SpawnSettings.RivalSpawnChance)
	{
		Type = EMGWorldEventType::RivalAppearance;
	}
	else if (Roll < SpawnSettings.PoliceSpawnChance + SpawnSettings.RivalSpawnChance + SpawnSettings.StreetRaceChance)
	{
		Type = EMGWorldEventType::StreetRace;
	}
	else
	{
		// Random other events
		int32 EventIndex = FMath::RandRange(0, 4);
		switch (EventIndex)
		{
		case 0: Type = EMGWorldEventType::StreetMeet; break;
		case 1: Type = EMGWorldEventType::TimeAttack; break;
		case 2: Type = EMGWorldEventType::HiddenRace; break;
		case 3: Type = EMGWorldEventType::SpecialVehicle; break;
		case 4: Type = EMGWorldEventType::Underground; break;
		}
	}

	ForceSpawnNearPlayer(Type);
}

void UMGWorldEventsSubsystem::CleanupExpiredEvents()
{
	ActiveEvents.RemoveAll([](const FMGWorldEvent& Event)
	{
		return Event.State == EMGWorldEventState::Expired ||
			   Event.State == EMGWorldEventState::Completed ||
			   Event.State == EMGWorldEventState::Failed;
	});
}

FMGWorldEvent UMGWorldEventsSubsystem::GenerateRandomEvent(EMGWorldEventType Type, FVector Location)
{
	FMGWorldEvent Event;
	Event.EventID = FGuid::NewGuid().ToString();
	Event.Type = Type;
	Event.Location = Location;
	Event.State = EMGWorldEventState::Pending;

	switch (Type)
	{
	case EMGWorldEventType::StreetRace:
		Event.DisplayName = FText::FromString(TEXT("Street Challenger"));
		Event.Description = FText::FromString(TEXT("Someone wants to race. Think you can take them?"));
		Event.Duration = 180.0f;
		Event.CashReward = FMath::RandRange(5000, 15000);
		Event.ReputationReward = 50;
		break;

	case EMGWorldEventType::StreetMeet:
		Event.DisplayName = FText::FromString(TEXT("Street Meet"));
		Event.Description = FText::FromString(TEXT("Car enthusiasts gathering nearby."));
		Event.Duration = 600.0f;
		Event.RadiusMeters = 200.0f;
		break;

	case EMGWorldEventType::RivalAppearance:
		Event.DisplayName = FText::FromString(TEXT("Rival Spotted"));
		Event.Description = FText::FromString(TEXT("Your rival is nearby. Time to settle the score."));
		Event.Duration = 120.0f;
		Event.CashReward = 20000;
		Event.ReputationReward = 100;
		break;

	case EMGWorldEventType::HiddenRace:
		Event.DisplayName = FText::FromString(TEXT("Secret Race"));
		Event.Description = FText::FromString(TEXT("An underground race is starting. Invitation only."));
		Event.Duration = 300.0f;
		Event.CashReward = 25000;
		Event.ReputationReward = 150;
		break;

	case EMGWorldEventType::TimeAttack:
		Event.DisplayName = FText::FromString(TEXT("Time Attack"));
		Event.Description = FText::FromString(TEXT("Beat the clock to win big."));
		Event.Duration = 120.0f;
		Event.CashReward = 10000;
		Event.ReputationReward = 30;
		break;

	case EMGWorldEventType::SpecialVehicle:
		Event.DisplayName = FText::FromString(TEXT("Rare Car Sighting"));
		Event.Description = FText::FromString(TEXT("A rare vehicle has been spotted in the area."));
		Event.Duration = 60.0f;
		break;

	case EMGWorldEventType::Underground:
		Event.DisplayName = FText::FromString(TEXT("Underground Meet"));
		Event.Description = FText::FromString(TEXT("The underground scene is active tonight."));
		Event.Duration = 900.0f;
		Event.RadiusMeters = 150.0f;
		break;

	default:
		Event.DisplayName = FText::FromString(TEXT("Event"));
		Event.Duration = 180.0f;
		break;
	}

	Event.TimeRemaining = Event.Duration;
	return Event;
}

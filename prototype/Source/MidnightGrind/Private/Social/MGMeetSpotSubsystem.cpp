// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGMeetSpotSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGMeetSpotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Start update timer (every second)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			UpdateTimer,
			this,
			&UMGMeetSpotSubsystem::OnUpdateTick,
			1.0f,
			true
		);
	}
}

void UMGMeetSpotSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
	}

	Super::Deinitialize();
}

// ==========================================
// INSTANCE MANAGEMENT
// ==========================================

FGuid UMGMeetSpotSubsystem::FindOrCreateInstance(FName MeetSpotID, FGuid PreferredCrewID)
{
	// First, look for instance with crew members if crew specified
	if (PreferredCrewID.IsValid())
	{
		FGuid CrewInstance = FindInstanceWithCrewMembers(MeetSpotID, PreferredCrewID);
		if (CrewInstance.IsValid())
		{
			return CrewInstance;
		}
	}

	// Find instance with available space
	for (const auto& Pair : ActiveInstances)
	{
		const FMGMeetSpotInstance& Instance = Pair.Value;
		if (Instance.MeetSpotID == MeetSpotID &&
			Instance.State == EMGMeetSpotState::Open &&
			Instance.CurrentPlayerCount < Instance.MaxPlayers)
		{
			return Instance.InstanceID;
		}
	}

	// Create new instance
	FMGMeetSpotInstance NewInstance = CreateNewInstance(MeetSpotID);
	ActiveInstances.Add(NewInstance.InstanceID, NewInstance);

	return NewInstance.InstanceID;
}

bool UMGMeetSpotSubsystem::GetInstanceInfo(FGuid InstanceID, FMGMeetSpotInstance& OutInstance) const
{
	if (const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID))
	{
		OutInstance = *Instance;
		return true;
	}
	return false;
}

TArray<FMGMeetSpotInstance> UMGMeetSpotSubsystem::GetActiveInstances(FName MeetSpotID) const
{
	TArray<FMGMeetSpotInstance> Results;

	for (const auto& Pair : ActiveInstances)
	{
		if (Pair.Value.MeetSpotID == MeetSpotID)
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

FGuid UMGMeetSpotSubsystem::FindInstanceWithCrewMembers(FName MeetSpotID, FGuid CrewID) const
{
	for (const auto& Pair : ActiveInstances)
	{
		const FMGMeetSpotInstance& Instance = Pair.Value;
		if (Instance.MeetSpotID != MeetSpotID)
		{
			continue;
		}

		// Check if any crew members are here
		for (const FMGMeetSpotPlayer& Player : Instance.Players)
		{
			if (Player.CrewID == CrewID)
			{
				return Instance.InstanceID;
			}
		}
	}

	return FGuid();
}

// ==========================================
// JOINING & LEAVING
// ==========================================

bool UMGMeetSpotSubsystem::JoinMeetSpot(FGuid PlayerID, FGuid InstanceID, FGuid VehicleID, int32& OutParkingSpot)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	// Check capacity
	if (Instance->CurrentPlayerCount >= Instance->MaxPlayers)
	{
		return false;
	}

	// Check if player already in a meet spot
	if (PlayerInstanceMap.Contains(PlayerID))
	{
		LeaveMeetSpot(PlayerID);
	}

	// Find parking spot
	OutParkingSpot = FindNearestAvailableSpot(InstanceID, FVector::ZeroVector, EMGMeetSpotZone::MainParking);
	if (OutParkingSpot < 0)
	{
		return false;
	}

	// Create player entry
	FMGMeetSpotPlayer NewPlayer;
	NewPlayer.PlayerID = PlayerID;
	NewPlayer.VehicleID = VehicleID;
	NewPlayer.ParkingSpotIndex = OutParkingSpot;
	NewPlayer.JoinTime = FDateTime::Now();

	// TODO: Populate from player profile
	// NewPlayer.DisplayName = ...
	// NewPlayer.CrewID = ...
	// NewPlayer.PerformanceIndex = ...

	// Mark parking spot as occupied
	if (OutParkingSpot >= 0 && OutParkingSpot < Instance->ParkingSpots.Num())
	{
		Instance->ParkingSpots[OutParkingSpot].bOccupied = true;
		Instance->ParkingSpots[OutParkingSpot].OccupantPlayerID = PlayerID;
		Instance->ParkingSpots[OutParkingSpot].OccupantVehicleID = VehicleID;
	}

	Instance->Players.Add(NewPlayer);
	Instance->CurrentPlayerCount++;

	PlayerInstanceMap.Add(PlayerID, InstanceID);

	OnPlayerJoined.Broadcast(InstanceID, NewPlayer);

	return true;
}

void UMGMeetSpotSubsystem::LeaveMeetSpot(FGuid PlayerID)
{
	FGuid* InstanceIDPtr = PlayerInstanceMap.Find(PlayerID);
	if (!InstanceIDPtr)
	{
		return;
	}

	FGuid InstanceID = *InstanceIDPtr;
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		PlayerInstanceMap.Remove(PlayerID);
		return;
	}

	// Find and remove player
	for (int32 i = 0; i < Instance->Players.Num(); ++i)
	{
		if (Instance->Players[i].PlayerID == PlayerID)
		{
			// Free parking spot
			int32 SpotIndex = Instance->Players[i].ParkingSpotIndex;
			if (SpotIndex >= 0 && SpotIndex < Instance->ParkingSpots.Num())
			{
				Instance->ParkingSpots[SpotIndex].bOccupied = false;
				Instance->ParkingSpots[SpotIndex].OccupantPlayerID = FGuid();
				Instance->ParkingSpots[SpotIndex].OccupantVehicleID = FGuid();
			}

			// Remove from showcase queue if present
			Instance->ShowcaseQueue.Remove(PlayerID);

			// If currently showcasing, advance queue
			if (Instance->CurrentShowcasePlayerID == PlayerID)
			{
				AdvanceShowcaseQueue(*Instance);
			}

			Instance->Players.RemoveAt(i);
			Instance->CurrentPlayerCount--;
			break;
		}
	}

	PlayerInstanceMap.Remove(PlayerID);

	OnPlayerLeft.Broadcast(InstanceID, PlayerID);
}

FGuid UMGMeetSpotSubsystem::GetPlayerMeetSpot(FGuid PlayerID) const
{
	const FGuid* InstanceID = PlayerInstanceMap.Find(PlayerID);
	return InstanceID ? *InstanceID : FGuid();
}

bool UMGMeetSpotSubsystem::IsPlayerInMeetSpot(FGuid PlayerID) const
{
	return PlayerInstanceMap.Contains(PlayerID);
}

// ==========================================
// PARKING
// ==========================================

bool UMGMeetSpotSubsystem::RequestParkingSpot(FGuid PlayerID, FGuid InstanceID, int32 SpotIndex)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	if (SpotIndex < 0 || SpotIndex >= Instance->ParkingSpots.Num())
	{
		return false;
	}

	FMGParkingSpot& Spot = Instance->ParkingSpots[SpotIndex];

	if (Spot.bOccupied)
	{
		return false;
	}

	// Check crew reservation
	if (Spot.bReserved && Spot.ReservedForCrewID.IsValid())
	{
		if (!IsCrewMember(PlayerID, Spot.ReservedForCrewID))
		{
			return false;
		}
	}

	// Find player
	FMGMeetSpotPlayer* Player = FindPlayer(InstanceID, PlayerID);
	if (!Player)
	{
		return false;
	}

	// Free old spot
	int32 OldSpot = Player->ParkingSpotIndex;
	if (OldSpot >= 0 && OldSpot < Instance->ParkingSpots.Num())
	{
		Instance->ParkingSpots[OldSpot].bOccupied = false;
		Instance->ParkingSpots[OldSpot].OccupantPlayerID = FGuid();
	}

	// Occupy new spot
	Spot.bOccupied = true;
	Spot.OccupantPlayerID = PlayerID;
	Spot.OccupantVehicleID = Player->VehicleID;
	Player->ParkingSpotIndex = SpotIndex;

	return true;
}

int32 UMGMeetSpotSubsystem::FindNearestAvailableSpot(FGuid InstanceID, FVector PlayerLocation, EMGMeetSpotZone PreferredZone) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return -1;
	}

	int32 BestSpot = -1;
	float BestDistance = FLT_MAX;

	for (int32 i = 0; i < Instance->ParkingSpots.Num(); ++i)
	{
		const FMGParkingSpot& Spot = Instance->ParkingSpots[i];

		if (Spot.bOccupied)
		{
			continue;
		}

		// Skip reserved spots for non-crew members
		if (Spot.bReserved)
		{
			continue;
		}

		// Prefer specified zone
		float ZoneBonus = (Spot.Zone == PreferredZone) ? -10000.0f : 0.0f;

		float Distance = FVector::Dist(PlayerLocation, Spot.Location) + ZoneBonus;

		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			BestSpot = i;
		}
	}

	return BestSpot;
}

void UMGMeetSpotSubsystem::LeaveParkingSpot(FGuid PlayerID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return;
	}

	FMGMeetSpotPlayer* Player = FindPlayer(InstanceID, PlayerID);
	if (!Player)
	{
		return;
	}

	int32 SpotIndex = Player->ParkingSpotIndex;
	if (SpotIndex >= 0 && SpotIndex < Instance->ParkingSpots.Num())
	{
		Instance->ParkingSpots[SpotIndex].bOccupied = false;
		Instance->ParkingSpots[SpotIndex].OccupantPlayerID = FGuid();
		Instance->ParkingSpots[SpotIndex].OccupantVehicleID = FGuid();
	}

	Player->ParkingSpotIndex = -1;
}

TArray<FMGParkingSpot> UMGMeetSpotSubsystem::GetAvailableSpots(FGuid InstanceID, EMGMeetSpotZone Zone) const
{
	TArray<FMGParkingSpot> Results;

	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return Results;
	}

	for (const FMGParkingSpot& Spot : Instance->ParkingSpots)
	{
		if (!Spot.bOccupied && Spot.Zone == Zone && !Spot.bReserved)
		{
			Results.Add(Spot);
		}
	}

	return Results;
}

// ==========================================
// SHOWCASE
// ==========================================

bool UMGMeetSpotSubsystem::JoinShowcaseQueue(FGuid PlayerID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	// Check not already in queue
	if (Instance->ShowcaseQueue.Contains(PlayerID))
	{
		return false;
	}

	Instance->ShowcaseQueue.Add(PlayerID);

	// If no one showcasing, start immediately
	if (!Instance->CurrentShowcasePlayerID.IsValid())
	{
		AdvanceShowcaseQueue(*Instance);
	}

	return true;
}

void UMGMeetSpotSubsystem::LeaveShowcaseQueue(FGuid PlayerID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return;
	}

	Instance->ShowcaseQueue.Remove(PlayerID);

	// If currently showcasing, advance queue
	if (Instance->CurrentShowcasePlayerID == PlayerID)
	{
		AdvanceShowcaseQueue(*Instance);
	}
}

int32 UMGMeetSpotSubsystem::GetShowcaseQueuePosition(FGuid PlayerID) const
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return -1;
	}

	return Instance->ShowcaseQueue.Find(PlayerID);
}

bool UMGMeetSpotSubsystem::VoteForShowcase(FGuid VoterID)
{
	FGuid InstanceID = GetPlayerMeetSpot(VoterID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	if (!Instance->CurrentShowcasePlayerID.IsValid())
	{
		return false;
	}

	// Can't vote for yourself
	if (VoterID == Instance->CurrentShowcasePlayerID)
	{
		return false;
	}

	FMGMeetSpotPlayer* ShowcasePlayer = FindPlayer(InstanceID, Instance->CurrentShowcasePlayerID);
	if (!ShowcasePlayer)
	{
		return false;
	}

	ShowcasePlayer->ShowcaseVotes++;

	OnShowcaseVote.Broadcast(InstanceID, VoterID, ShowcasePlayer->ShowcaseVotes);

	return true;
}

bool UMGMeetSpotSubsystem::GetCurrentShowcase(FGuid InstanceID, FMGMeetSpotPlayer& OutShowcasePlayer) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance || !Instance->CurrentShowcasePlayerID.IsValid())
	{
		return false;
	}

	const FMGMeetSpotPlayer* Player = FindPlayerConst(InstanceID, Instance->CurrentShowcasePlayerID);
	if (Player)
	{
		OutShowcasePlayer = *Player;
		return true;
	}

	return false;
}

void UMGMeetSpotSubsystem::SkipCurrentShowcase(FGuid ModeratorID, FGuid InstanceID)
{
	// TODO: Verify moderator permissions

	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (Instance)
	{
		AdvanceShowcaseQueue(*Instance);
	}
}

// ==========================================
// PHOTO SPOTS
// ==========================================

bool UMGMeetSpotSubsystem::QueueForPhotoSpot(FGuid PlayerID, int32 PhotoSpotIndex)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	if (PhotoSpotIndex < 0 || PhotoSpotIndex >= Instance->PhotoSpots.Num())
	{
		return false;
	}

	TArray<FGuid>& Queue = PhotoSpotQueues.FindOrAdd(PhotoSpotIndex);
	if (!Queue.Contains(PlayerID))
	{
		Queue.Add(PlayerID);
	}

	return true;
}

void UMGMeetSpotSubsystem::LeavePhotoSpotQueue(FGuid PlayerID)
{
	for (auto& Pair : PhotoSpotQueues)
	{
		Pair.Value.Remove(PlayerID);
	}
}

TArray<FMGPhotoSpot> UMGMeetSpotSubsystem::GetPhotoSpots(FGuid InstanceID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (Instance)
	{
		return Instance->PhotoSpots;
	}
	return TArray<FMGPhotoSpot>();
}

void UMGMeetSpotSubsystem::SetPhotoSpotLighting(FGuid InstanceID, int32 SpotIndex, FName LightingPreset)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return;
	}

	if (SpotIndex >= 0 && SpotIndex < Instance->PhotoSpots.Num())
	{
		Instance->PhotoSpots[SpotIndex].LightingPreset = LightingPreset;
	}
}

// ==========================================
// VENDORS
// ==========================================

TArray<FMGVendorInstance> UMGMeetSpotSubsystem::GetVendors(FGuid InstanceID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (Instance)
	{
		return Instance->Vendors;
	}
	return TArray<FMGVendorInstance>();
}

TArray<FMGVendorInstance> UMGMeetSpotSubsystem::GetVendorsByType(FGuid InstanceID, EMGVendorType Type) const
{
	TArray<FMGVendorInstance> Results;

	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return Results;
	}

	for (const FMGVendorInstance& Vendor : Instance->Vendors)
	{
		if (Vendor.Type == Type)
		{
			Results.Add(Vendor);
		}
	}

	return Results;
}

bool UMGMeetSpotSubsystem::InteractWithVendor(FGuid PlayerID, FGuid VendorID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	for (const FMGVendorInstance& Vendor : Instance->Vendors)
	{
		if (Vendor.VendorID == VendorID && Vendor.bAvailable)
		{
			// TODO: Open appropriate shop UI based on vendor type
			return true;
		}
	}

	return false;
}

// ==========================================
// EVENTS
// ==========================================

bool UMGMeetSpotSubsystem::CreateEvent(FGuid OrganizerID, FGuid InstanceID, const FMGMeetSpotEvent& EventData, FGuid& OutEventID)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	// Verify organizer is in this instance
	if (!FindPlayer(InstanceID, OrganizerID))
	{
		return false;
	}

	FMGMeetSpotEvent NewEvent = EventData;
	NewEvent.EventID = FGuid::NewGuid();
	NewEvent.OrganizerID = OrganizerID;

	Instance->UpcomingEvents.Add(NewEvent);
	OutEventID = NewEvent.EventID;

	return true;
}

bool UMGMeetSpotSubsystem::RegisterForEvent(FGuid PlayerID, FGuid EventID)
{
	for (auto& Pair : ActiveInstances)
	{
		for (FMGMeetSpotEvent& Event : Pair.Value.UpcomingEvents)
		{
			if (Event.EventID == EventID)
			{
				if (Event.RegisteredParticipants.Num() >= Event.MaxParticipants)
				{
					return false;
				}

				// Check crew requirement
				if (Event.bCrewOnly && !IsCrewMember(PlayerID, Event.RequiredCrewID))
				{
					return false;
				}

				if (!Event.RegisteredParticipants.Contains(PlayerID))
				{
					Event.RegisteredParticipants.Add(PlayerID);
				}
				return true;
			}
		}
	}

	return false;
}

void UMGMeetSpotSubsystem::UnregisterFromEvent(FGuid PlayerID, FGuid EventID)
{
	for (auto& Pair : ActiveInstances)
	{
		for (FMGMeetSpotEvent& Event : Pair.Value.UpcomingEvents)
		{
			if (Event.EventID == EventID)
			{
				Event.RegisteredParticipants.Remove(PlayerID);
				return;
			}
		}
	}
}

bool UMGMeetSpotSubsystem::StartEvent(FGuid OrganizerID, FGuid EventID)
{
	for (auto& Pair : ActiveInstances)
	{
		FMGMeetSpotInstance& Instance = Pair.Value;
		for (int32 i = 0; i < Instance.UpcomingEvents.Num(); ++i)
		{
			FMGMeetSpotEvent& Event = Instance.UpcomingEvents[i];
			if (Event.EventID == EventID && Event.OrganizerID == OrganizerID)
			{
				Instance.CurrentEvent = Event;
				Instance.State = EMGMeetSpotState::Event;
				Instance.UpcomingEvents.RemoveAt(i);

				OnEventStarted.Broadcast(Instance.InstanceID, Instance.CurrentEvent);
				return true;
			}
		}
	}

	return false;
}

bool UMGMeetSpotSubsystem::EndEvent(FGuid OrganizerID, FGuid EventID)
{
	for (auto& Pair : ActiveInstances)
	{
		FMGMeetSpotInstance& Instance = Pair.Value;
		if (Instance.CurrentEvent.EventID == EventID)
		{
			if (Instance.CurrentEvent.OrganizerID != OrganizerID)
			{
				return false;
			}

			FGuid EndedEventID = Instance.CurrentEvent.EventID;
			Instance.CurrentEvent = FMGMeetSpotEvent();
			Instance.State = EMGMeetSpotState::Open;

			OnEventEnded.Broadcast(Instance.InstanceID, EndedEventID);
			return true;
		}
	}

	return false;
}

TArray<FMGMeetSpotEvent> UMGMeetSpotSubsystem::GetUpcomingEvents(FGuid InstanceID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (Instance)
	{
		return Instance->UpcomingEvents;
	}
	return TArray<FMGMeetSpotEvent>();
}

// ==========================================
// CREW FEATURES
// ==========================================

bool UMGMeetSpotSubsystem::ReserveCrewSpots(FGuid CrewLeaderID, FGuid InstanceID, int32 NumSpots)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	// TODO: Verify crew leader permissions

	// Get crew ID
	const FMGMeetSpotPlayer* Leader = FindPlayerConst(InstanceID, CrewLeaderID);
	if (!Leader || !Leader->CrewID.IsValid())
	{
		return false;
	}

	// Find unreserved spots in crew parking zone
	int32 ReservedCount = 0;
	for (FMGParkingSpot& Spot : Instance->ParkingSpots)
	{
		if (Spot.Zone == EMGMeetSpotZone::CrewParking && !Spot.bReserved && !Spot.bOccupied)
		{
			Spot.bReserved = true;
			Spot.ReservedForCrewID = Leader->CrewID;
			ReservedCount++;

			if (ReservedCount >= NumSpots)
			{
				break;
			}
		}
	}

	Instance->CrewReservedSpots.FindOrAdd(Leader->CrewID) = ReservedCount;

	return ReservedCount > 0;
}

void UMGMeetSpotSubsystem::ReleaseCrewSpots(FGuid CrewLeaderID, FGuid InstanceID)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return;
	}

	const FMGMeetSpotPlayer* Leader = FindPlayerConst(InstanceID, CrewLeaderID);
	if (!Leader || !Leader->CrewID.IsValid())
	{
		return;
	}

	for (FMGParkingSpot& Spot : Instance->ParkingSpots)
	{
		if (Spot.ReservedForCrewID == Leader->CrewID)
		{
			Spot.bReserved = false;
			Spot.ReservedForCrewID = FGuid();
		}
	}

	Instance->CrewReservedSpots.Remove(Leader->CrewID);
}

TArray<FMGMeetSpotPlayer> UMGMeetSpotSubsystem::GetCrewMembersInMeetSpot(FGuid InstanceID, FGuid CrewID) const
{
	TArray<FMGMeetSpotPlayer> Results;

	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return Results;
	}

	for (const FMGMeetSpotPlayer& Player : Instance->Players)
	{
		if (Player.CrewID == CrewID)
		{
			Results.Add(Player);
		}
	}

	return Results;
}

// ==========================================
// RACE ORGANIZATION
// ==========================================

bool UMGMeetSpotSubsystem::ProposeRace(FGuid OrganizerID, FName RaceType, float PILimit, int32 MaxParticipants, int64 EntryFee)
{
	// TODO: Integrate with race management subsystem
	// Create race lobby from meet spot

	return true;
}

bool UMGMeetSpotSubsystem::AcceptRaceProposal(FGuid PlayerID, FGuid RaceProposalID)
{
	// TODO: Add player to race lobby

	return true;
}

bool UMGMeetSpotSubsystem::LaunchRace(FGuid OrganizerID, FGuid RaceProposalID)
{
	// TODO: Transition players to race start

	return true;
}

// ==========================================
// SOCIAL
// ==========================================

void UMGMeetSpotSubsystem::SendProximityMessage(FGuid SenderID, const FString& Message, float Range)
{
	// TODO: Broadcast to players within range
}

void UMGMeetSpotSubsystem::UseEmote(FGuid PlayerID, FName EmoteID)
{
	// TODO: Trigger emote animation
}

void UMGMeetSpotSubsystem::FlashHeadlights(FGuid PlayerID)
{
	// Flash headlights = challenge another player to race
	// TODO: Trigger headlight flash visual
	// TODO: If facing another vehicle, send race challenge
}

void UMGMeetSpotSubsystem::RevEngine(FGuid PlayerID)
{
	// TODO: Trigger engine rev audio
}

// ==========================================
// INTERNAL
// ==========================================

void UMGMeetSpotSubsystem::OnUpdateTick()
{
	UpdateShowcases();
	UpdatePhotoSpots();
	CleanupEmptyInstances();
}

void UMGMeetSpotSubsystem::UpdateShowcases()
{
	FDateTime Now = FDateTime::Now();

	for (auto& Pair : ActiveInstances)
	{
		FMGMeetSpotInstance& Instance = Pair.Value;

		if (Instance.CurrentShowcasePlayerID.IsValid() && Now >= Instance.ShowcaseEndTime)
		{
			AdvanceShowcaseQueue(Instance);
		}
	}
}

void UMGMeetSpotSubsystem::UpdatePhotoSpots()
{
	// Process photo spot queues
	// Advance players when current occupant leaves
}

void UMGMeetSpotSubsystem::CleanupEmptyInstances()
{
	TArray<FGuid> ToRemove;

	for (const auto& Pair : ActiveInstances)
	{
		if (Pair.Value.CurrentPlayerCount == 0)
		{
			ToRemove.Add(Pair.Key);
		}
	}

	for (const FGuid& ID : ToRemove)
	{
		ActiveInstances.Remove(ID);
	}
}

FMGMeetSpotInstance UMGMeetSpotSubsystem::CreateNewInstance(FName MeetSpotID)
{
	FMGMeetSpotInstance Instance;
	Instance.MeetSpotID = MeetSpotID;
	Instance.MaxPlayers = 200; // Per PRD Section 2.1

	SetupDefaultInfrastructure(Instance);

	return Instance;
}

void UMGMeetSpotSubsystem::SetupDefaultInfrastructure(FMGMeetSpotInstance& Instance)
{
	// Create parking spots (100 main + 50 crew + 20 showcase)
	for (int32 i = 0; i < 100; ++i)
	{
		FMGParkingSpot Spot;
		Spot.SpotIndex = i;
		Spot.Zone = EMGMeetSpotZone::MainParking;
		// Location would be set from map data
		Instance.ParkingSpots.Add(Spot);
	}

	for (int32 i = 100; i < 150; ++i)
	{
		FMGParkingSpot Spot;
		Spot.SpotIndex = i;
		Spot.Zone = EMGMeetSpotZone::CrewParking;
		Instance.ParkingSpots.Add(Spot);
	}

	for (int32 i = 150; i < 170; ++i)
	{
		FMGParkingSpot Spot;
		Spot.SpotIndex = i;
		Spot.Zone = EMGMeetSpotZone::ShowcaseStage;
		Instance.ParkingSpots.Add(Spot);
	}

	// Create vendors
	FMGVendorInstance PartsVendor;
	PartsVendor.Type = EMGVendorType::PartsSeller;
	PartsVendor.DisplayName = FText::FromString(TEXT("Parts & Performance"));
	Instance.Vendors.Add(PartsVendor);

	FMGVendorInstance TuningVendor;
	TuningVendor.Type = EMGVendorType::TuningShop;
	TuningVendor.DisplayName = FText::FromString(TEXT("Quick Tune"));
	Instance.Vendors.Add(TuningVendor);

	FMGVendorInstance NitrousVendor;
	NitrousVendor.Type = EMGVendorType::NitrousRefill;
	NitrousVendor.DisplayName = FText::FromString(TEXT("N2O Refill"));
	Instance.Vendors.Add(NitrousVendor);

	FMGVendorInstance TireVendor;
	TireVendor.Type = EMGVendorType::TireShop;
	TireVendor.DisplayName = FText::FromString(TEXT("Tire Service"));
	Instance.Vendors.Add(TireVendor);

	FMGVendorInstance RepairVendor;
	RepairVendor.Type = EMGVendorType::RepairService;
	RepairVendor.DisplayName = FText::FromString(TEXT("Quick Repair"));
	Instance.Vendors.Add(RepairVendor);

	// Create photo spots
	FMGPhotoSpot Spot1;
	Spot1.SpotIndex = 0;
	Spot1.SpotName = FText::FromString(TEXT("Neon Alley"));
	Spot1.LightingPreset = TEXT("NeonNight");
	Instance.PhotoSpots.Add(Spot1);

	FMGPhotoSpot Spot2;
	Spot2.SpotIndex = 1;
	Spot2.SpotName = FText::FromString(TEXT("Studio White"));
	Spot2.LightingPreset = TEXT("Studio");
	Instance.PhotoSpots.Add(Spot2);

	FMGPhotoSpot Spot3;
	Spot3.SpotIndex = 2;
	Spot3.SpotName = FText::FromString(TEXT("Golden Hour"));
	Spot3.LightingPreset = TEXT("Sunset");
	Instance.PhotoSpots.Add(Spot3);
}

void UMGMeetSpotSubsystem::AdvanceShowcaseQueue(FMGMeetSpotInstance& Instance)
{
	// Reset current showcaser
	if (Instance.CurrentShowcasePlayerID.IsValid())
	{
		FMGMeetSpotPlayer* OldShowcase = FindPlayer(Instance.InstanceID, Instance.CurrentShowcasePlayerID);
		if (OldShowcase)
		{
			OldShowcase->bIsShowcasing = false;
		}
	}

	Instance.CurrentShowcasePlayerID = FGuid();

	// Get next in queue
	if (Instance.ShowcaseQueue.Num() > 0)
	{
		FGuid NextPlayerID = Instance.ShowcaseQueue[0];
		Instance.ShowcaseQueue.RemoveAt(0);

		FMGMeetSpotPlayer* NextPlayer = FindPlayer(Instance.InstanceID, NextPlayerID);
		if (NextPlayer)
		{
			Instance.CurrentShowcasePlayerID = NextPlayerID;
			Instance.ShowcaseEndTime = FDateTime::Now() + FTimespan::FromSeconds(ShowcaseDuration);
			NextPlayer->bIsShowcasing = true;
			NextPlayer->ShowcaseVotes = 0;

			OnShowcaseStarted.Broadcast(Instance.InstanceID, *NextPlayer);
		}
	}
}

bool UMGMeetSpotSubsystem::IsCrewMember(FGuid PlayerID, FGuid CrewID) const
{
	// TODO: Check via crew subsystem
	return true;
}

FMGMeetSpotPlayer* UMGMeetSpotSubsystem::FindPlayer(FGuid InstanceID, FGuid PlayerID)
{
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return nullptr;
	}

	for (FMGMeetSpotPlayer& Player : Instance->Players)
	{
		if (Player.PlayerID == PlayerID)
		{
			return &Player;
		}
	}

	return nullptr;
}

const FMGMeetSpotPlayer* UMGMeetSpotSubsystem::FindPlayerConst(FGuid InstanceID, FGuid PlayerID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return nullptr;
	}

	for (const FMGMeetSpotPlayer& Player : Instance->Players)
	{
		if (Player.PlayerID == PlayerID)
		{
			return &Player;
		}
	}

	return nullptr;
}

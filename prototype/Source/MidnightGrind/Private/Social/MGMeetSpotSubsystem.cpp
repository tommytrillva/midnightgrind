// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMeetSpotSubsystem.cpp
 * @brief Implementation of Meet Spot Social Hub Subsystem
 *
 * Meet spots are the heart of car culture in Midnight Grind.
 * Per GDD Design Pillar 4: "Living Car Culture" - the social aspects
 * are as important as the racing itself.
 */

#include "Social/MGMeetSpotSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Reputation/MGReputationSubsystem.h"
#include "Crew/MGCrewSubsystem.h"
#include "Engine/GameInstance.h"
#include "GameModes/MGRaceFlowManager.h"
#include "EngineAudio/MGEngineAudioSubsystem.h"

void UMGMeetSpotSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize locations and emotes
	InitializeMeetSpotLocations();
	InitializeEmotes();

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

		// Start presence reputation timer (every minute)
		World->GetTimerManager().SetTimer(
			PresenceReputationTimer,
			this,
			&UMGMeetSpotSubsystem::OnPresenceReputationTick,
			PresenceReputationInterval,
			true
		);
	}
}

void UMGMeetSpotSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UpdateTimer);
		World->GetTimerManager().ClearTimer(PresenceReputationTimer);
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

TArray<FMGMeetSpotLocation> UMGMeetSpotSubsystem::GetAccessibleMeetSpots(int32 ReputationTier) const
{
	TArray<FMGMeetSpotLocation> AccessibleLocations;

	for (const FMGMeetSpotLocation& Location : MeetSpotLocations)
	{
		if (Location.RequiredReputationTier <= ReputationTier)
		{
			AccessibleLocations.Add(Location);
		}
	}

	return AccessibleLocations;
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

	// Update vibe level
	Instance->VibeLevel = CalculateVibeLevel(*Instance);

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
			// Award showcase reputation if they were showcasing
			if (Instance->Players[i].bIsShowcasing)
			{
				AwardShowcaseReputation(PlayerID, Instance->Players[i].ShowcaseVotes);
			}

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

			// Remove from any active challenges
			for (int32 j = Instance->ActiveChallenges.Num() - 1; j >= 0; --j)
			{
				FMGRaceChallenge& Challenge = Instance->ActiveChallenges[j];
				if (Challenge.ChallengerID == PlayerID)
				{
					Instance->ActiveChallenges.RemoveAt(j);
				}
				else
				{
					Challenge.AcceptedParticipants.Remove(PlayerID);
				}
			}

			Instance->Players.RemoveAt(i);
			Instance->CurrentPlayerCount--;
			break;
		}
	}

	// Update vibe level
	Instance->VibeLevel = CalculateVibeLevel(*Instance);

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
		// Award reputation before leaving
		FMGMeetSpotPlayer* Player = FindPlayer(InstanceID, PlayerID);
		if (Player)
		{
			AwardShowcaseReputation(PlayerID, Player->ShowcaseVotes);
		}
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

	// Award social reputation for voting
	AwardSocialReputation(VoterID, TEXT("VoteForBuild"));

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
	// Verify moderator permissions (crew leader, event organizer, or instance creator)
	if (!HasModeratorPermissions(ModeratorID, InstanceID))
	{
		return;
	}

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
			// Broadcast vendor interaction event for UI system to handle
			OnVendorInteraction.Broadcast(PlayerID, VendorID, Vendor.Type);

			// Award small social reputation for engaging with vendors
			AwardSocialReputation(PlayerID, TEXT("VendorInteraction"));

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

				// Boost vibe level for event
				Instance.VibeLevel = FMath::Min(100, Instance.VibeLevel + 30);
				OnVibeChanged.Broadcast(Instance.InstanceID, Instance.VibeLevel);

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

			// Award reputation to participants
			for (const FGuid& ParticipantID : Instance.CurrentEvent.RegisteredParticipants)
			{
				if (Instance.CurrentEvent.ReputationReward > 0)
				{
					AwardSocialReputation(ParticipantID, TEXT("EventParticipation"));
				}
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
// RACE CHALLENGES
// ==========================================

bool UMGMeetSpotSubsystem::CreateRaceChallenge(
	FGuid ChallengerID,
	EMGRaceChallengeType ChallengeType,
	FName RaceType,
	FName TrackID,
	float PILimit,
	int64 WagerAmount,
	FGuid TargetID,
	bool bIsOpen,
	FGuid& OutChallengeID)
{
	FGuid InstanceID = GetPlayerMeetSpot(ChallengerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	// Verify challenger is in the meet spot
	const FMGMeetSpotPlayer* Challenger = FindPlayerConst(InstanceID, ChallengerID);
	if (!Challenger)
	{
		return false;
	}

	// Check for pink slip validity
	if (ChallengeType == EMGRaceChallengeType::PinkSlip)
	{
		// Must have valid vehicle to wager
		if (!Challenger->VehicleID.IsValid())
		{
			return false;
		}
	}

	// If targeting specific player, verify they're in the meet spot
	if (TargetID.IsValid())
	{
		if (!FindPlayerConst(InstanceID, TargetID))
		{
			return false;
		}
	}

	// Create challenge
	FMGRaceChallenge Challenge;
	Challenge.ChallengerID = ChallengerID;
	Challenge.ChallengerName = Challenger->DisplayName;
	Challenge.TargetID = TargetID;
	Challenge.ChallengeType = ChallengeType;
	Challenge.RaceType = RaceType;
	Challenge.TrackID = TrackID;
	Challenge.PILimit = PILimit;
	Challenge.WagerAmount = WagerAmount;
	Challenge.bIsOpenChallenge = bIsOpen;
	Challenge.AcceptedParticipants.Add(ChallengerID);

	// Set max participants based on challenge type
	if (ChallengeType == EMGRaceChallengeType::PinkSlip)
	{
		Challenge.MaxParticipants = 2; // Pink slip is always 1v1
	}
	else if (ChallengeType == EMGRaceChallengeType::CrewBattle)
	{
		Challenge.MaxParticipants = 8;
	}

	Instance->ActiveChallenges.Add(Challenge);
	OutChallengeID = Challenge.ChallengeID;

	// Boost vibe level when challenges are created
	Instance->VibeLevel = FMath::Min(100, Instance->VibeLevel + 5);

	OnRaceChallengeCreated.Broadcast(InstanceID, Challenge);

	return true;
}

bool UMGMeetSpotSubsystem::AcceptRaceChallenge(FGuid PlayerID, FGuid ChallengeID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	for (FMGRaceChallenge& Challenge : Instance->ActiveChallenges)
	{
		if (Challenge.ChallengeID == ChallengeID)
		{
			// Can't accept own challenge
			if (Challenge.ChallengerID == PlayerID)
			{
				return false;
			}

			// Check if it's a targeted challenge
			if (Challenge.TargetID.IsValid() && Challenge.TargetID != PlayerID)
			{
				return false;
			}

			// Check if already accepted
			if (Challenge.AcceptedParticipants.Contains(PlayerID))
			{
				return false;
			}

			// Check capacity
			if (Challenge.AcceptedParticipants.Num() >= Challenge.MaxParticipants)
			{
				return false;
			}

			Challenge.AcceptedParticipants.Add(PlayerID);

			OnRaceChallengeAccepted.Broadcast(ChallengeID, PlayerID);

			return true;
		}
	}

	return false;
}

bool UMGMeetSpotSubsystem::CancelRaceChallenge(FGuid PlayerID, FGuid ChallengeID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	for (int32 i = 0; i < Instance->ActiveChallenges.Num(); ++i)
	{
		if (Instance->ActiveChallenges[i].ChallengeID == ChallengeID)
		{
			// Only challenger can cancel
			if (Instance->ActiveChallenges[i].ChallengerID != PlayerID)
			{
				return false;
			}

			Instance->ActiveChallenges.RemoveAt(i);
			return true;
		}
	}

	return false;
}

bool UMGMeetSpotSubsystem::LaunchChallengeRace(FGuid ChallengerID, FGuid ChallengeID)
{
	FGuid InstanceID = GetPlayerMeetSpot(ChallengerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	for (int32 i = 0; i < Instance->ActiveChallenges.Num(); ++i)
	{
		FMGRaceChallenge& Challenge = Instance->ActiveChallenges[i];
		if (Challenge.ChallengeID == ChallengeID && Challenge.ChallengerID == ChallengerID)
		{
			// Need at least 2 participants
			if (Challenge.AcceptedParticipants.Num() < 2)
			{
				return false;
			}

			// Integrate with race management subsystem
			if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
			{
				if (UMGRaceFlowManager* RaceFlowManager = GI->GetSubsystem<UMGRaceFlowManager>())
				{
					// Build race config from challenge parameters
					FMGRaceConfig RaceConfig;
					RaceConfig.RaceType = Challenge.RaceType;
					RaceConfig.TrackID = Challenge.TrackID;
					RaceConfig.MaxParticipants = Challenge.MaxParticipants;
					RaceConfig.PILimit = Challenge.PILimit;
					RaceConfig.bIsPinkSlip = (Challenge.ChallengeType == EMGRaceChallengeType::PinkSlip);
					RaceConfig.WagerAmount = Challenge.WagerAmount;

					// Get track map path (would be looked up from track registry in production)
					FSoftObjectPath TrackMapPath;
					if (Challenge.TrackID != NAME_None)
					{
						// Format: /Game/Maps/Tracks/{TrackID}/{TrackID}
						FString TrackPath = FString::Printf(TEXT("/Game/Maps/Tracks/%s/%s"), *Challenge.TrackID.ToString(), *Challenge.TrackID.ToString());
						TrackMapPath = FSoftObjectPath(TrackPath);
					}

					// Get challenger's vehicle ID for race initialization
					const FMGMeetSpotPlayer* Challenger = FindPlayerConst(InstanceID, ChallengerID);
					FName PlayerVehicleID = Challenger ? *Challenger->VehicleID.ToString() : NAME_None;

					// Begin race load via race flow manager
					RaceFlowManager->BeginRaceLoad(TrackMapPath, RaceConfig, PlayerVehicleID);
				}
			}

			// Also broadcast event for any other listeners
			OnRaceLaunchRequested.Broadcast(
				InstanceID,
				Challenge,
				Challenge.AcceptedParticipants,
				Challenge.RaceType,
				Challenge.TrackID
			);

			// Remove challenge after launch
			Instance->ActiveChallenges.RemoveAt(i);
			return true;
		}
	}

	return false;
}

TArray<FMGRaceChallenge> UMGMeetSpotSubsystem::GetActiveChallenges(FGuid InstanceID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (Instance)
	{
		return Instance->ActiveChallenges;
	}
	return TArray<FMGRaceChallenge>();
}

TArray<FMGRaceChallenge> UMGMeetSpotSubsystem::GetChallengesForPlayer(FGuid PlayerID) const
{
	TArray<FMGRaceChallenge> Results;
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return Results;
	}

	for (const FMGRaceChallenge& Challenge : Instance->ActiveChallenges)
	{
		// Include open challenges or challenges specifically targeting this player
		if (Challenge.bIsOpenChallenge || Challenge.TargetID == PlayerID)
		{
			Results.Add(Challenge);
		}
	}

	return Results;
}

// Legacy race methods
bool UMGMeetSpotSubsystem::ProposeRace(FGuid OrganizerID, FName RaceType, float PILimit, int32 MaxParticipants, int64 EntryFee)
{
	FGuid ChallengeID;
	return CreateRaceChallenge(
		OrganizerID,
		EntryFee > 0 ? EMGRaceChallengeType::CashWager : EMGRaceChallengeType::Friendly,
		RaceType,
		NAME_None, // No specific track
		PILimit,
		EntryFee,
		FGuid(), // No specific target
		true, // Open challenge
		ChallengeID
	);
}

bool UMGMeetSpotSubsystem::AcceptRaceProposal(FGuid PlayerID, FGuid RaceProposalID)
{
	return AcceptRaceChallenge(PlayerID, RaceProposalID);
}

bool UMGMeetSpotSubsystem::LaunchRace(FGuid OrganizerID, FGuid RaceProposalID)
{
	return LaunchChallengeRace(OrganizerID, RaceProposalID);
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
// SOCIAL INTERACTIONS
// ==========================================

bool UMGMeetSpotSubsystem::PlayEmote(FGuid PlayerID, FName EmoteID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	FMGMeetSpotPlayer* Player = FindPlayer(InstanceID, PlayerID);
	if (!Player)
	{
		return false;
	}

	// Check if emote exists and is available
	const FMGSocialEmote* Emote = nullptr;
	for (const FMGSocialEmote& E : AvailableEmotes)
	{
		if (E.EmoteID == EmoteID)
		{
			Emote = &E;
			break;
		}
	}

	if (!Emote)
	{
		return false;
	}

	// Check reputation tier requirement
	if (Emote->RequiredReputationTier > Player->ReputationTier)
	{
		return false;
	}

	Player->CurrentEmote = EmoteID;

	// Award social reputation for using emotes
	AwardSocialReputation(PlayerID, TEXT("UseEmote"));

	OnEmotePlayed.Broadcast(InstanceID, PlayerID, EmoteID);

	return true;
}

TArray<FMGSocialEmote> UMGMeetSpotSubsystem::GetAvailableEmotes(int32 ReputationTier) const
{
	TArray<FMGSocialEmote> Available;

	for (const FMGSocialEmote& Emote : AvailableEmotes)
	{
		if (Emote.RequiredReputationTier <= ReputationTier)
		{
			FMGSocialEmote EmoteCopy = Emote;
			EmoteCopy.bUnlocked = true;
			Available.Add(EmoteCopy);
		}
		else
		{
			FMGSocialEmote EmoteCopy = Emote;
			EmoteCopy.bUnlocked = false;
			Available.Add(EmoteCopy);
		}
	}

	return Available;
}

void UMGMeetSpotSubsystem::UseHorn(FGuid PlayerID, EMGHornPattern Pattern)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	if (!InstanceID.IsValid())
	{
		return;
	}

	// Double short honk is a challenge signal
	if (Pattern == EMGHornPattern::DoubleShort)
	{
		// Find nearest player we're facing and signal challenge intent
		FGuid TargetID = FindNearestFacingPlayer(PlayerID, InstanceID);
		if (TargetID.IsValid())
		{
			OnChallengeIntent.Broadcast(PlayerID, TargetID, TEXT("Horn"));
		}
	}

	OnHornPlayed.Broadcast(InstanceID, PlayerID, Pattern);
}

void UMGMeetSpotSubsystem::FlashHeadlights(FGuid PlayerID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return;
	}

	// Flashing headlights = challenge signal per street racing culture
	// Find nearest vehicle we're facing and send challenge notification
	FGuid TargetID = FindNearestFacingPlayer(PlayerID, InstanceID);
	if (TargetID.IsValid())
	{
		OnChallengeIntent.Broadcast(PlayerID, TargetID, TEXT("Headlights"));
	}

	// Award small social reputation
	AwardSocialReputation(PlayerID, TEXT("FlashLights"));
}

void UMGMeetSpotSubsystem::RevEngine(FGuid PlayerID)
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	if (!InstanceID.IsValid())
	{
		return;
	}

	// Broadcast engine rev audio event for audio system to handle
	OnEngineRevAudio.Broadcast(InstanceID, PlayerID);

	// Award tiny social reputation for engagement
	AwardSocialReputation(PlayerID, TEXT("RevEngine"));
}

bool UMGMeetSpotSubsystem::GiveRespect(FGuid FromPlayerID, FGuid ToPlayerID, FName RespectType)
{
	// Can't respect yourself
	if (FromPlayerID == ToPlayerID)
	{
		return false;
	}

	// Check both players are in meet spots
	FGuid FromInstance = GetPlayerMeetSpot(FromPlayerID);
	FGuid ToInstance = GetPlayerMeetSpot(ToPlayerID);
	if (!FromInstance.IsValid() || !ToInstance.IsValid())
	{
		return false;
	}

	// They should be in the same instance
	if (FromInstance != ToInstance)
	{
		return false;
	}

	// Check cooldown
	TPair<FGuid, FGuid> CooldownKey(FromPlayerID, ToPlayerID);
	if (FDateTime* LastRespect = RespectCooldowns.Find(CooldownKey))
	{
		FTimespan TimeSince = FDateTime::Now() - *LastRespect;
		if (TimeSince.GetTotalSeconds() < RespectCooldownSeconds)
		{
			return false;
		}
	}

	FMGMeetSpotPlayer* ReceivingPlayer = FindPlayer(ToInstance, ToPlayerID);
	if (!ReceivingPlayer)
	{
		return false;
	}

	// Add respect
	ReceivingPlayer->RespectReceived++;

	// Update cooldown
	RespectCooldowns.Add(CooldownKey, FDateTime::Now());

	// Award reputation to receiver
	AwardSocialReputation(ToPlayerID, TEXT("ReceivedRespect"));

	// Update vibe level
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(ToInstance);
	if (Instance)
	{
		Instance->VibeLevel = FMath::Min(100, Instance->VibeLevel + 1);
	}

	OnRespectGiven.Broadcast(FromPlayerID, ToPlayerID, ReceivingPlayer->RespectReceived);

	return true;
}

int32 UMGMeetSpotSubsystem::GetPlayerRespect(FGuid PlayerID) const
{
	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	const FMGMeetSpotPlayer* Player = FindPlayerConst(InstanceID, PlayerID);
	if (Player)
	{
		return Player->RespectReceived;
	}
	return 0;
}

void UMGMeetSpotSubsystem::SendProximityMessage(FGuid SenderID, const FString& Message, float Range)
{
	FGuid InstanceID = GetPlayerMeetSpot(SenderID);
	FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return;
	}

	const FMGMeetSpotPlayer* Sender = FindPlayerConst(InstanceID, SenderID);
	if (!Sender)
	{
		return;
	}

	// Get all players within range of sender
	TArray<FGuid> Recipients = GetPlayersInRange(InstanceID, Sender->CurrentLocation, Range);

	// Broadcast message to players within range
	if (Recipients.Num() > 0)
	{
		OnProximityMessage.Broadcast(InstanceID, SenderID, Message, Recipients);
	}
}

// ==========================================
// REPUTATION INTEGRATION
// ==========================================

void UMGMeetSpotSubsystem::AwardPresenceReputation(FGuid PlayerID)
{
	// Award small amount of Social reputation for being present at meet
	// Amount scales with vibe level

	FGuid InstanceID = GetPlayerMeetSpot(PlayerID);
	if (const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID))
	{
		// Base presence rep: 5, scaled by vibe level (0.5x to 1.5x)
		float VibeMultiplier = 0.5f + (Instance->VibeLevel / 100.0f);
		int32 PresenceRep = FMath::RoundToInt(5.0f * VibeMultiplier);

		// Integrate with reputation subsystem
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
		{
			if (UMGReputationSubsystem* ReputationSubsystem = GI->GetSubsystem<UMGReputationSubsystem>())
			{
				ReputationSubsystem->AddReputation(
					EMGReputationCategory::Social,
					PresenceRep,
					TEXT("MeetSpotPresence")
				);
			}
		}
	}
}

void UMGMeetSpotSubsystem::AwardShowcaseReputation(FGuid PlayerID, int32 VoteCount)
{
	// Showcase reputation scales with votes received
	// Base: 25 rep, +10 per vote, capped at 200
	int32 ShowcaseRep = FMath::Min(200, 25 + (VoteCount * 10));

	// Integrate with reputation subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UMGReputationSubsystem* ReputationSubsystem = GI->GetSubsystem<UMGReputationSubsystem>())
		{
			ReputationSubsystem->AddReputation(
				EMGReputationCategory::Social,
				ShowcaseRep,
				FString::Printf(TEXT("VehicleShowcase_%dVotes"), VoteCount)
			);
		}
	}
}

void UMGMeetSpotSubsystem::AwardSocialReputation(FGuid PlayerID, FName InteractionType)
{
	// Different interactions award different amounts
	int32 RepAmount = 0;

	if (InteractionType == TEXT("UseEmote"))
	{
		RepAmount = 1;
	}
	else if (InteractionType == TEXT("VoteForBuild"))
	{
		RepAmount = 3;
	}
	else if (InteractionType == TEXT("FlashLights"))
	{
		RepAmount = 1;
	}
	else if (InteractionType == TEXT("RevEngine"))
	{
		RepAmount = 1;
	}
	else if (InteractionType == TEXT("ReceivedRespect"))
	{
		RepAmount = 10;
	}
	else if (InteractionType == TEXT("EventParticipation"))
	{
		RepAmount = 50;
	}

	// In production: ReputationSubsystem->AddReputation(EMGReputationCategory::Social, RepAmount, InteractionType.ToString());
}

int32 UMGMeetSpotSubsystem::GetVibeLevel(FGuid InstanceID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (Instance)
	{
		return Instance->VibeLevel;
	}
	return 0;
}

// ==========================================
// INTERNAL
// ==========================================

void UMGMeetSpotSubsystem::OnUpdateTick()
{
	UpdateShowcases();
	UpdatePhotoSpots();
	UpdateChallenges();
	UpdateVibeLevels();
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
			// Award reputation before advancing
			FMGMeetSpotPlayer* CurrentShowcase = FindPlayer(Instance.InstanceID, Instance.CurrentShowcasePlayerID);
			if (CurrentShowcase)
			{
				AwardShowcaseReputation(Instance.CurrentShowcasePlayerID, CurrentShowcase->ShowcaseVotes);
			}
			AdvanceShowcaseQueue(Instance);
		}
	}
}

void UMGMeetSpotSubsystem::UpdatePhotoSpots()
{
	// Process photo spot queues
	// Advance players when current occupant leaves
}

void UMGMeetSpotSubsystem::UpdateChallenges()
{
	FDateTime Now = FDateTime::Now();

	for (auto& Pair : ActiveInstances)
	{
		FMGMeetSpotInstance& Instance = Pair.Value;

		// Remove expired challenges
		for (int32 i = Instance.ActiveChallenges.Num() - 1; i >= 0; --i)
		{
			if (Now >= Instance.ActiveChallenges[i].ExpirationTime)
			{
				Instance.ActiveChallenges.RemoveAt(i);
			}
		}
	}
}

void UMGMeetSpotSubsystem::UpdateVibeLevels()
{
	// Vibe levels slowly decay toward baseline (50)
	for (auto& Pair : ActiveInstances)
	{
		FMGMeetSpotInstance& Instance = Pair.Value;

		int32 TargetVibe = CalculateVibeLevel(Instance);

		// Slowly move toward target
		if (Instance.VibeLevel < TargetVibe)
		{
			Instance.VibeLevel = FMath::Min(Instance.VibeLevel + 1, TargetVibe);
		}
		else if (Instance.VibeLevel > TargetVibe)
		{
			Instance.VibeLevel = FMath::Max(Instance.VibeLevel - 1, TargetVibe);
		}
	}
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

void UMGMeetSpotSubsystem::OnPresenceReputationTick()
{
	// Award presence reputation to all players in meet spots
	for (const auto& Pair : PlayerInstanceMap)
	{
		AwardPresenceReputation(Pair.Key);
	}
}

FMGMeetSpotInstance UMGMeetSpotSubsystem::CreateNewInstance(FName MeetSpotID)
{
	FMGMeetSpotInstance Instance;
	Instance.MeetSpotID = MeetSpotID;

	// Get location data
	const FMGMeetSpotLocation* LocationData = GetLocationData(MeetSpotID);
	if (LocationData)
	{
		Instance.DisplayName = LocationData->DisplayName;
		Instance.LocationType = LocationData->LocationType;
		Instance.MaxPlayers = LocationData->MaxCapacity;
	}
	else
	{
		Instance.MaxPlayers = 200; // Per PRD Section 2.1
	}

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
		Spot.bIsPremiumSpot = i < 10; // First 10 are premium
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

	FMGVendorInstance Photographer;
	Photographer.Type = EMGVendorType::Photographer;
	Photographer.DisplayName = FText::FromString(TEXT("Street Shots"));
	Instance.Vendors.Add(Photographer);

	FMGVendorInstance FoodTruck;
	FoodTruck.Type = EMGVendorType::FoodTruck;
	FoodTruck.DisplayName = FText::FromString(TEXT("Midnight Eats"));
	Instance.Vendors.Add(FoodTruck);

	// Create photo spots
	FMGPhotoSpot Spot1;
	Spot1.SpotIndex = 0;
	Spot1.SpotName = FText::FromString(TEXT("Neon Alley"));
	Spot1.LightingPreset = TEXT("NeonNight");
	Spot1.BackdropType = TEXT("Urban");
	Instance.PhotoSpots.Add(Spot1);

	FMGPhotoSpot Spot2;
	Spot2.SpotIndex = 1;
	Spot2.SpotName = FText::FromString(TEXT("Studio White"));
	Spot2.LightingPreset = TEXT("Studio");
	Spot2.BackdropType = TEXT("Clean");
	Instance.PhotoSpots.Add(Spot2);

	FMGPhotoSpot Spot3;
	Spot3.SpotIndex = 2;
	Spot3.SpotName = FText::FromString(TEXT("Golden Hour"));
	Spot3.LightingPreset = TEXT("Sunset");
	Spot3.BackdropType = TEXT("Skyline");
	Instance.PhotoSpots.Add(Spot3);

	FMGPhotoSpot Spot4;
	Spot4.SpotIndex = 3;
	Spot4.SpotName = FText::FromString(TEXT("Underground"));
	Spot4.LightingPreset = TEXT("DarkMoody");
	Spot4.BackdropType = TEXT("Garage");
	Instance.PhotoSpots.Add(Spot4);
}

void UMGMeetSpotSubsystem::InitializeMeetSpotLocations()
{
	// Downtown Parking - Starting location per GDD
	{
		FMGMeetSpotLocation Location;
		Location.LocationID = TEXT("DowntownParking");
		Location.DisplayName = FText::FromString(TEXT("Downtown Parking Garage"));
		Location.Description = FText::FromString(TEXT("Multi-level parking garage in the heart of the city. The classic meet spot."));
		Location.LocationType = EMGMeetSpotLocationType::ParkingLot;
		Location.DistrictID = TEXT("Downtown");
		Location.MaxCapacity = 200;
		Location.RequiredReputationTier = 0; // Available from start
		Location.LightingPreset = TEXT("NeonUrban");
		Location.AmbientAudioPreset = TEXT("CityNight");
		MeetSpotLocations.Add(Location);
	}

	// Industrial Warehouses
	{
		FMGMeetSpotLocation Location;
		Location.LocationID = TEXT("IndustrialWarehouse");
		Location.DisplayName = FText::FromString(TEXT("Industrial Warehouse Lot"));
		Location.Description = FText::FromString(TEXT("Abandoned warehouse district. Low traffic, minimal witnesses."));
		Location.LocationType = EMGMeetSpotLocationType::Industrial;
		Location.DistrictID = TEXT("Industrial");
		Location.MaxCapacity = 150;
		Location.RequiredReputationTier = 1; // Need some rep
		Location.LightingPreset = TEXT("GrittyIndustrial");
		Location.AmbientAudioPreset = TEXT("IndustrialAmbient");
		MeetSpotLocations.Add(Location);
	}

	// Mountain Overlook - Higher tier
	{
		FMGMeetSpotLocation Location;
		Location.LocationID = TEXT("MountainOverlook");
		Location.DisplayName = FText::FromString(TEXT("Canyon Overlook"));
		Location.Description = FText::FromString(TEXT("Scenic vista overlooking the city. Where legends gather."));
		Location.LocationType = EMGMeetSpotLocationType::Overlook;
		Location.DistrictID = TEXT("TheHills");
		Location.MaxCapacity = 100;
		Location.RequiredReputationTier = 3; // Need reputation
		Location.LightingPreset = TEXT("MoonlitVista");
		Location.AmbientAudioPreset = TEXT("MountainWind");
		MeetSpotLocations.Add(Location);
	}

	// Waterfront Docks
	{
		FMGMeetSpotLocation Location;
		Location.LocationID = TEXT("WaterfrontDocks");
		Location.DisplayName = FText::FromString(TEXT("Port District Docks"));
		Location.Description = FText::FromString(TEXT("Container port with ocean views. Import tuner territory."));
		Location.LocationType = EMGMeetSpotLocationType::Waterfront;
		Location.DistrictID = TEXT("PortDistrict");
		Location.MaxCapacity = 150;
		Location.RequiredReputationTier = 2;
		Location.LightingPreset = TEXT("HarborNight");
		Location.AmbientAudioPreset = TEXT("PortAmbient");
		MeetSpotLocations.Add(Location);
	}

	// Highway Rest Stop
	{
		FMGMeetSpotLocation Location;
		Location.LocationID = TEXT("HighwayRestStop");
		Location.DisplayName = FText::FromString(TEXT("Highway Rest Area"));
		Location.Description = FText::FromString(TEXT("Perfect staging point for highway battles. Wangan warriors welcome."));
		Location.LocationType = EMGMeetSpotLocationType::RestStop;
		Location.DistrictID = TEXT("Highway");
		Location.MaxCapacity = 80;
		Location.RequiredReputationTier = 2;
		Location.LightingPreset = TEXT("HighwayLights");
		Location.AmbientAudioPreset = TEXT("HighwayTraffic");
		MeetSpotLocations.Add(Location);
	}

	// The Underground - Legendary spot
	{
		FMGMeetSpotLocation Location;
		Location.LocationID = TEXT("TheUnderground");
		Location.DisplayName = FText::FromString(TEXT("The Underground"));
		Location.Description = FText::FromString(TEXT("Invitation only. Where pink slip legends are made."));
		Location.LocationType = EMGMeetSpotLocationType::Historic;
		Location.DistrictID = TEXT("Downtown");
		Location.MaxCapacity = 50;
		Location.RequiredReputationTier = 5; // Legendary tier
		Location.bIsLegendarySpot = true;
		Location.LightingPreset = TEXT("UndergroundNeon");
		Location.AmbientAudioPreset = TEXT("UndergroundBasement");
		MeetSpotLocations.Add(Location);
	}
}

void UMGMeetSpotSubsystem::InitializeEmotes()
{
	// Greeting emotes - Available from start
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Wave");
		Emote.DisplayName = FText::FromString(TEXT("Wave"));
		Emote.Category = EMGEmoteCategory::Greeting;
		Emote.Duration = 2.0f;
		Emote.RequiredReputationTier = 0;
		AvailableEmotes.Add(Emote);
	}
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Nod");
		Emote.DisplayName = FText::FromString(TEXT("Respectful Nod"));
		Emote.Category = EMGEmoteCategory::Greeting;
		Emote.Duration = 1.5f;
		Emote.RequiredReputationTier = 0;
		AvailableEmotes.Add(Emote);
	}

	// Respect emotes
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Clap");
		Emote.DisplayName = FText::FromString(TEXT("Applause"));
		Emote.Category = EMGEmoteCategory::Respect;
		Emote.Duration = 3.0f;
		Emote.RequiredReputationTier = 1;
		AvailableEmotes.Add(Emote);
	}
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Bow");
		Emote.DisplayName = FText::FromString(TEXT("Respectful Bow"));
		Emote.Category = EMGEmoteCategory::Respect;
		Emote.Duration = 2.5f;
		Emote.RequiredReputationTier = 2;
		AvailableEmotes.Add(Emote);
	}

	// Celebration emotes
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("FistPump");
		Emote.DisplayName = FText::FromString(TEXT("Fist Pump"));
		Emote.Category = EMGEmoteCategory::Celebration;
		Emote.Duration = 2.0f;
		Emote.RequiredReputationTier = 0;
		AvailableEmotes.Add(Emote);
	}
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Victory");
		Emote.DisplayName = FText::FromString(TEXT("Victory Pose"));
		Emote.Category = EMGEmoteCategory::Celebration;
		Emote.Duration = 3.0f;
		Emote.RequiredReputationTier = 2;
		AvailableEmotes.Add(Emote);
	}

	// Taunt emotes - Higher tier
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Flex");
		Emote.DisplayName = FText::FromString(TEXT("Flex"));
		Emote.Category = EMGEmoteCategory::Taunt;
		Emote.Duration = 2.5f;
		Emote.RequiredReputationTier = 2;
		AvailableEmotes.Add(Emote);
	}
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("ComeAtMe");
		Emote.DisplayName = FText::FromString(TEXT("Come At Me"));
		Emote.Category = EMGEmoteCategory::Taunt;
		Emote.Duration = 2.0f;
		Emote.RequiredReputationTier = 3;
		AvailableEmotes.Add(Emote);
	}

	// Vehicle interaction emotes
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("LeanOnCar");
		Emote.DisplayName = FText::FromString(TEXT("Lean On Car"));
		Emote.Category = EMGEmoteCategory::VehicleInteraction;
		Emote.Duration = 5.0f;
		Emote.RequiredReputationTier = 1;
		AvailableEmotes.Add(Emote);
	}
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("PopHood");
		Emote.DisplayName = FText::FromString(TEXT("Show Engine"));
		Emote.Category = EMGEmoteCategory::VehicleInteraction;
		Emote.Duration = 4.0f;
		Emote.RequiredReputationTier = 1;
		AvailableEmotes.Add(Emote);
	}
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("CleanWheel");
		Emote.DisplayName = FText::FromString(TEXT("Polish Wheels"));
		Emote.Category = EMGEmoteCategory::VehicleInteraction;
		Emote.Duration = 4.0f;
		Emote.RequiredReputationTier = 2;
		AvailableEmotes.Add(Emote);
	}

	// Dance emotes - Fun additions
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("Groove");
		Emote.DisplayName = FText::FromString(TEXT("Groove"));
		Emote.Category = EMGEmoteCategory::Dance;
		Emote.Duration = 5.0f;
		Emote.RequiredReputationTier = 2;
		AvailableEmotes.Add(Emote);
	}

	// Legendary emotes
	{
		FMGSocialEmote Emote;
		Emote.EmoteID = TEXT("LegendaryPose");
		Emote.DisplayName = FText::FromString(TEXT("Legendary Stance"));
		Emote.Category = EMGEmoteCategory::Celebration;
		Emote.Duration = 4.0f;
		Emote.RequiredReputationTier = 5; // Legendary tier only
		AvailableEmotes.Add(Emote);
	}
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
	// Check via crew subsystem
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UMGCrewSubsystem* CrewSubsystem = GI->GetSubsystem<UMGCrewSubsystem>())
		{
			// Check if the player is in the local player's crew
			// Note: For full multiplayer support, this would need server-side verification
			if (CrewSubsystem->IsInCrew())
			{
				const FMGCrewInfo& CurrentCrew = CrewSubsystem->GetCurrentCrew();

				// Check if this is the same crew
				if (CurrentCrew.CrewID == CrewID)
				{
					// Convert Guid to FName for crew lookup
					FName PlayerIDName(*PlayerID.ToString());
					FMGCrewMember Member = CrewSubsystem->GetMember(PlayerIDName);

					// If member has valid PlayerID, they're in the crew
					return !Member.PlayerID.IsNone();
				}
			}
		}
	}

	// Default to false if we can't verify (fail closed for security)
	return false;
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

const FMGMeetSpotLocation* UMGMeetSpotSubsystem::GetLocationData(FName MeetSpotID) const
{
	for (const FMGMeetSpotLocation& Location : MeetSpotLocations)
	{
		if (Location.LocationID == MeetSpotID)
		{
			return &Location;
		}
	}
	return nullptr;
}

int32 UMGMeetSpotSubsystem::CalculateVibeLevel(const FMGMeetSpotInstance& Instance) const
{
	// Base vibe from player count (max 50 from this)
	float PlayerContribution = FMath::Min(50.0f, Instance.CurrentPlayerCount * 0.5f);

	// Bonus from event (up to 30)
	float EventBonus = Instance.CurrentEvent.EventID.IsValid() ? 30.0f : 0.0f;

	// Bonus from active showcasing (up to 10)
	float ShowcaseBonus = Instance.CurrentShowcasePlayerID.IsValid() ? 10.0f : 0.0f;

	// Bonus from active challenges (up to 10)
	float ChallengeBonus = FMath::Min(10.0f, Instance.ActiveChallenges.Num() * 2.0f);

	return FMath::RoundToInt(PlayerContribution + EventBonus + ShowcaseBonus + ChallengeBonus);
}

bool UMGMeetSpotSubsystem::HasModeratorPermissions(FGuid PlayerID, FGuid InstanceID) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return false;
	}

	// Check if player is event organizer
	if (Instance->CurrentEvent.EventID.IsValid() && Instance->CurrentEvent.OrganizerID == PlayerID)
	{
		return true;
	}

	// Check if player is a crew leader present in the meet spot
	if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		if (UMGCrewSubsystem* CrewSubsystem = GI->GetSubsystem<UMGCrewSubsystem>())
		{
			if (CrewSubsystem->IsInCrew())
			{
				const FMGCrewInfo& CurrentCrew = CrewSubsystem->GetCurrentCrew();

				// Check if this player is the crew leader
				FName PlayerIDName(*PlayerID.ToString());
				if (CurrentCrew.LeaderID == PlayerIDName)
				{
					return true;
				}

				// Check if player is an officer or higher rank
				FMGCrewMember Member = CrewSubsystem->GetMember(PlayerIDName);
				if (Member.Rank >= EMGCrewRank::Officer)
				{
					return true;
				}
			}
		}
	}

	return false;
}

FGuid UMGMeetSpotSubsystem::FindNearestFacingPlayer(FGuid PlayerID, FGuid InstanceID, float MaxDistance) const
{
	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return FGuid();
	}

	const FMGMeetSpotPlayer* Player = FindPlayerConst(InstanceID, PlayerID);
	if (!Player)
	{
		return FGuid();
	}

	FGuid NearestID;
	float NearestDistSq = MaxDistance * MaxDistance;

	// Get player's parking spot for direction
	int32 SpotIndex = Player->ParkingSpotIndex;
	FVector PlayerLocation = Player->CurrentLocation;
	FVector ForwardDir = FVector::ForwardVector;

	// If in a parking spot, use the spot's rotation for forward direction
	if (SpotIndex >= 0 && SpotIndex < Instance->ParkingSpots.Num())
	{
		ForwardDir = Instance->ParkingSpots[SpotIndex].Rotation.Vector();
		PlayerLocation = Instance->ParkingSpots[SpotIndex].Location;
	}

	for (const FMGMeetSpotPlayer& OtherPlayer : Instance->Players)
	{
		if (OtherPlayer.PlayerID == PlayerID)
		{
			continue;
		}

		FVector OtherLocation = OtherPlayer.CurrentLocation;

		// If other player is in a parking spot, use spot location
		if (OtherPlayer.ParkingSpotIndex >= 0 && OtherPlayer.ParkingSpotIndex < Instance->ParkingSpots.Num())
		{
			OtherLocation = Instance->ParkingSpots[OtherPlayer.ParkingSpotIndex].Location;
		}

		FVector ToOther = OtherLocation - PlayerLocation;
		float DistSq = ToOther.SizeSquared();

		// Check distance
		if (DistSq > NearestDistSq)
		{
			continue;
		}

		// Check if facing (within 45 degrees)
		ToOther.Normalize();
		float DotProduct = FVector::DotProduct(ForwardDir, ToOther);
		if (DotProduct > 0.707f) // cos(45 degrees)
		{
			NearestDistSq = DistSq;
			NearestID = OtherPlayer.PlayerID;
		}
	}

	return NearestID;
}

TArray<FGuid> UMGMeetSpotSubsystem::GetPlayersInRange(FGuid InstanceID, FVector Position, float Range) const
{
	TArray<FGuid> PlayersInRange;

	const FMGMeetSpotInstance* Instance = ActiveInstances.Find(InstanceID);
	if (!Instance)
	{
		return PlayersInRange;
	}

	float RangeSq = Range * Range;

	for (const FMGMeetSpotPlayer& Player : Instance->Players)
	{
		FVector PlayerLocation = Player.CurrentLocation;

		// If in a parking spot, use spot location
		if (Player.ParkingSpotIndex >= 0 && Player.ParkingSpotIndex < Instance->ParkingSpots.Num())
		{
			PlayerLocation = Instance->ParkingSpots[Player.ParkingSpotIndex].Location;
		}

		float DistSq = FVector::DistSquared(Position, PlayerLocation);
		if (DistSq <= RangeSq)
		{
			PlayersInRange.Add(Player.PlayerID);
		}
	}

	return PlayersInRange;
}

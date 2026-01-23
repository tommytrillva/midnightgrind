// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMeetSpotSubsystem.generated.h"

class AMGVehiclePawn;

/**
 * Zone type within a meet spot
 */
UENUM(BlueprintType)
enum class EMGMeetSpotZone : uint8
{
	MainParking UMETA(DisplayName = "Main Parking"),
	ShowcaseStage UMETA(DisplayName = "Showcase Stage"),
	PhotoSpot UMETA(DisplayName = "Photo Spot"),
	VendorArea UMETA(DisplayName = "Vendor Area"),
	RaceTerminal UMETA(DisplayName = "Race Terminal"),
	CrewParking UMETA(DisplayName = "Crew Parking"),
	Entrance UMETA(DisplayName = "Entrance"),
	DriftPad UMETA(DisplayName = "Drift Pad"),
	BuildShowcase UMETA(DisplayName = "Build Showcase")
};

/**
 * Meet spot instance state
 */
UENUM(BlueprintType)
enum class EMGMeetSpotState : uint8
{
	Open UMETA(DisplayName = "Open"),
	Full UMETA(DisplayName = "Full"),
	Event UMETA(DisplayName = "Event In Progress"),
	Private UMETA(DisplayName = "Private/Crew Only"),
	Maintenance UMETA(DisplayName = "Maintenance")
};

/**
 * NPC vendor type
 */
UENUM(BlueprintType)
enum class EMGVendorType : uint8
{
	PartsSeller UMETA(DisplayName = "Parts Seller"),
	TuningShop UMETA(DisplayName = "Tuning Shop"),
	CosmeticsVendor UMETA(DisplayName = "Cosmetics Vendor"),
	NitrousRefill UMETA(DisplayName = "Nitrous Refill"),
	TireShop UMETA(DisplayName = "Tire Shop"),
	RepairService UMETA(DisplayName = "Repair Service"),
	Photographer UMETA(DisplayName = "Photographer"),
	FoodTruck UMETA(DisplayName = "Food Truck") // Ambiance
};

/**
 * Event type at meet spot
 */
UENUM(BlueprintType)
enum class EMGMeetSpotEventType : uint8
{
	None UMETA(DisplayName = "None"),
	CarShow UMETA(DisplayName = "Car Show"),
	DriftSession UMETA(DisplayName = "Drift Session"),
	BuildShowcase UMETA(DisplayName = "Build Showcase"),
	CrewMeet UMETA(DisplayName = "Crew Meet"),
	PhotoShoot UMETA(DisplayName = "Photo Shoot"),
	RaceMeet UMETA(DisplayName = "Race Organization")
};

/**
 * Parking spot information
 */
USTRUCT(BlueprintType)
struct FMGParkingSpot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMeetSpotZone Zone = EMGMeetSpotZone::MainParking;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid OccupantPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid OccupantVehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReserved = false; // For crew spots

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReservedForCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpotlightEnabled = false; // For showcase
};

/**
 * NPC vendor instance
 */
USTRUCT(BlueprintType)
struct FMGVendorInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VendorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVendorType Type = EMGVendorType::PartsSeller;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiscountPercent = 0.0f; // Special event discounts

	FMGVendorInstance()
	{
		VendorID = FGuid::NewGuid();
	}
};

/**
 * Photo spot location
 */
USTRUCT(BlueprintType)
struct FMGPhotoSpot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector VehicleLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator VehicleRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> CameraPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LightingPreset; // Day, Night, Neon, Studio, etc.

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueueWaitTime = 0.0f;
};

/**
 * Meet spot event
 */
USTRUCT(BlueprintType)
struct FMGMeetSpotEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMeetSpotEventType Type = EMGMeetSpotEventType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText EventName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid OrganizerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OrganizerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxParticipants = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> RegisteredParticipants;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCrewOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid RequiredCrewID;

	// Car show specifics
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShowTheme; // "JDM Night", "Muscle Monday", "Euro Classics"

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PIMinimum = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PIMaximum = 999.0f;

	FMGMeetSpotEvent()
	{
		EventID = FGuid::NewGuid();
	}
};

/**
 * Player in meet spot instance
 */
USTRUCT(BlueprintType)
struct FMGMeetSpotPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PerformanceIndex = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ParkingSpotIndex = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShowcasing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ShowcaseVotes = 0;
};

/**
 * Meet spot instance data
 */
USTRUCT(BlueprintType)
struct FMGMeetSpotInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MeetSpotID; // "DowntownMeet", "HighwayRest", "MountainOverlook"

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMeetSpotState State = EMGMeetSpotState::Open;

	// Capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayerCount = 0;

	// Players
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMeetSpotPlayer> Players;

	// Infrastructure
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGParkingSpot> ParkingSpots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGVendorInstance> Vendors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPhotoSpot> PhotoSpots;

	// Events
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGMeetSpotEvent CurrentEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMeetSpotEvent> UpcomingEvents;

	// Showcase
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> ShowcaseQueue; // Players waiting to showcase

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CurrentShowcasePlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ShowcaseEndTime;

	// Crew reservations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGuid, int32> CrewReservedSpots; // CrewID -> Number of spots

	FMGMeetSpotInstance()
	{
		InstanceID = FGuid::NewGuid();
	}
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinedMeetSpot, const FGuid&, InstanceID, const FMGMeetSpotPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeftMeetSpot, const FGuid&, InstanceID, const FGuid&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeetSpotEventStarted, const FGuid&, InstanceID, const FMGMeetSpotEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMeetSpotEventEnded, const FGuid&, InstanceID, const FGuid&, EventID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowcaseStarted, const FGuid&, InstanceID, const FMGMeetSpotPlayer&, ShowcasePlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShowcaseVote, const FGuid&, InstanceID, const FGuid&, VoterID, int32, NewVoteCount);

/**
 * Meet Spot Social Hub Subsystem
 * Manages social gathering spaces per PRD Section 2.1
 * Supports 200-player instances with showcase, photo, and race organization features
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMeetSpotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// INSTANCE MANAGEMENT
	// ==========================================

	/**
	 * Find or create a meet spot instance
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot")
	FGuid FindOrCreateInstance(FName MeetSpotID, FGuid PreferredCrewID = FGuid());

	/**
	 * Get instance info
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	bool GetInstanceInfo(FGuid InstanceID, FMGMeetSpotInstance& OutInstance) const;

	/**
	 * Get all active instances for a meet spot
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	TArray<FMGMeetSpotInstance> GetActiveInstances(FName MeetSpotID) const;

	/**
	 * Get instance with crew members
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	FGuid FindInstanceWithCrewMembers(FName MeetSpotID, FGuid CrewID) const;

	// ==========================================
	// JOINING & LEAVING
	// ==========================================

	/**
	 * Join a meet spot instance
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot")
	bool JoinMeetSpot(FGuid PlayerID, FGuid InstanceID, FGuid VehicleID, int32& OutParkingSpot);

	/**
	 * Leave current meet spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot")
	void LeaveMeetSpot(FGuid PlayerID);

	/**
	 * Get player's current meet spot instance
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	FGuid GetPlayerMeetSpot(FGuid PlayerID) const;

	/**
	 * Check if player is in any meet spot
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	bool IsPlayerInMeetSpot(FGuid PlayerID) const;

	// ==========================================
	// PARKING
	// ==========================================

	/**
	 * Request a specific parking spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Parking")
	bool RequestParkingSpot(FGuid PlayerID, FGuid InstanceID, int32 SpotIndex);

	/**
	 * Find nearest available parking spot
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Parking")
	int32 FindNearestAvailableSpot(FGuid InstanceID, FVector PlayerLocation, EMGMeetSpotZone PreferredZone = EMGMeetSpotZone::MainParking) const;

	/**
	 * Leave parking spot (stay in meet but move to driving)
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Parking")
	void LeaveParkingSpot(FGuid PlayerID);

	/**
	 * Get available spots in zone
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Parking")
	TArray<FMGParkingSpot> GetAvailableSpots(FGuid InstanceID, EMGMeetSpotZone Zone) const;

	// ==========================================
	// SHOWCASE
	// ==========================================

	/**
	 * Join showcase queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	bool JoinShowcaseQueue(FGuid PlayerID);

	/**
	 * Leave showcase queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	void LeaveShowcaseQueue(FGuid PlayerID);

	/**
	 * Get position in showcase queue
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Showcase")
	int32 GetShowcaseQueuePosition(FGuid PlayerID) const;

	/**
	 * Vote for showcased vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	bool VoteForShowcase(FGuid VoterID);

	/**
	 * Get current showcase info
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Showcase")
	bool GetCurrentShowcase(FGuid InstanceID, FMGMeetSpotPlayer& OutShowcasePlayer) const;

	/**
	 * Skip current showcase (moderator only)
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	void SkipCurrentShowcase(FGuid ModeratorID, FGuid InstanceID);

	// ==========================================
	// PHOTO SPOTS
	// ==========================================

	/**
	 * Queue for photo spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Photo")
	bool QueueForPhotoSpot(FGuid PlayerID, int32 PhotoSpotIndex);

	/**
	 * Leave photo spot queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Photo")
	void LeavePhotoSpotQueue(FGuid PlayerID);

	/**
	 * Get available photo spots
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Photo")
	TArray<FMGPhotoSpot> GetPhotoSpots(FGuid InstanceID) const;

	/**
	 * Set photo spot lighting
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Photo")
	void SetPhotoSpotLighting(FGuid InstanceID, int32 SpotIndex, FName LightingPreset);

	// ==========================================
	// VENDORS
	// ==========================================

	/**
	 * Get vendors in instance
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Vendors")
	TArray<FMGVendorInstance> GetVendors(FGuid InstanceID) const;

	/**
	 * Get vendors by type
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Vendors")
	TArray<FMGVendorInstance> GetVendorsByType(FGuid InstanceID, EMGVendorType Type) const;

	/**
	 * Interact with vendor (opens shop UI)
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Vendors")
	bool InteractWithVendor(FGuid PlayerID, FGuid VendorID);

	// ==========================================
	// EVENTS
	// ==========================================

	/**
	 * Create an event at meet spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool CreateEvent(FGuid OrganizerID, FGuid InstanceID, const FMGMeetSpotEvent& EventData, FGuid& OutEventID);

	/**
	 * Register for event
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool RegisterForEvent(FGuid PlayerID, FGuid EventID);

	/**
	 * Unregister from event
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	void UnregisterFromEvent(FGuid PlayerID, FGuid EventID);

	/**
	 * Start event (organizer only)
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool StartEvent(FGuid OrganizerID, FGuid EventID);

	/**
	 * End event
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool EndEvent(FGuid OrganizerID, FGuid EventID);

	/**
	 * Get upcoming events
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Events")
	TArray<FMGMeetSpotEvent> GetUpcomingEvents(FGuid InstanceID) const;

	// ==========================================
	// CREW FEATURES
	// ==========================================

	/**
	 * Reserve crew parking spots
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Crew")
	bool ReserveCrewSpots(FGuid CrewLeaderID, FGuid InstanceID, int32 NumSpots);

	/**
	 * Release crew parking spots
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Crew")
	void ReleaseCrewSpots(FGuid CrewLeaderID, FGuid InstanceID);

	/**
	 * Get crew members in meet spot
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Crew")
	TArray<FMGMeetSpotPlayer> GetCrewMembersInMeetSpot(FGuid InstanceID, FGuid CrewID) const;

	// ==========================================
	// RACE ORGANIZATION
	// ==========================================

	/**
	 * Propose a race from meet spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Race")
	bool ProposeRace(FGuid OrganizerID, FName RaceType, float PILimit, int32 MaxParticipants, int64 EntryFee);

	/**
	 * Accept race proposal
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Race")
	bool AcceptRaceProposal(FGuid PlayerID, FGuid RaceProposalID);

	/**
	 * Start race from meet spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Race")
	bool LaunchRace(FGuid OrganizerID, FGuid RaceProposalID);

	// ==========================================
	// SOCIAL
	// ==========================================

	/**
	 * Send proximity chat message
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void SendProximityMessage(FGuid SenderID, const FString& Message, float Range = 2000.0f);

	/**
	 * Use emote
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void UseEmote(FGuid PlayerID, FName EmoteID);

	/**
	 * Flash headlights (challenge)
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void FlashHeadlights(FGuid PlayerID);

	/**
	 * Rev engine
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void RevEngine(FGuid PlayerID);

	// ==========================================
	// EVENTS (DELEGATES)
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnPlayerJoinedMeetSpot OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnPlayerLeftMeetSpot OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnMeetSpotEventStarted OnEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnMeetSpotEventEnded OnEventEnded;

	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnShowcaseStarted OnShowcaseStarted;

	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnShowcaseVote OnShowcaseVote;

protected:
	// Active instances
	UPROPERTY()
	TMap<FGuid, FMGMeetSpotInstance> ActiveInstances;

	// Player to instance mapping
	TMap<FGuid, FGuid> PlayerInstanceMap;

	// Photo spot queues (SpotIndex -> PlayerQueue)
	TMap<int32, TArray<FGuid>> PhotoSpotQueues;

	// Timer for instance updates
	FTimerHandle UpdateTimer;

	// Showcase duration (seconds)
	static constexpr float ShowcaseDuration = 120.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	void OnUpdateTick();
	void UpdateShowcases();
	void UpdatePhotoSpots();
	void CleanupEmptyInstances();
	FMGMeetSpotInstance CreateNewInstance(FName MeetSpotID);
	void SetupDefaultInfrastructure(FMGMeetSpotInstance& Instance);
	void AdvanceShowcaseQueue(FMGMeetSpotInstance& Instance);
	bool IsCrewMember(FGuid PlayerID, FGuid CrewID) const;
	FMGMeetSpotPlayer* FindPlayer(FGuid InstanceID, FGuid PlayerID);
	const FMGMeetSpotPlayer* FindPlayerConst(FGuid InstanceID, FGuid PlayerID) const;
};

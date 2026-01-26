// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGMeetSpotSubsystem.h
 * @brief Meet Spot Social Hub Subsystem for car culture gatherings
 *
 * Meet spots are social hubs where car culture comes alive. They provide:
 * - Social gathering spaces for 200+ players
 * - Vehicle showcasing and voting systems
 * - Race challenge organization
 * - Photo opportunities with various lighting presets
 * - Reputation building through social presence
 * - Emote and communication systems
 *
 * Per GDD Section 3.2: "The Meet" - Industrial areas, large parking lots,
 * scenic overlooks, and historic spots with legacy.
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMeetSpotSubsystem.generated.h"

class AMGVehiclePawn;
class UMGReputationSubsystem;
class UMGSocialSubsystem;

/**
 * @brief Meet spot location type
 * Per GDD Section 3.2: Location Selection for car meets
 */
UENUM(BlueprintType)
enum class EMGMeetSpotLocationType : uint8
{
	/** Industrial area - low traffic, minimal residents */
	Industrial UMETA(DisplayName = "Industrial Area"),
	/** Large parking lot - shopping centers after hours */
	ParkingLot UMETA(DisplayName = "Parking Lot"),
	/** Scenic overlook - canyon views, city skylines */
	Overlook UMETA(DisplayName = "Scenic Overlook"),
	/** Historic spot - known locations with legacy */
	Historic UMETA(DisplayName = "Historic Spot"),
	/** Private property - permitted events */
	Private UMETA(DisplayName = "Private Property"),
	/** Highway rest stop - highway battle staging */
	RestStop UMETA(DisplayName = "Highway Rest Stop"),
	/** Waterfront/port area - docks, water views */
	Waterfront UMETA(DisplayName = "Waterfront")
};

/**
 * @brief Zone type within a meet spot
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
	BuildShowcase UMETA(DisplayName = "Build Showcase"),
	SocialLounge UMETA(DisplayName = "Social Lounge")
};

/**
 * @brief Meet spot instance state
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
 * @brief NPC vendor type
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
	FoodTruck UMETA(DisplayName = "Food Truck")
};

/**
 * @brief Event type at meet spot
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
	RaceMeet UMETA(DisplayName = "Race Organization"),
	ThemeNight UMETA(DisplayName = "Theme Night")
};

/**
 * @brief Emote category for social interactions
 */
UENUM(BlueprintType)
enum class EMGEmoteCategory : uint8
{
	/** Greeting emotes (wave, nod, etc.) */
	Greeting UMETA(DisplayName = "Greeting"),
	/** Celebration emotes (cheer, fist pump, etc.) */
	Celebration UMETA(DisplayName = "Celebration"),
	/** Respect emotes (bow, clap, etc.) */
	Respect UMETA(DisplayName = "Respect"),
	/** Taunt emotes (flex, dismiss, etc.) */
	Taunt UMETA(DisplayName = "Taunt"),
	/** Dance emotes */
	Dance UMETA(DisplayName = "Dance"),
	/** Vehicle interactions (lean on car, inspect engine, etc.) */
	VehicleInteraction UMETA(DisplayName = "Vehicle Interaction")
};

/**
 * @brief Challenge type for race challenges at meet spots
 */
UENUM(BlueprintType)
enum class EMGRaceChallengeType : uint8
{
	/** Friendly race, no stakes */
	Friendly UMETA(DisplayName = "Friendly"),
	/** Cash wager race */
	CashWager UMETA(DisplayName = "Cash Wager"),
	/** Pink slip race - winner takes loser's car */
	PinkSlip UMETA(DisplayName = "Pink Slip"),
	/** Crew vs crew race */
	CrewBattle UMETA(DisplayName = "Crew Battle"),
	/** Reputation on the line */
	ReputationStakes UMETA(DisplayName = "Reputation Stakes")
};

/**
 * @brief Horn communication pattern
 * Per GDD: Car horn communication is part of street racing culture
 */
UENUM(BlueprintType)
enum class EMGHornPattern : uint8
{
	/** Single short honk - greeting/acknowledgment */
	SingleShort UMETA(DisplayName = "Single Short"),
	/** Double short honk - challenge/attention */
	DoubleShort UMETA(DisplayName = "Double Short"),
	/** Long honk - warning/anger */
	LongHonk UMETA(DisplayName = "Long Honk"),
	/** Two short one long - let's go/rally */
	TwoShortOneLong UMETA(DisplayName = "Two Short One Long"),
	/** Rapid honks - celebration */
	RapidCelebration UMETA(DisplayName = "Rapid Celebration")
};

/**
 * @brief Social emote definition
 */
USTRUCT(BlueprintType)
struct FMGSocialEmote
{
	GENERATED_BODY()

	/** Unique emote identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EmoteID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Emote category */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGEmoteCategory Category = EMGEmoteCategory::Greeting;

	/** Animation montage to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UAnimMontage> AnimMontage;

	/** Duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 2.0f;

	/** Reputation tier required to unlock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReputationTier = 0;

	/** Is this emote unlocked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUnlocked = true;
};

/**
 * @brief Meet spot location definition
 */
USTRUCT(BlueprintType)
struct FMGMeetSpotLocation
{
	GENERATED_BODY()

	/** Unique location identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LocationID;

	/** Display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Description text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Location type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMeetSpotLocationType LocationType = EMGMeetSpotLocationType::ParkingLot;

	/** World position */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	/** District/area this location is in */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DistrictID;

	/** Maximum player capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxCapacity = 200;

	/** Reputation tier required to access */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredReputationTier = 0;

	/** Is this a legendary/historic spot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLegendarySpot = false;

	/** Ambient audio preset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AmbientAudioPreset;

	/** Lighting preset for night scenes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LightingPreset;
};

/**
 * @brief Parking spot information
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

	/** Reserved for crew spots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bReserved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReservedForCrewID;

	/** Spotlight enabled for showcase */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpotlightEnabled = false;

	/** Premium spot (closer to action) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPremiumSpot = false;
};

/**
 * @brief NPC vendor instance
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

	/** Special event discounts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DiscountPercent = 0.0f;

	FMGVendorInstance()
	{
		VendorID = FGuid::NewGuid();
	}
};

/**
 * @brief Photo spot location with camera presets
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

	/** Lighting preset: Day, Night, Neon, Studio, Sunset, etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName LightingPreset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccupied = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float QueueWaitTime = 0.0f;

	/** Backdrop type for photo mode */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BackdropType;
};

/**
 * @brief Race challenge proposal at meet spot
 */
USTRUCT(BlueprintType)
struct FMGRaceChallenge
{
	GENERATED_BODY()

	/** Unique challenge identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ChallengeID;

	/** Challenger player ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ChallengerID;

	/** Challenger display name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChallengerName;

	/** Target player ID (can be empty for open challenge) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid TargetID;

	/** Challenge type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRaceChallengeType ChallengeType = EMGRaceChallengeType::Friendly;

	/** Race type (Sprint, Drag, Touge, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceType;

	/** Track/route ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	/** Performance Index limit (0 = no limit) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PILimit = 0.0f;

	/** Wager amount (cash or reputation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 WagerAmount = 0;

	/** Maximum participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxParticipants = 2;

	/** Accepted participants */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> AcceptedParticipants;

	/** Challenge creation time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	/** Challenge expiration time */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpirationTime;

	/** Is this an open challenge anyone can join */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOpenChallenge = false;

	FMGRaceChallenge()
	{
		ChallengeID = FGuid::NewGuid();
		CreatedTime = FDateTime::Now();
		ExpirationTime = CreatedTime + FTimespan::FromMinutes(5);
	}
};

/**
 * @brief Meet spot event
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

	/** Car show theme: "JDM Night", "Muscle Monday", "Euro Classics" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShowTheme;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PIMinimum = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PIMaximum = 999.0f;

	/** Reputation reward for participating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationReward = 0;

	FMGMeetSpotEvent()
	{
		EventID = FGuid::NewGuid();
	}
};

/**
 * @brief Player respect/interaction tracking for reputation
 */
USTRUCT(BlueprintType)
struct FMGPlayerRespect
{
	GENERATED_BODY()

	/** Player who gave respect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid FromPlayerID;

	/** Player who received respect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ToPlayerID;

	/** Type of respect action */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RespectType;

	/** Timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * @brief Player in meet spot instance
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

	/** Player's reputation tier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationTier = 0;

	/** Player's title */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Title;

	/** Total respect received this session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RespectReceived = 0;

	/** Current location within meet spot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector CurrentLocation = FVector::ZeroVector;

	/** Is player AFK */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAFK = false;

	/** Currently playing emote */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentEmote;
};

/**
 * @brief Meet spot instance data
 */
USTRUCT(BlueprintType)
struct FMGMeetSpotInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid InstanceID;

	/** Location identifier: "DowntownMeet", "HighwayRest", "MountainOverlook" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MeetSpotID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMeetSpotState State = EMGMeetSpotState::Open;

	/** Location type */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMeetSpotLocationType LocationType = EMGMeetSpotLocationType::ParkingLot;

	/** Capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayerCount = 0;

	/** Players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMeetSpotPlayer> Players;

	/** Infrastructure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGParkingSpot> ParkingSpots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGVendorInstance> Vendors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPhotoSpot> PhotoSpots;

	/** Current event */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGMeetSpotEvent CurrentEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMeetSpotEvent> UpcomingEvents;

	/** Active race challenges */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGRaceChallenge> ActiveChallenges;

	/** Showcase queue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> ShowcaseQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid CurrentShowcasePlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ShowcaseEndTime;

	/** Crew reservations: CrewID -> Number of spots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGuid, int32> CrewReservedSpots;

	/** Ambient vibe level (0-100, affects reputation gains) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VibeLevel = 50;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceChallengeCreated, const FGuid&, InstanceID, const FMGRaceChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceChallengeAccepted, const FGuid&, ChallengeID, const FGuid&, AcceptorID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEmotePlayed, const FGuid&, InstanceID, const FGuid&, PlayerID, FName, EmoteID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHornPlayed, const FGuid&, InstanceID, const FGuid&, PlayerID, EMGHornPattern, Pattern);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRespectGiven, const FGuid&, FromPlayerID, const FGuid&, ToPlayerID, int32, TotalRespect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVibeChanged, const FGuid&, InstanceID, int32, NewVibeLevel);

/**
 * @class UMGMeetSpotSubsystem
 * @brief Meet Spot Social Hub Subsystem
 *
 * Manages social gathering spaces per GDD Section 3.2 and Design Pillar 4: Living Car Culture.
 * Supports 200-player instances with showcase, photo, challenge, and social interaction features.
 *
 * Key Features:
 * - Meet spot locations with different vibes (industrial, overlook, historic)
 * - Car shows and build showcases with voting
 * - Race challenge system (friendly to pink slip)
 * - Social interactions (emotes, horn patterns, respect giving)
 * - Reputation building through presence and recognition
 * - Photo spots with lighting presets
 * - Crew parking reservations
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
	 * @brief Find or create a meet spot instance
	 * @param MeetSpotID The meet spot location identifier
	 * @param PreferredCrewID Optional crew ID to find instance with crew members
	 * @return Instance ID
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot")
	FGuid FindOrCreateInstance(FName MeetSpotID, FGuid PreferredCrewID = FGuid());

	/**
	 * @brief Get instance info
	 * @param InstanceID The instance to query
	 * @param OutInstance Output instance data
	 * @return True if instance found
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	bool GetInstanceInfo(FGuid InstanceID, FMGMeetSpotInstance& OutInstance) const;

	/**
	 * @brief Get all active instances for a meet spot
	 * @param MeetSpotID The meet spot location identifier
	 * @return Array of active instances
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	TArray<FMGMeetSpotInstance> GetActiveInstances(FName MeetSpotID) const;

	/**
	 * @brief Get instance with crew members
	 * @param MeetSpotID The meet spot location identifier
	 * @param CrewID The crew to search for
	 * @return Instance ID or invalid GUID if not found
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	FGuid FindInstanceWithCrewMembers(FName MeetSpotID, FGuid CrewID) const;

	/**
	 * @brief Get all available meet spot locations
	 * @return Array of defined meet spot locations
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	TArray<FMGMeetSpotLocation> GetAllMeetSpotLocations() const { return MeetSpotLocations; }

	/**
	 * @brief Get meet spots accessible at player's reputation tier
	 * @param ReputationTier Player's current reputation tier
	 * @return Array of accessible meet spot locations
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	TArray<FMGMeetSpotLocation> GetAccessibleMeetSpots(int32 ReputationTier) const;

	// ==========================================
	// JOINING & LEAVING
	// ==========================================

	/**
	 * @brief Join a meet spot instance
	 * @param PlayerID The player joining
	 * @param InstanceID The instance to join
	 * @param VehicleID The vehicle the player is bringing
	 * @param OutParkingSpot Assigned parking spot index
	 * @return True if successfully joined
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot")
	bool JoinMeetSpot(FGuid PlayerID, FGuid InstanceID, FGuid VehicleID, int32& OutParkingSpot);

	/**
	 * @brief Leave current meet spot
	 * @param PlayerID The player leaving
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot")
	void LeaveMeetSpot(FGuid PlayerID);

	/**
	 * @brief Get player's current meet spot instance
	 * @param PlayerID The player to query
	 * @return Instance ID or invalid GUID if not in a meet spot
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	FGuid GetPlayerMeetSpot(FGuid PlayerID) const;

	/**
	 * @brief Check if player is in any meet spot
	 * @param PlayerID The player to check
	 * @return True if player is in a meet spot
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot")
	bool IsPlayerInMeetSpot(FGuid PlayerID) const;

	// ==========================================
	// PARKING
	// ==========================================

	/**
	 * @brief Request a specific parking spot
	 * @param PlayerID The requesting player
	 * @param InstanceID The meet spot instance
	 * @param SpotIndex The desired spot index
	 * @return True if spot was assigned
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Parking")
	bool RequestParkingSpot(FGuid PlayerID, FGuid InstanceID, int32 SpotIndex);

	/**
	 * @brief Find nearest available parking spot
	 * @param InstanceID The meet spot instance
	 * @param PlayerLocation Player's current location
	 * @param PreferredZone Preferred zone type
	 * @return Spot index or -1 if none available
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Parking")
	int32 FindNearestAvailableSpot(FGuid InstanceID, FVector PlayerLocation, EMGMeetSpotZone PreferredZone = EMGMeetSpotZone::MainParking) const;

	/**
	 * @brief Leave parking spot (stay in meet but move to driving)
	 * @param PlayerID The player leaving their spot
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Parking")
	void LeaveParkingSpot(FGuid PlayerID);

	/**
	 * @brief Get available spots in zone
	 * @param InstanceID The meet spot instance
	 * @param Zone The zone to query
	 * @return Array of available parking spots
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Parking")
	TArray<FMGParkingSpot> GetAvailableSpots(FGuid InstanceID, EMGMeetSpotZone Zone) const;

	// ==========================================
	// SHOWCASE
	// ==========================================

	/**
	 * @brief Join showcase queue
	 * @param PlayerID The player wanting to showcase
	 * @return True if added to queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	bool JoinShowcaseQueue(FGuid PlayerID);

	/**
	 * @brief Leave showcase queue
	 * @param PlayerID The player leaving queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	void LeaveShowcaseQueue(FGuid PlayerID);

	/**
	 * @brief Get position in showcase queue
	 * @param PlayerID The player to check
	 * @return Queue position (0-based) or -1 if not in queue
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Showcase")
	int32 GetShowcaseQueuePosition(FGuid PlayerID) const;

	/**
	 * @brief Vote for showcased vehicle
	 * @param VoterID The voting player
	 * @return True if vote was counted
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	bool VoteForShowcase(FGuid VoterID);

	/**
	 * @brief Get current showcase info
	 * @param InstanceID The meet spot instance
	 * @param OutShowcasePlayer The player currently showcasing
	 * @return True if someone is currently showcasing
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Showcase")
	bool GetCurrentShowcase(FGuid InstanceID, FMGMeetSpotPlayer& OutShowcasePlayer) const;

	/**
	 * @brief Skip current showcase (moderator only)
	 * @param ModeratorID The moderator requesting skip
	 * @param InstanceID The meet spot instance
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Showcase")
	void SkipCurrentShowcase(FGuid ModeratorID, FGuid InstanceID);

	// ==========================================
	// PHOTO SPOTS
	// ==========================================

	/**
	 * @brief Queue for photo spot
	 * @param PlayerID The requesting player
	 * @param PhotoSpotIndex The desired photo spot
	 * @return True if added to queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Photo")
	bool QueueForPhotoSpot(FGuid PlayerID, int32 PhotoSpotIndex);

	/**
	 * @brief Leave photo spot queue
	 * @param PlayerID The player leaving queue
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Photo")
	void LeavePhotoSpotQueue(FGuid PlayerID);

	/**
	 * @brief Get available photo spots
	 * @param InstanceID The meet spot instance
	 * @return Array of photo spots
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Photo")
	TArray<FMGPhotoSpot> GetPhotoSpots(FGuid InstanceID) const;

	/**
	 * @brief Set photo spot lighting preset
	 * @param InstanceID The meet spot instance
	 * @param SpotIndex The photo spot index
	 * @param LightingPreset The lighting preset name
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Photo")
	void SetPhotoSpotLighting(FGuid InstanceID, int32 SpotIndex, FName LightingPreset);

	// ==========================================
	// VENDORS
	// ==========================================

	/**
	 * @brief Get vendors in instance
	 * @param InstanceID The meet spot instance
	 * @return Array of vendors
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Vendors")
	TArray<FMGVendorInstance> GetVendors(FGuid InstanceID) const;

	/**
	 * @brief Get vendors by type
	 * @param InstanceID The meet spot instance
	 * @param Type The vendor type to filter
	 * @return Array of vendors of specified type
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Vendors")
	TArray<FMGVendorInstance> GetVendorsByType(FGuid InstanceID, EMGVendorType Type) const;

	/**
	 * @brief Interact with vendor (opens shop UI)
	 * @param PlayerID The interacting player
	 * @param VendorID The vendor to interact with
	 * @return True if interaction started
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Vendors")
	bool InteractWithVendor(FGuid PlayerID, FGuid VendorID);

	// ==========================================
	// EVENTS
	// ==========================================

	/**
	 * @brief Create an event at meet spot
	 * @param OrganizerID The organizing player
	 * @param InstanceID The meet spot instance
	 * @param EventData The event configuration
	 * @param OutEventID The created event's ID
	 * @return True if event was created
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool CreateEvent(FGuid OrganizerID, FGuid InstanceID, const FMGMeetSpotEvent& EventData, FGuid& OutEventID);

	/**
	 * @brief Register for event
	 * @param PlayerID The registering player
	 * @param EventID The event to register for
	 * @return True if registration successful
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool RegisterForEvent(FGuid PlayerID, FGuid EventID);

	/**
	 * @brief Unregister from event
	 * @param PlayerID The unregistering player
	 * @param EventID The event to unregister from
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	void UnregisterFromEvent(FGuid PlayerID, FGuid EventID);

	/**
	 * @brief Start event (organizer only)
	 * @param OrganizerID The organizing player
	 * @param EventID The event to start
	 * @return True if event started
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool StartEvent(FGuid OrganizerID, FGuid EventID);

	/**
	 * @brief End event
	 * @param OrganizerID The organizing player
	 * @param EventID The event to end
	 * @return True if event ended
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Events")
	bool EndEvent(FGuid OrganizerID, FGuid EventID);

	/**
	 * @brief Get upcoming events
	 * @param InstanceID The meet spot instance
	 * @return Array of upcoming events
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Events")
	TArray<FMGMeetSpotEvent> GetUpcomingEvents(FGuid InstanceID) const;

	// ==========================================
	// RACE CHALLENGES (NEW)
	// ==========================================

	/**
	 * @brief Create a race challenge at meet spot
	 * @param ChallengerID The challenging player
	 * @param ChallengeType Type of challenge (friendly, wager, pink slip)
	 * @param RaceType Type of race (sprint, drag, touge)
	 * @param TrackID Optional specific track
	 * @param PILimit Performance Index limit (0 = no limit)
	 * @param WagerAmount Wager amount for cash/rep challenges
	 * @param TargetID Optional specific target player
	 * @param bIsOpen True if anyone can accept
	 * @param OutChallengeID The created challenge ID
	 * @return True if challenge was created
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Challenge")
	bool CreateRaceChallenge(
		FGuid ChallengerID,
		EMGRaceChallengeType ChallengeType,
		FName RaceType,
		FName TrackID,
		float PILimit,
		int64 WagerAmount,
		FGuid TargetID,
		bool bIsOpen,
		FGuid& OutChallengeID
	);

	/**
	 * @brief Accept a race challenge
	 * @param PlayerID The accepting player
	 * @param ChallengeID The challenge to accept
	 * @return True if challenge was accepted
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Challenge")
	bool AcceptRaceChallenge(FGuid PlayerID, FGuid ChallengeID);

	/**
	 * @brief Cancel a race challenge (challenger only)
	 * @param PlayerID The canceling player (must be challenger)
	 * @param ChallengeID The challenge to cancel
	 * @return True if challenge was canceled
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Challenge")
	bool CancelRaceChallenge(FGuid PlayerID, FGuid ChallengeID);

	/**
	 * @brief Launch a race from accepted challenge
	 * @param ChallengerID The challenger launching the race
	 * @param ChallengeID The challenge to launch
	 * @return True if race was launched
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Challenge")
	bool LaunchChallengeRace(FGuid ChallengerID, FGuid ChallengeID);

	/**
	 * @brief Get active challenges in instance
	 * @param InstanceID The meet spot instance
	 * @return Array of active challenges
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Challenge")
	TArray<FMGRaceChallenge> GetActiveChallenges(FGuid InstanceID) const;

	/**
	 * @brief Get challenges targeting a specific player
	 * @param PlayerID The targeted player
	 * @return Array of challenges targeting this player
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Challenge")
	TArray<FMGRaceChallenge> GetChallengesForPlayer(FGuid PlayerID) const;

	// Legacy race organization methods
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Race")
	bool ProposeRace(FGuid OrganizerID, FName RaceType, float PILimit, int32 MaxParticipants, int64 EntryFee);

	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Race")
	bool AcceptRaceProposal(FGuid PlayerID, FGuid RaceProposalID);

	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Race")
	bool LaunchRace(FGuid OrganizerID, FGuid RaceProposalID);

	// ==========================================
	// CREW FEATURES
	// ==========================================

	/**
	 * @brief Reserve crew parking spots
	 * @param CrewLeaderID The crew leader
	 * @param InstanceID The meet spot instance
	 * @param NumSpots Number of spots to reserve
	 * @return True if spots were reserved
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Crew")
	bool ReserveCrewSpots(FGuid CrewLeaderID, FGuid InstanceID, int32 NumSpots);

	/**
	 * @brief Release crew parking spots
	 * @param CrewLeaderID The crew leader
	 * @param InstanceID The meet spot instance
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Crew")
	void ReleaseCrewSpots(FGuid CrewLeaderID, FGuid InstanceID);

	/**
	 * @brief Get crew members in meet spot
	 * @param InstanceID The meet spot instance
	 * @param CrewID The crew to search for
	 * @return Array of crew members present
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Crew")
	TArray<FMGMeetSpotPlayer> GetCrewMembersInMeetSpot(FGuid InstanceID, FGuid CrewID) const;

	// ==========================================
	// SOCIAL INTERACTIONS (NEW)
	// ==========================================

	/**
	 * @brief Play an emote
	 * @param PlayerID The player performing the emote
	 * @param EmoteID The emote to play
	 * @return True if emote was played
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	bool PlayEmote(FGuid PlayerID, FName EmoteID);

	/**
	 * @brief Get available emotes for player
	 * @param ReputationTier Player's reputation tier
	 * @return Array of available emotes
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Social")
	TArray<FMGSocialEmote> GetAvailableEmotes(int32 ReputationTier) const;

	/**
	 * @brief Use car horn with a pattern
	 * @param PlayerID The player honking
	 * @param Pattern The horn pattern
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void UseHorn(FGuid PlayerID, EMGHornPattern Pattern);

	/**
	 * @brief Flash headlights (challenge signal)
	 * @param PlayerID The player flashing
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void FlashHeadlights(FGuid PlayerID);

	/**
	 * @brief Rev engine
	 * @param PlayerID The player revving
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void RevEngine(FGuid PlayerID);

	/**
	 * @brief Give respect to another player
	 * @param FromPlayerID The player giving respect
	 * @param ToPlayerID The player receiving respect
	 * @param RespectType Type of respect action
	 * @return True if respect was given
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	bool GiveRespect(FGuid FromPlayerID, FGuid ToPlayerID, FName RespectType);

	/**
	 * @brief Get player's respect received in session
	 * @param PlayerID The player to check
	 * @return Total respect received
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Social")
	int32 GetPlayerRespect(FGuid PlayerID) const;

	/**
	 * @brief Send proximity chat message
	 * @param SenderID The sending player
	 * @param Message The message text
	 * @param Range Proximity range in units
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Social")
	void SendProximityMessage(FGuid SenderID, const FString& Message, float Range = 2000.0f);

	// ==========================================
	// REPUTATION INTEGRATION (NEW)
	// ==========================================

	/**
	 * @brief Award presence reputation for being at meet spot
	 * Called periodically while player is in a meet spot
	 * @param PlayerID The player to award
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Reputation")
	void AwardPresenceReputation(FGuid PlayerID);

	/**
	 * @brief Award showcase reputation when player is showcased
	 * @param PlayerID The showcased player
	 * @param VoteCount Number of votes received
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Reputation")
	void AwardShowcaseReputation(FGuid PlayerID, int32 VoteCount);

	/**
	 * @brief Award social reputation for interactions
	 * @param PlayerID The player to award
	 * @param InteractionType Type of social interaction
	 */
	UFUNCTION(BlueprintCallable, Category = "MeetSpot|Reputation")
	void AwardSocialReputation(FGuid PlayerID, FName InteractionType);

	/**
	 * @brief Get instance vibe level
	 * Vibe level affects reputation gain rates
	 * @param InstanceID The meet spot instance
	 * @return Vibe level (0-100)
	 */
	UFUNCTION(BlueprintPure, Category = "MeetSpot|Reputation")
	int32 GetVibeLevel(FGuid InstanceID) const;

	// ==========================================
	// EVENTS (DELEGATES)
	// ==========================================

	/** Called when a player joins a meet spot */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnPlayerJoinedMeetSpot OnPlayerJoined;

	/** Called when a player leaves a meet spot */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnPlayerLeftMeetSpot OnPlayerLeft;

	/** Called when a meet spot event starts */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnMeetSpotEventStarted OnEventStarted;

	/** Called when a meet spot event ends */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnMeetSpotEventEnded OnEventEnded;

	/** Called when a player starts showcasing */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnShowcaseStarted OnShowcaseStarted;

	/** Called when a showcase receives a vote */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnShowcaseVote OnShowcaseVote;

	/** Called when a race challenge is created */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnRaceChallengeCreated OnRaceChallengeCreated;

	/** Called when a race challenge is accepted */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnRaceChallengeAccepted OnRaceChallengeAccepted;

	/** Called when a player plays an emote */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnEmotePlayed OnEmotePlayed;

	/** Called when a player uses their horn */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnHornPlayed OnHornPlayed;

	/** Called when respect is given */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnRespectGiven OnRespectGiven;

	/** Called when instance vibe level changes */
	UPROPERTY(BlueprintAssignable, Category = "MeetSpot|Events")
	FOnVibeChanged OnVibeChanged;

protected:
	/** Active meet spot instances */
	UPROPERTY()
	TMap<FGuid, FMGMeetSpotInstance> ActiveInstances;

	/** Player to instance mapping */
	UPROPERTY()
	TMap<FGuid, FGuid> PlayerInstanceMap;

	/** Photo spot queues (SpotIndex -> PlayerQueue) */
	UPROPERTY()
	TMap<int32, TArray<FGuid>> PhotoSpotQueues;

	/** Defined meet spot locations */
	UPROPERTY()
	TArray<FMGMeetSpotLocation> MeetSpotLocations;

	/** Available social emotes */
	UPROPERTY()
	TArray<FMGSocialEmote> AvailableEmotes;

	/** Respect tracking for cooldown */
	TMap<TPair<FGuid, FGuid>, FDateTime> RespectCooldowns;

	/** Timer for instance updates */
	FTimerHandle UpdateTimer;

	/** Timer for presence reputation awards */
	FTimerHandle PresenceReputationTimer;

	/** Showcase duration in seconds */
	static constexpr float ShowcaseDuration = 120.0f;

	/** Minimum time between respect actions to same player (seconds) */
	static constexpr float RespectCooldownSeconds = 300.0f;

	/** Presence reputation award interval (seconds) */
	static constexpr float PresenceReputationInterval = 60.0f;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Called every tick to update instances */
	void OnUpdateTick();

	/** Update showcase states */
	void UpdateShowcases();

	/** Update photo spot queues */
	void UpdatePhotoSpots();

	/** Update race challenges (expire old ones) */
	void UpdateChallenges();

	/** Update vibe levels based on activity */
	void UpdateVibeLevels();

	/** Clean up empty instances */
	void CleanupEmptyInstances();

	/** Award presence reputation to all players */
	void OnPresenceReputationTick();

	/** Create a new meet spot instance */
	FMGMeetSpotInstance CreateNewInstance(FName MeetSpotID);

	/** Set up default infrastructure for instance */
	void SetupDefaultInfrastructure(FMGMeetSpotInstance& Instance);

	/** Initialize meet spot locations */
	void InitializeMeetSpotLocations();

	/** Initialize available emotes */
	void InitializeEmotes();

	/** Advance showcase queue to next player */
	void AdvanceShowcaseQueue(FMGMeetSpotInstance& Instance);

	/** Check if player is crew member */
	bool IsCrewMember(FGuid PlayerID, FGuid CrewID) const;

	/** Find player in instance (mutable) */
	FMGMeetSpotPlayer* FindPlayer(FGuid InstanceID, FGuid PlayerID);

	/** Find player in instance (const) */
	const FMGMeetSpotPlayer* FindPlayerConst(FGuid InstanceID, FGuid PlayerID) const;

	/** Get location data for a meet spot ID */
	const FMGMeetSpotLocation* GetLocationData(FName MeetSpotID) const;

	/** Calculate vibe boost from player count and activity */
	int32 CalculateVibeLevel(const FMGMeetSpotInstance& Instance) const;
};

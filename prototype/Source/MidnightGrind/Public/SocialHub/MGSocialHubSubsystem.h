// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSocialHubSubsystem.h
 * @brief Social Hub System - Virtual spaces where players can meet, show off vehicles, and socialize
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ==========================
 * This file defines the Social Hub system for Midnight Grind. Think of Social Hubs as virtual
 * "car meets" or "hangout spots" where players can gather outside of races. It's similar to
 * the social spaces you might find in games like Forza Horizon's festival sites or GTA Online's
 * car meets.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * --------------------------
 *
 * 1. WHAT IS A GAMEINSTANCE SUBSYSTEM?
 *    - This class inherits from UGameInstanceSubsystem, which means it lives for the entire
 *      duration of the game session (not just one level/map).
 *    - Subsystems are Unreal Engine's way of creating modular, self-contained systems that
 *      automatically initialize and clean up with their parent object.
 *    - You can access this subsystem from anywhere using:
 *      UMGSocialHubSubsystem* HubSystem = GetGameInstance()->GetSubsystem<UMGSocialHubSubsystem>();
 *
 * 2. WHAT ARE STRUCTS (USTRUCT)?
 *    - Structs are data containers that group related information together.
 *    - For example, FMGHubPlayer contains all info about a player: their name, vehicle, location, etc.
 *    - The 'F' prefix is an Unreal convention for structs (F = "F"ile or "F"actory struct).
 *    - BlueprintType allows these to be used in Unreal's visual scripting system (Blueprints).
 *
 * 3. WHAT ARE ENUMS (UENUM)?
 *    - Enums define a list of named options/states.
 *    - EMGHubType lists all possible hub types (Garage, Meetup, Showroom, etc.).
 *    - Using enums instead of strings prevents typos and makes code more readable.
 *
 * 4. WHAT ARE DELEGATES?
 *    - Delegates are like "event broadcasters" - they notify other parts of the code when
 *      something happens.
 *    - For example, OnPlayerJoinedHub fires whenever a new player enters the hub, allowing
 *      the UI to update, sounds to play, etc.
 *    - "Dynamic Multicast" means multiple listeners can subscribe and it works with Blueprints.
 *
 * 5. WHAT ARE UPROPERTY AND UFUNCTION?
 *    - These macros expose variables and functions to Unreal's reflection system.
 *    - BlueprintReadWrite: Can be read and modified in Blueprints.
 *    - BlueprintCallable: Function can be called from Blueprints.
 *    - BlueprintPure: Function doesn't modify anything (like a "getter").
 *    - BlueprintAssignable: Delegate that can be bound in Blueprints.
 *
 * SYSTEM OVERVIEW:
 * ---------------
 * The Social Hub system provides:
 *
 * - HUB NAVIGATION: Join/leave different social spaces (meetups, showrooms, lounges)
 * - PERSONAL GARAGES: Each player has a customizable garage they can invite others to visit
 * - PLAYER INTERACTIONS: Wave, honk, rev engine, challenge to race, trade requests
 * - HUB EVENTS: Schedule and host car meets or community events
 * - CHAT SYSTEM: Public messages and private whispers
 * - PHOTO SPOTS: Designated locations for taking screenshots with vehicles
 *
 * DATA FLOW EXAMPLE:
 * -----------------
 * 1. Player calls JoinHub("hub_downtown_meetup")
 * 2. System validates the player meets requirements (level, VIP status, etc.)
 * 3. Player is added to the hub's player list
 * 4. OnHubJoined delegate fires, notifying UI to show the hub
 * 5. OnPlayerJoinedHub fires for all other players in the hub
 * 6. Player can now use interactions, chat, and see other players
 *
 * @see UGameInstanceSubsystem - Base class for game-wide subsystems
 * @see UMGRivalsSubsystem - Related system for tracking player rivalries
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSocialHubSubsystem.generated.h"

// Forward declarations
class UMGSocialHubSubsystem;

/**
 * EMGHubType - Types of social hubs
 */
UENUM(BlueprintType)
enum class EMGHubType : uint8
{
    Garage          UMETA(DisplayName = "Personal Garage"),
    Meetup          UMETA(DisplayName = "Meetup Spot"),
    Showroom        UMETA(DisplayName = "Showroom"),
    RacingLounge    UMETA(DisplayName = "Racing Lounge"),
    Workshop        UMETA(DisplayName = "Workshop"),
    VIPArea         UMETA(DisplayName = "VIP Area"),
    EventSpace      UMETA(DisplayName = "Event Space")
};

/**
 * EMGPlayerStatus - Player online status
 */
UENUM(BlueprintType)
enum class EMGPlayerStatus : uint8
{
    Online          UMETA(DisplayName = "Online"),
    InRace          UMETA(DisplayName = "In Race"),
    InHub           UMETA(DisplayName = "In Hub"),
    InGarage        UMETA(DisplayName = "In Garage"),
    Away            UMETA(DisplayName = "Away"),
    DoNotDisturb    UMETA(DisplayName = "Do Not Disturb"),
    Invisible       UMETA(DisplayName = "Invisible"),
    Offline         UMETA(DisplayName = "Offline")
};

/**
 * EMGInteractionType - Types of player interactions
 */
UENUM(BlueprintType)
enum class EMGInteractionType : uint8
{
    Wave            UMETA(DisplayName = "Wave"),
    Honk            UMETA(DisplayName = "Honk"),
    RevEngine       UMETA(DisplayName = "Rev Engine"),
    ShowVehicle     UMETA(DisplayName = "Show Vehicle"),
    TradeRequest    UMETA(DisplayName = "Trade Request"),
    RaceChallenge   UMETA(DisplayName = "Race Challenge"),
    Emote           UMETA(DisplayName = "Emote"),
    Photo           UMETA(DisplayName = "Take Photo")
};

/**
 * FMGHubPlayer - A player in a social hub
 */
USTRUCT(BlueprintType)
struct FMGHubPlayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGPlayerStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName CurrentVehicleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurrentVehicleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CrewName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> AvatarTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFriend;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVIP;

    FMGHubPlayer()
        : Status(EMGPlayerStatus::Online)
        , Level(1)
        , CurrentVehicleId(NAME_None)
        , Location(FVector::ZeroVector)
        , Rotation(FRotator::ZeroRotator)
        , bIsFriend(false)
        , bIsBlocked(false)
        , bIsVIP(false)
    {}
};

/**
 * FMGSocialHub - A social hub instance
 */
USTRUCT(BlueprintType)
struct FMGSocialHub
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HubId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText HubName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGHubType HubType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MapId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGHubPlayer> Players;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ParkingSpots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PhotoSpots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPrivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HostPlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Password;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVIPOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinLevelRequired;

    FMGSocialHub()
        : HubType(EMGHubType::Meetup)
        , MapId(NAME_None)
        , CurrentPlayers(0)
        , MaxPlayers(20)
        , bIsPrivate(false)
        , bIsVIPOnly(false)
        , MinLevelRequired(1)
    {}
};

/**
 * FMGHubInteraction - A player interaction in the hub
 */
USTRUCT(BlueprintType)
struct FMGHubInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InteractionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FromPlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ToPlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EmoteId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    FMGHubInteraction()
        : InteractionType(EMGInteractionType::Wave)
        , EmoteId(NAME_None)
    {}
};

/**
 * FMGHubEvent - A scheduled event in a hub
 */
USTRUCT(BlueprintType)
struct FMGHubEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HubId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HostPlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString HostName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RSVPCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxAttendees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RSVPPlayerIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOfficial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EventTheme;

    FMGHubEvent()
        : RSVPCount(0)
        , MaxAttendees(50)
        , bIsOfficial(false)
        , EventTheme(NAME_None)
    {}
};

/**
 * FMGPersonalGarage - Player's personal garage display
 */
USTRUCT(BlueprintType)
struct FMGPersonalGarage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OwnerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText GarageName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName GarageTheme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> DisplayedVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> DisplayedTrophies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> WallDecorations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GarageLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxVehicleSlots;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowVisitors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFriendsOnly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalVisits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageRating;

    FMGPersonalGarage()
        : GarageTheme(FName("Default"))
        , GarageLevel(1)
        , MaxVehicleSlots(5)
        , bAllowVisitors(true)
        , bFriendsOnly(false)
        , TotalVisits(0)
        , AverageRating(0.0f)
    {}
};

/**
 * FMGHubMessage - Chat message in hub
 */
USTRUCT(BlueprintType)
struct FMGHubMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MessageId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SenderId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SenderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWhisper;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WhisperTargetId;

    FMGHubMessage()
        : bIsSystem(false)
        , bIsWhisper(false)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHubJoined, const FMGSocialHub&, Hub);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnHubLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerJoinedHub, const FMGHubPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerLeftHub, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnInteractionReceived, const FMGHubInteraction&, Interaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHubMessageReceived, const FMGHubMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnGarageVisitor, const FMGHubPlayer&, Visitor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnHubEventStarting, const FMGHubEvent&, Event);

/**
 * UMGSocialHubSubsystem
 *
 * Manages social hub spaces for Midnight Grind.
 * Features include:
 * - Public and private hub spaces
 * - Personal garage displays
 * - Player interactions and emotes
 * - Hub events and meetups
 * - Chat and communication
 * - Vehicle showcases
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSocialHubSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGSocialHubSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Hub Navigation =====

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Navigation")
    bool JoinHub(const FString& HubId, const FString& Password = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Navigation")
    bool JoinRandomHub(EMGHubType HubType);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Navigation")
    void LeaveHub();

    UFUNCTION(BlueprintPure, Category = "SocialHub|Navigation")
    bool IsInHub() const;

    UFUNCTION(BlueprintPure, Category = "SocialHub|Navigation")
    FMGSocialHub GetCurrentHub() const;

    UFUNCTION(BlueprintPure, Category = "SocialHub|Navigation")
    TArray<FMGSocialHub> GetAvailableHubs(EMGHubType HubType) const;

    // ===== Hub Creation =====

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Create")
    FString CreateHub(EMGHubType HubType, const FText& HubName, bool bIsPrivate, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Create")
    bool CloseHub(const FString& HubId);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Create")
    bool SetHubSettings(const FString& HubId, bool bIsPrivate, const FString& Password, int32 MaxPlayers);

    // ===== Players =====

    UFUNCTION(BlueprintPure, Category = "SocialHub|Players")
    TArray<FMGHubPlayer> GetPlayersInHub() const;

    UFUNCTION(BlueprintPure, Category = "SocialHub|Players")
    FMGHubPlayer GetPlayer(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Players")
    void UpdatePlayerLocation(FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Players")
    void UpdatePlayerStatus(EMGPlayerStatus NewStatus);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Players")
    void UpdateDisplayedVehicle(FName VehicleId);

    // ===== Interactions =====

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Interact")
    bool SendInteraction(const FString& TargetPlayerId, EMGInteractionType InteractionType);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Interact")
    bool SendEmote(FName EmoteId);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Interact")
    bool ChallengeToRace(const FString& TargetPlayerId, FName TrackId);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Interact")
    bool RequestTrade(const FString& TargetPlayerId);

    // ===== Garage =====

    UFUNCTION(BlueprintPure, Category = "SocialHub|Garage")
    FMGPersonalGarage GetMyGarage() const;

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Garage")
    void SetGarageSettings(const FMGPersonalGarage& Settings);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Garage")
    bool AddVehicleToDisplay(FName VehicleId, int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Garage")
    bool RemoveVehicleFromDisplay(int32 SlotIndex);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Garage")
    bool VisitGarage(const FString& OwnerId);

    UFUNCTION(BlueprintPure, Category = "SocialHub|Garage")
    FMGPersonalGarage GetGarage(const FString& OwnerId) const;

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Garage")
    void RateGarage(const FString& OwnerId, int32 Rating);

    // ===== Events =====

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Events")
    FString CreateHubEvent(const FMGHubEvent& EventInfo);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Events")
    bool CancelHubEvent(const FString& EventId);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Events")
    bool RSVPToEvent(const FString& EventId);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Events")
    bool CancelRSVP(const FString& EventId);

    UFUNCTION(BlueprintPure, Category = "SocialHub|Events")
    TArray<FMGHubEvent> GetUpcomingEvents() const;

    UFUNCTION(BlueprintPure, Category = "SocialHub|Events")
    TArray<FMGHubEvent> GetMyEvents() const;

    // ===== Chat =====

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Chat")
    void SendHubMessage(const FString& Message);

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Chat")
    void SendWhisper(const FString& TargetPlayerId, const FString& Message);

    UFUNCTION(BlueprintPure, Category = "SocialHub|Chat")
    TArray<FMGHubMessage> GetRecentMessages(int32 Count) const;

    // ===== Photo Spots =====

    UFUNCTION(BlueprintPure, Category = "SocialHub|Photo")
    TArray<FVector> GetPhotoSpots() const;

    UFUNCTION(BlueprintCallable, Category = "SocialHub|Photo")
    bool TakeGroupPhoto(const TArray<FString>& PlayerIds);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnHubJoined OnHubJoined;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnHubLeft OnHubLeft;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnPlayerJoinedHub OnPlayerJoinedHub;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnPlayerLeftHub OnPlayerLeftHub;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnInteractionReceived OnInteractionReceived;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnHubMessageReceived OnHubMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnGarageVisitor OnGarageVisitor;

    UPROPERTY(BlueprintAssignable, Category = "SocialHub|Events")
    FMGOnHubEventStarting OnHubEventStarting;

protected:
    void InitializeSampleHubs();
    void InitializePlayerGarage();
    void CheckUpcomingEvents();

private:
    UPROPERTY()
    TMap<FString, FMGSocialHub> AllHubs;

    UPROPERTY()
    FMGSocialHub CurrentHub;

    UPROPERTY()
    bool bIsInHub;

    UPROPERTY()
    FMGPersonalGarage MyGarage;

    UPROPERTY()
    TMap<FString, FMGPersonalGarage> VisitedGarages;

    UPROPERTY()
    TArray<FMGHubEvent> HubEvents;

    UPROPERTY()
    TArray<FMGHubMessage> ChatHistory;

    UPROPERTY()
    FString LocalPlayerId;

    UPROPERTY()
    FMGHubPlayer LocalPlayer;

    FTimerHandle EventCheckTimerHandle;
};

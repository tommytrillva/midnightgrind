// MidnightGrind - Arcade Street Racing Game
// Social Hub Subsystem - Implementation

#include "SocialHub/MGSocialHubSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGSocialHubSubsystem::UMGSocialHubSubsystem()
    : bIsInHub(false)
    , LocalPlayerId(TEXT("LocalPlayer"))
{
}

void UMGSocialHubSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    InitializeSampleHubs();
    InitializePlayerGarage();

    // Set up local player
    LocalPlayer.PlayerId = LocalPlayerId;
    LocalPlayer.DisplayName = TEXT("LocalPlayer");
    LocalPlayer.Status = EMGPlayerStatus::Online;
    LocalPlayer.Level = 25;
    LocalPlayer.Title = TEXT("Street Legend");

    // Check for upcoming events periodically
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            EventCheckTimerHandle,
            [this]()
            {
                CheckUpcomingEvents();
            },
            60.0f,
            true
        );
    }
}

void UMGSocialHubSubsystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(EventCheckTimerHandle);
    }

    Super::Deinitialize();
}

// ===== Hub Navigation =====

bool UMGSocialHubSubsystem::JoinHub(const FString& HubId, const FString& Password)
{
    FMGSocialHub* Hub = AllHubs.Find(HubId);
    if (!Hub)
    {
        return false;
    }

    // Check password
    if (Hub->bIsPrivate && Hub->Password != Password)
    {
        return false;
    }

    // Check VIP
    if (Hub->bIsVIPOnly && !LocalPlayer.bIsVIP)
    {
        return false;
    }

    // Check level
    if (LocalPlayer.Level < Hub->MinLevelRequired)
    {
        return false;
    }

    // Check capacity
    if (Hub->CurrentPlayers >= Hub->MaxPlayers)
    {
        return false;
    }

    // Leave current hub if in one
    if (bIsInHub)
    {
        LeaveHub();
    }

    // Join hub
    Hub->Players.Add(LocalPlayer);
    Hub->CurrentPlayers++;
    CurrentHub = *Hub;
    bIsInHub = true;

    OnHubJoined.Broadcast(CurrentHub);

    // Notify other players
    for (const FMGHubPlayer& Player : Hub->Players)
    {
        if (Player.PlayerId != LocalPlayerId)
        {
            // In production, would send network message
        }
    }

    return true;
}

bool UMGSocialHubSubsystem::JoinRandomHub(EMGHubType HubType)
{
    for (auto& Pair : AllHubs)
    {
        if (Pair.Value.HubType == HubType && !Pair.Value.bIsPrivate && Pair.Value.CurrentPlayers < Pair.Value.MaxPlayers)
        {
            return JoinHub(Pair.Key, TEXT(""));
        }
    }
    return false;
}

void UMGSocialHubSubsystem::LeaveHub()
{
    if (!bIsInHub)
    {
        return;
    }

    // Remove from hub
    if (FMGSocialHub* Hub = AllHubs.Find(CurrentHub.HubId))
    {
        Hub->Players.RemoveAll([this](const FMGHubPlayer& Player)
        {
            return Player.PlayerId == LocalPlayerId;
        });
        Hub->CurrentPlayers = FMath::Max(0, Hub->CurrentPlayers - 1);
    }

    CurrentHub = FMGSocialHub();
    bIsInHub = false;

    OnHubLeft.Broadcast();
}

bool UMGSocialHubSubsystem::IsInHub() const
{
    return bIsInHub;
}

FMGSocialHub UMGSocialHubSubsystem::GetCurrentHub() const
{
    return CurrentHub;
}

TArray<FMGSocialHub> UMGSocialHubSubsystem::GetAvailableHubs(EMGHubType HubType) const
{
    TArray<FMGSocialHub> Available;

    for (const auto& Pair : AllHubs)
    {
        if (Pair.Value.HubType == HubType && !Pair.Value.bIsPrivate)
        {
            Available.Add(Pair.Value);
        }
    }

    // Sort by player count
    Available.Sort([](const FMGSocialHub& A, const FMGSocialHub& B)
    {
        return A.CurrentPlayers > B.CurrentPlayers;
    });

    return Available;
}

// ===== Hub Creation =====

FString UMGSocialHubSubsystem::CreateHub(EMGHubType HubType, const FText& HubName, bool bIsPrivate, const FString& Password)
{
    FMGSocialHub NewHub;
    NewHub.HubId = FGuid::NewGuid().ToString();
    NewHub.HubName = HubName;
    NewHub.HubType = HubType;
    NewHub.bIsPrivate = bIsPrivate;
    NewHub.Password = Password;
    NewHub.HostPlayerId = LocalPlayerId;
    NewHub.MaxPlayers = 20;

    // Set map based on type
    switch (HubType)
    {
    case EMGHubType::Garage:
        NewHub.MapId = FName("Hub_Garage");
        NewHub.MaxPlayers = 10;
        break;
    case EMGHubType::Meetup:
        NewHub.MapId = FName("Hub_Parking_Lot");
        break;
    case EMGHubType::Showroom:
        NewHub.MapId = FName("Hub_Showroom");
        NewHub.MaxPlayers = 30;
        break;
    case EMGHubType::RacingLounge:
        NewHub.MapId = FName("Hub_Racing_Lounge");
        break;
    default:
        NewHub.MapId = FName("Hub_Default");
        break;
    }

    AllHubs.Add(NewHub.HubId, NewHub);
    return NewHub.HubId;
}

bool UMGSocialHubSubsystem::CloseHub(const FString& HubId)
{
    FMGSocialHub* Hub = AllHubs.Find(HubId);
    if (!Hub || Hub->HostPlayerId != LocalPlayerId)
    {
        return false;
    }

    // Kick all players
    for (const FMGHubPlayer& Player : Hub->Players)
    {
        // In production, would notify players
    }

    AllHubs.Remove(HubId);

    if (CurrentHub.HubId == HubId)
    {
        bIsInHub = false;
        CurrentHub = FMGSocialHub();
        OnHubLeft.Broadcast();
    }

    return true;
}

bool UMGSocialHubSubsystem::SetHubSettings(const FString& HubId, bool bIsPrivate, const FString& Password, int32 MaxPlayers)
{
    FMGSocialHub* Hub = AllHubs.Find(HubId);
    if (!Hub || Hub->HostPlayerId != LocalPlayerId)
    {
        return false;
    }

    Hub->bIsPrivate = bIsPrivate;
    Hub->Password = Password;
    Hub->MaxPlayers = FMath::Max(Hub->CurrentPlayers, MaxPlayers);

    return true;
}

// ===== Players =====

TArray<FMGHubPlayer> UMGSocialHubSubsystem::GetPlayersInHub() const
{
    return CurrentHub.Players;
}

FMGHubPlayer UMGSocialHubSubsystem::GetPlayer(const FString& PlayerId) const
{
    for (const FMGHubPlayer& Player : CurrentHub.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            return Player;
        }
    }
    return FMGHubPlayer();
}

void UMGSocialHubSubsystem::UpdatePlayerLocation(FVector Location, FRotator Rotation)
{
    LocalPlayer.Location = Location;
    LocalPlayer.Rotation = Rotation;

    // Update in current hub
    if (bIsInHub)
    {
        for (FMGHubPlayer& Player : CurrentHub.Players)
        {
            if (Player.PlayerId == LocalPlayerId)
            {
                Player.Location = Location;
                Player.Rotation = Rotation;
                break;
            }
        }
    }
}

void UMGSocialHubSubsystem::UpdatePlayerStatus(EMGPlayerStatus NewStatus)
{
    LocalPlayer.Status = NewStatus;

    if (bIsInHub)
    {
        for (FMGHubPlayer& Player : CurrentHub.Players)
        {
            if (Player.PlayerId == LocalPlayerId)
            {
                Player.Status = NewStatus;
                break;
            }
        }
    }
}

void UMGSocialHubSubsystem::UpdateDisplayedVehicle(FName VehicleId)
{
    LocalPlayer.CurrentVehicleId = VehicleId;

    if (bIsInHub)
    {
        for (FMGHubPlayer& Player : CurrentHub.Players)
        {
            if (Player.PlayerId == LocalPlayerId)
            {
                Player.CurrentVehicleId = VehicleId;
                break;
            }
        }
    }
}

// ===== Interactions =====

bool UMGSocialHubSubsystem::SendInteraction(const FString& TargetPlayerId, EMGInteractionType InteractionType)
{
    if (!bIsInHub)
    {
        return false;
    }

    FMGHubInteraction Interaction;
    Interaction.InteractionId = FGuid::NewGuid().ToString();
    Interaction.FromPlayerId = LocalPlayerId;
    Interaction.ToPlayerId = TargetPlayerId;
    Interaction.InteractionType = InteractionType;
    Interaction.Timestamp = FDateTime::Now();

    // In production, would send to server
    OnInteractionReceived.Broadcast(Interaction);
    return true;
}

bool UMGSocialHubSubsystem::SendEmote(FName EmoteId)
{
    if (!bIsInHub)
    {
        return false;
    }

    FMGHubInteraction Interaction;
    Interaction.InteractionId = FGuid::NewGuid().ToString();
    Interaction.FromPlayerId = LocalPlayerId;
    Interaction.InteractionType = EMGInteractionType::Emote;
    Interaction.EmoteId = EmoteId;
    Interaction.Timestamp = FDateTime::Now();

    OnInteractionReceived.Broadcast(Interaction);
    return true;
}

bool UMGSocialHubSubsystem::ChallengeToRace(const FString& TargetPlayerId, FName TrackId)
{
    if (!bIsInHub)
    {
        return false;
    }

    FMGHubInteraction Interaction;
    Interaction.InteractionId = FGuid::NewGuid().ToString();
    Interaction.FromPlayerId = LocalPlayerId;
    Interaction.ToPlayerId = TargetPlayerId;
    Interaction.InteractionType = EMGInteractionType::RaceChallenge;
    Interaction.Timestamp = FDateTime::Now();

    OnInteractionReceived.Broadcast(Interaction);
    return true;
}

bool UMGSocialHubSubsystem::RequestTrade(const FString& TargetPlayerId)
{
    if (!bIsInHub)
    {
        return false;
    }

    FMGHubInteraction Interaction;
    Interaction.InteractionId = FGuid::NewGuid().ToString();
    Interaction.FromPlayerId = LocalPlayerId;
    Interaction.ToPlayerId = TargetPlayerId;
    Interaction.InteractionType = EMGInteractionType::TradeRequest;
    Interaction.Timestamp = FDateTime::Now();

    OnInteractionReceived.Broadcast(Interaction);
    return true;
}

// ===== Garage =====

FMGPersonalGarage UMGSocialHubSubsystem::GetMyGarage() const
{
    return MyGarage;
}

void UMGSocialHubSubsystem::SetGarageSettings(const FMGPersonalGarage& Settings)
{
    MyGarage.GarageName = Settings.GarageName;
    MyGarage.GarageTheme = Settings.GarageTheme;
    MyGarage.bAllowVisitors = Settings.bAllowVisitors;
    MyGarage.bFriendsOnly = Settings.bFriendsOnly;
    MyGarage.WallDecorations = Settings.WallDecorations;
}

bool UMGSocialHubSubsystem::AddVehicleToDisplay(FName VehicleId, int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= MyGarage.MaxVehicleSlots)
    {
        return false;
    }

    // Expand array if needed
    while (MyGarage.DisplayedVehicles.Num() <= SlotIndex)
    {
        MyGarage.DisplayedVehicles.Add(NAME_None);
    }

    MyGarage.DisplayedVehicles[SlotIndex] = VehicleId;
    return true;
}

bool UMGSocialHubSubsystem::RemoveVehicleFromDisplay(int32 SlotIndex)
{
    if (SlotIndex < 0 || SlotIndex >= MyGarage.DisplayedVehicles.Num())
    {
        return false;
    }

    MyGarage.DisplayedVehicles[SlotIndex] = NAME_None;
    return true;
}

bool UMGSocialHubSubsystem::VisitGarage(const FString& OwnerId)
{
    // In production, would fetch garage data from server
    FMGPersonalGarage VisitedGarage;
    VisitedGarage.OwnerId = OwnerId;
    VisitedGarage.GarageName = FText::FromString(FString::Printf(TEXT("%s's Garage"), *OwnerId));
    VisitedGarage.TotalVisits++;

    VisitedGarages.Add(OwnerId, VisitedGarage);

    // Notify garage owner
    FMGHubPlayer Visitor = LocalPlayer;
    OnGarageVisitor.Broadcast(Visitor);

    return true;
}

FMGPersonalGarage UMGSocialHubSubsystem::GetGarage(const FString& OwnerId) const
{
    if (OwnerId == LocalPlayerId)
    {
        return MyGarage;
    }

    if (const FMGPersonalGarage* Garage = VisitedGarages.Find(OwnerId))
    {
        return *Garage;
    }

    return FMGPersonalGarage();
}

void UMGSocialHubSubsystem::RateGarage(const FString& OwnerId, int32 Rating)
{
    // In production, would send rating to server
}

// ===== Events =====

FString UMGSocialHubSubsystem::CreateHubEvent(const FMGHubEvent& EventInfo)
{
    FMGHubEvent NewEvent = EventInfo;
    NewEvent.EventId = FGuid::NewGuid().ToString();
    NewEvent.HostPlayerId = LocalPlayerId;
    NewEvent.HostName = LocalPlayer.DisplayName;
    NewEvent.RSVPCount = 1;
    NewEvent.RSVPPlayerIds.Add(LocalPlayerId);

    HubEvents.Add(NewEvent);
    return NewEvent.EventId;
}

bool UMGSocialHubSubsystem::CancelHubEvent(const FString& EventId)
{
    for (int32 i = 0; i < HubEvents.Num(); i++)
    {
        if (HubEvents[i].EventId == EventId && HubEvents[i].HostPlayerId == LocalPlayerId)
        {
            HubEvents.RemoveAt(i);
            return true;
        }
    }
    return false;
}

bool UMGSocialHubSubsystem::RSVPToEvent(const FString& EventId)
{
    for (FMGHubEvent& Event : HubEvents)
    {
        if (Event.EventId == EventId)
        {
            if (Event.RSVPCount >= Event.MaxAttendees)
            {
                return false;
            }

            if (Event.RSVPPlayerIds.Contains(LocalPlayerId))
            {
                return false;
            }

            Event.RSVPPlayerIds.Add(LocalPlayerId);
            Event.RSVPCount++;
            return true;
        }
    }
    return false;
}

bool UMGSocialHubSubsystem::CancelRSVP(const FString& EventId)
{
    for (FMGHubEvent& Event : HubEvents)
    {
        if (Event.EventId == EventId)
        {
            if (Event.RSVPPlayerIds.Remove(LocalPlayerId) > 0)
            {
                Event.RSVPCount = FMath::Max(0, Event.RSVPCount - 1);
                return true;
            }
        }
    }
    return false;
}

TArray<FMGHubEvent> UMGSocialHubSubsystem::GetUpcomingEvents() const
{
    TArray<FMGHubEvent> Upcoming;
    FDateTime Now = FDateTime::Now();

    for (const FMGHubEvent& Event : HubEvents)
    {
        if (Event.StartTime > Now)
        {
            Upcoming.Add(Event);
        }
    }

    Upcoming.Sort([](const FMGHubEvent& A, const FMGHubEvent& B)
    {
        return A.StartTime < B.StartTime;
    });

    return Upcoming;
}

TArray<FMGHubEvent> UMGSocialHubSubsystem::GetMyEvents() const
{
    TArray<FMGHubEvent> MyEvents;

    for (const FMGHubEvent& Event : HubEvents)
    {
        if (Event.HostPlayerId == LocalPlayerId || Event.RSVPPlayerIds.Contains(LocalPlayerId))
        {
            MyEvents.Add(Event);
        }
    }

    return MyEvents;
}

// ===== Chat =====

void UMGSocialHubSubsystem::SendHubMessage(const FString& Message)
{
    if (!bIsInHub)
    {
        return;
    }

    FMGHubMessage NewMessage;
    NewMessage.MessageId = FGuid::NewGuid().ToString();
    NewMessage.SenderId = LocalPlayerId;
    NewMessage.SenderName = LocalPlayer.DisplayName;
    NewMessage.Message = Message;
    NewMessage.Timestamp = FDateTime::Now();

    ChatHistory.Add(NewMessage);
    OnHubMessageReceived.Broadcast(NewMessage);
}

void UMGSocialHubSubsystem::SendWhisper(const FString& TargetPlayerId, const FString& Message)
{
    FMGHubMessage NewMessage;
    NewMessage.MessageId = FGuid::NewGuid().ToString();
    NewMessage.SenderId = LocalPlayerId;
    NewMessage.SenderName = LocalPlayer.DisplayName;
    NewMessage.Message = Message;
    NewMessage.Timestamp = FDateTime::Now();
    NewMessage.bIsWhisper = true;
    NewMessage.WhisperTargetId = TargetPlayerId;

    ChatHistory.Add(NewMessage);
    OnHubMessageReceived.Broadcast(NewMessage);
}

TArray<FMGHubMessage> UMGSocialHubSubsystem::GetRecentMessages(int32 Count) const
{
    int32 StartIndex = FMath::Max(0, ChatHistory.Num() - Count);
    TArray<FMGHubMessage> Recent;

    for (int32 i = StartIndex; i < ChatHistory.Num(); i++)
    {
        Recent.Add(ChatHistory[i]);
    }

    return Recent;
}

// ===== Photo Spots =====

TArray<FVector> UMGSocialHubSubsystem::GetPhotoSpots() const
{
    return CurrentHub.PhotoSpots;
}

bool UMGSocialHubSubsystem::TakeGroupPhoto(const TArray<FString>& PlayerIds)
{
    if (!bIsInHub)
    {
        return false;
    }

    // In production, would trigger photo capture
    return true;
}

// ===== Protected =====

void UMGSocialHubSubsystem::InitializeSampleHubs()
{
    // Downtown Meetup
    {
        FMGSocialHub Meetup;
        Meetup.HubId = TEXT("hub_downtown_meetup");
        Meetup.HubName = FText::FromString(TEXT("Downtown Parking Meetup"));
        Meetup.HubType = EMGHubType::Meetup;
        Meetup.MapId = FName("Hub_Downtown_Parking");
        Meetup.MaxPlayers = 30;
        Meetup.CurrentPlayers = 12;

        Meetup.ParkingSpots = {
            FVector(0, 0, 0), FVector(500, 0, 0), FVector(1000, 0, 0),
            FVector(0, 500, 0), FVector(500, 500, 0), FVector(1000, 500, 0)
        };
        Meetup.PhotoSpots = {
            FVector(250, 250, 100), FVector(750, 250, 100)
        };

        // Add sample players
        for (int32 i = 0; i < 5; i++)
        {
            FMGHubPlayer Player;
            Player.PlayerId = FString::Printf(TEXT("player_%d"), i);
            Player.DisplayName = FString::Printf(TEXT("Racer_%d"), FMath::RandRange(1000, 9999));
            Player.Level = FMath::RandRange(5, 50);
            Player.Status = EMGPlayerStatus::InHub;
            Meetup.Players.Add(Player);
        }

        AllHubs.Add(Meetup.HubId, Meetup);
    }

    // Racing Lounge
    {
        FMGSocialHub Lounge;
        Lounge.HubId = TEXT("hub_racing_lounge");
        Lounge.HubName = FText::FromString(TEXT("Midnight Racing Lounge"));
        Lounge.HubType = EMGHubType::RacingLounge;
        Lounge.MapId = FName("Hub_Racing_Lounge");
        Lounge.MaxPlayers = 50;
        Lounge.CurrentPlayers = 23;

        AllHubs.Add(Lounge.HubId, Lounge);
    }

    // VIP Showroom
    {
        FMGSocialHub VIP;
        VIP.HubId = TEXT("hub_vip_showroom");
        VIP.HubName = FText::FromString(TEXT("VIP Showroom"));
        VIP.HubType = EMGHubType::Showroom;
        VIP.MapId = FName("Hub_VIP_Showroom");
        VIP.MaxPlayers = 20;
        VIP.CurrentPlayers = 8;
        VIP.bIsVIPOnly = true;
        VIP.MinLevelRequired = 30;

        AllHubs.Add(VIP.HubId, VIP);
    }

    // Sample events
    FDateTime Now = FDateTime::Now();

    FMGHubEvent CarMeet;
    CarMeet.EventId = TEXT("event_weekly_meet");
    CarMeet.EventName = FText::FromString(TEXT("Weekly JDM Meet"));
    CarMeet.Description = FText::FromString(TEXT("Show off your best JDM builds! Prizes for best in show."));
    CarMeet.HubId = TEXT("hub_downtown_meetup");
    CarMeet.StartTime = Now + FTimespan::FromHours(24);
    CarMeet.EndTime = Now + FTimespan::FromHours(26);
    CarMeet.HostPlayerId = TEXT("official");
    CarMeet.HostName = TEXT("Midnight Grind");
    CarMeet.bIsOfficial = true;
    CarMeet.MaxAttendees = 100;
    CarMeet.RSVPCount = 45;
    CarMeet.EventTheme = FName("JDM");
    HubEvents.Add(CarMeet);
}

void UMGSocialHubSubsystem::InitializePlayerGarage()
{
    MyGarage.OwnerId = LocalPlayerId;
    MyGarage.GarageName = FText::FromString(TEXT("My Garage"));
    MyGarage.GarageTheme = FName("Industrial");
    MyGarage.GarageLevel = 3;
    MyGarage.MaxVehicleSlots = 8;
    MyGarage.bAllowVisitors = true;
    MyGarage.bFriendsOnly = false;
}

void UMGSocialHubSubsystem::CheckUpcomingEvents()
{
    FDateTime Now = FDateTime::Now();

    for (const FMGHubEvent& Event : HubEvents)
    {
        // Notify 15 minutes before event starts
        FTimespan TimeUntilStart = Event.StartTime - Now;
        if (TimeUntilStart.GetTotalMinutes() > 14 && TimeUntilStart.GetTotalMinutes() <= 15)
        {
            if (Event.RSVPPlayerIds.Contains(LocalPlayerId))
            {
                OnHubEventStarting.Broadcast(Event);
            }
        }
    }
}

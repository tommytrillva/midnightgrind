// Copyright Midnight Grind. All Rights Reserved.

// MidnightGrind - Arcade Street Racing Game
// Platform Integration Subsystem Implementation

#include "PlatformIntegration/MGPlatformIntegrationSubsystem.h"
#include "Engine/World.h"

UMGPlatformIntegrationSubsystem::UMGPlatformIntegrationSubsystem()
    : CurrentPlatform(EMGPlatformType::Unknown)
    , bUserLoggedIn(false)
    , bOverlayActive(false)
    , bVideoCapturing(false)
    , bPlatformInitialized(false)
{
}

void UMGPlatformIntegrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Detect current platform
    DetectPlatform();

    // Initialize platform services
    InitializePlatformServices();

    // Initialize default achievements
    InitializeDefaultAchievements();

    bPlatformInitialized = true;
    OnPlatformInitialized.Broadcast(CurrentPlatform);

    UE_LOG(LogTemp, Log, TEXT("MGPlatformIntegrationSubsystem initialized for platform: %s"),
        *GetPlatformName());
}

void UMGPlatformIntegrationSubsystem::Deinitialize()
{
    // Clear presence on shutdown
    ClearRichPresence();

    Super::Deinitialize();
}

// ===== Platform Info =====

EMGPlatformType UMGPlatformIntegrationSubsystem::GetCurrentPlatform() const
{
    return CurrentPlatform;
}

FMGPlatformCapabilities UMGPlatformIntegrationSubsystem::GetPlatformCapabilities() const
{
    return Capabilities;
}

bool UMGPlatformIntegrationSubsystem::HasCapability(EMGOnlineCapability Capability) const
{
    switch (Capability)
    {
        case EMGOnlineCapability::Multiplayer: return true;
        case EMGOnlineCapability::Leaderboards: return Capabilities.bSupportsLeaderboards;
        case EMGOnlineCapability::CloudSave: return Capabilities.bSupportsCloudSave;
        case EMGOnlineCapability::Achievements: return Capabilities.bSupportsAchievements;
        case EMGOnlineCapability::Friends: return Capabilities.bSupportsFriends;
        case EMGOnlineCapability::VoiceChat: return Capabilities.bSupportsVoiceChat;
        case EMGOnlineCapability::Streaming: return Capabilities.bSupportsStreaming;
        case EMGOnlineCapability::UGC: return Capabilities.bSupportsUGC;
        default: return false;
    }
}

FString UMGPlatformIntegrationSubsystem::GetPlatformName() const
{
    switch (CurrentPlatform)
    {
        case EMGPlatformType::PC_Steam: return TEXT("Steam");
        case EMGPlatformType::PC_Epic: return TEXT("Epic Games Store");
        case EMGPlatformType::PC_GOG: return TEXT("GOG");
        case EMGPlatformType::PC_Windows: return TEXT("Windows Store");
        case EMGPlatformType::PlayStation4: return TEXT("PlayStation 4");
        case EMGPlatformType::PlayStation5: return TEXT("PlayStation 5");
        case EMGPlatformType::XboxOne: return TEXT("Xbox One");
        case EMGPlatformType::XboxSeriesX: return TEXT("Xbox Series X|S");
        case EMGPlatformType::NintendoSwitch: return TEXT("Nintendo Switch");
        case EMGPlatformType::Mobile_iOS: return TEXT("iOS");
        case EMGPlatformType::Mobile_Android: return TEXT("Android");
        default: return TEXT("Unknown");
    }
}

bool UMGPlatformIntegrationSubsystem::IsPlatformOnline() const
{
    // In a real implementation, check actual platform online status
    return bUserLoggedIn;
}

// ===== User Management =====

FMGPlatformUser UMGPlatformIntegrationSubsystem::GetCurrentUser() const
{
    return CurrentUser;
}

bool UMGPlatformIntegrationSubsystem::IsUserLoggedIn() const
{
    return bUserLoggedIn;
}

void UMGPlatformIntegrationSubsystem::RequestLogin()
{
    // In a real implementation, trigger platform login flow
    // Simulating successful login
    CurrentUser.PlatformUserId = TEXT("12345678");
    CurrentUser.DisplayName = TEXT("MidnightRacer");
    CurrentUser.Platform = CurrentPlatform;
    CurrentUser.bIsOnline = true;
    CurrentUser.PresenceState = EMGPresenceState::Online;

    bUserLoggedIn = true;
    OnUserLoggedIn.Broadcast(CurrentUser);

    UE_LOG(LogTemp, Log, TEXT("User logged in: %s"), *CurrentUser.DisplayName);
}

void UMGPlatformIntegrationSubsystem::RequestLogout()
{
    if (!bUserLoggedIn) return;

    ClearRichPresence();
    CurrentUser = FMGPlatformUser();
    bUserLoggedIn = false;

    OnUserLoggedOut.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("User logged out"));
}

FString UMGPlatformIntegrationSubsystem::GetUserDisplayName() const
{
    return CurrentUser.DisplayName;
}

FString UMGPlatformIntegrationSubsystem::GetUserId() const
{
    return CurrentUser.PlatformUserId;
}

// ===== Achievements =====

void UMGPlatformIntegrationSubsystem::UnlockAchievement(FName AchievementId)
{
    FMGPlatformAchievement* Achievement = Achievements.Find(AchievementId);
    if (!Achievement) return;

    if (Achievement->Status == EMGAchievementStatus::Unlocked) return;

    Achievement->Status = EMGAchievementStatus::Unlocked;
    Achievement->Progress = Achievement->MaxProgress;
    Achievement->UnlockTime = FDateTime::Now();

    // In a real implementation, sync with platform
    SyncAchievementsWithPlatform();

    OnAchievementUnlocked.Broadcast(*Achievement);
    UE_LOG(LogTemp, Log, TEXT("Achievement unlocked: %s"), *Achievement->DisplayName.ToString());
}

void UMGPlatformIntegrationSubsystem::UpdateAchievementProgress(FName AchievementId, float Progress)
{
    FMGPlatformAchievement* Achievement = Achievements.Find(AchievementId);
    if (!Achievement) return;

    if (Achievement->Status == EMGAchievementStatus::Unlocked) return;

    Achievement->Progress = FMath::Clamp(Progress, 0.0f, Achievement->MaxProgress);
    Achievement->Status = Achievement->Progress > 0.0f ? EMGAchievementStatus::InProgress : EMGAchievementStatus::Locked;

    OnAchievementProgressUpdated.Broadcast(AchievementId, Achievement->Progress);

    if (Achievement->IsComplete())
    {
        UnlockAchievement(AchievementId);
    }
}

void UMGPlatformIntegrationSubsystem::IncrementAchievementProgress(FName AchievementId, float Amount)
{
    FMGPlatformAchievement* Achievement = Achievements.Find(AchievementId);
    if (!Achievement) return;

    UpdateAchievementProgress(AchievementId, Achievement->Progress + Amount);
}

FMGPlatformAchievement UMGPlatformIntegrationSubsystem::GetAchievement(FName AchievementId) const
{
    const FMGPlatformAchievement* Achievement = Achievements.Find(AchievementId);
    return Achievement ? *Achievement : FMGPlatformAchievement();
}

TArray<FMGPlatformAchievement> UMGPlatformIntegrationSubsystem::GetAllAchievements() const
{
    TArray<FMGPlatformAchievement> Result;
    Achievements.GenerateValueArray(Result);
    return Result;
}

TArray<FMGPlatformAchievement> UMGPlatformIntegrationSubsystem::GetUnlockedAchievements() const
{
    TArray<FMGPlatformAchievement> Result;
    for (const auto& Pair : Achievements)
    {
        if (Pair.Value.Status == EMGAchievementStatus::Unlocked)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

float UMGPlatformIntegrationSubsystem::GetAchievementCompletionPercent() const
{
    if (Achievements.Num() == 0) return 0.0f;

    int32 UnlockedCount = 0;
    for (const auto& Pair : Achievements)
    {
        if (Pair.Value.Status == EMGAchievementStatus::Unlocked)
        {
            UnlockedCount++;
        }
    }
    return static_cast<float>(UnlockedCount) / Achievements.Num();
}

int32 UMGPlatformIntegrationSubsystem::GetTotalGamerScore() const
{
    int32 TotalScore = 0;
    for (const auto& Pair : Achievements)
    {
        if (Pair.Value.Status == EMGAchievementStatus::Unlocked)
        {
            TotalScore += Pair.Value.PointValue;
        }
    }
    return TotalScore;
}

void UMGPlatformIntegrationSubsystem::ShowAchievementUI()
{
    // In a real implementation, open platform achievement overlay
    UE_LOG(LogTemp, Log, TEXT("Opening achievement UI"));
}

// ===== Rich Presence =====

void UMGPlatformIntegrationSubsystem::SetRichPresence(const FMGRichPresence& Presence)
{
    CurrentPresence = Presence;
    OnPresenceUpdated.Broadcast(CurrentPresence);

    // In a real implementation, sync with platform
    UE_LOG(LogTemp, Verbose, TEXT("Rich presence updated: %s"), *Presence.Details);
}

void UMGPlatformIntegrationSubsystem::SetPresenceState(EMGPresenceState State)
{
    CurrentPresence.State = State;
    OnPresenceUpdated.Broadcast(CurrentPresence);
}

void UMGPlatformIntegrationSubsystem::SetPresenceDetails(const FString& Details)
{
    CurrentPresence.Details = Details;
    OnPresenceUpdated.Broadcast(CurrentPresence);
}

void UMGPlatformIntegrationSubsystem::SetPresenceInRace(FName TrackId, FName VehicleId, int32 Position, int32 TotalRacers)
{
    CurrentPresence.State = EMGPresenceState::InRace;
    CurrentPresence.CurrentTrack = TrackId;
    CurrentPresence.CurrentVehicle = VehicleId;
    CurrentPresence.Details = FString::Printf(TEXT("Racing on %s - Position %d/%d"),
        *TrackId.ToString(), Position, TotalRacers);
    CurrentPresence.LargeImageKey = TrackId.ToString();
    CurrentPresence.SmallImageKey = VehicleId.ToString();

    OnPresenceUpdated.Broadcast(CurrentPresence);
}

void UMGPlatformIntegrationSubsystem::SetPresenceInGarage(FName VehicleId)
{
    CurrentPresence.State = EMGPresenceState::InGarage;
    CurrentPresence.CurrentVehicle = VehicleId;
    CurrentPresence.Details = FString::Printf(TEXT("Tuning %s in Garage"), *VehicleId.ToString());
    CurrentPresence.LargeImageKey = TEXT("garage");
    CurrentPresence.SmallImageKey = VehicleId.ToString();

    OnPresenceUpdated.Broadcast(CurrentPresence);
}

void UMGPlatformIntegrationSubsystem::SetPresenceInMenu(const FString& MenuName)
{
    CurrentPresence.State = EMGPresenceState::InMenu;
    CurrentPresence.Details = FString::Printf(TEXT("Browsing %s"), *MenuName);
    CurrentPresence.LargeImageKey = TEXT("menu");

    OnPresenceUpdated.Broadcast(CurrentPresence);
}

void UMGPlatformIntegrationSubsystem::SetPresenceMatchmaking(const FString& ModeSearching)
{
    CurrentPresence.State = EMGPresenceState::Matchmaking;
    CurrentPresence.Details = FString::Printf(TEXT("Searching for %s match"), *ModeSearching);
    CurrentPresence.LargeImageKey = TEXT("matchmaking");
    CurrentPresence.bJoinable = true;

    OnPresenceUpdated.Broadcast(CurrentPresence);
}

void UMGPlatformIntegrationSubsystem::ClearRichPresence()
{
    CurrentPresence = FMGRichPresence();
    CurrentPresence.State = EMGPresenceState::Online;
    OnPresenceUpdated.Broadcast(CurrentPresence);
}

FMGRichPresence UMGPlatformIntegrationSubsystem::GetCurrentPresence() const
{
    return CurrentPresence;
}

// ===== Entitlements =====

void UMGPlatformIntegrationSubsystem::RefreshEntitlements()
{
    // In a real implementation, query platform for entitlements
    UE_LOG(LogTemp, Log, TEXT("Refreshing entitlements"));

    // Simulate base game entitlement
    FMGEntitlement BaseGame;
    BaseGame.EntitlementId = FName("BaseGame");
    BaseGame.DisplayName = FText::FromString(TEXT("Midnight Grind"));
    BaseGame.Type = EMGEntitlementType::BaseGame;
    BaseGame.bOwned = true;
    BaseGame.PurchaseDate = FDateTime::Now() - FTimespan::FromDays(30);
    Entitlements.Add(BaseGame.EntitlementId, BaseGame);
}

bool UMGPlatformIntegrationSubsystem::HasEntitlement(FName EntitlementId) const
{
    const FMGEntitlement* Entitlement = Entitlements.Find(EntitlementId);
    return Entitlement && Entitlement->IsValid();
}

FMGEntitlement UMGPlatformIntegrationSubsystem::GetEntitlement(FName EntitlementId) const
{
    const FMGEntitlement* Entitlement = Entitlements.Find(EntitlementId);
    return Entitlement ? *Entitlement : FMGEntitlement();
}

TArray<FMGEntitlement> UMGPlatformIntegrationSubsystem::GetAllEntitlements() const
{
    TArray<FMGEntitlement> Result;
    Entitlements.GenerateValueArray(Result);
    return Result;
}

TArray<FMGEntitlement> UMGPlatformIntegrationSubsystem::GetOwnedEntitlements() const
{
    TArray<FMGEntitlement> Result;
    for (const auto& Pair : Entitlements)
    {
        if (Pair.Value.IsValid())
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

void UMGPlatformIntegrationSubsystem::ConsumeEntitlement(FName EntitlementId)
{
    FMGEntitlement* Entitlement = Entitlements.Find(EntitlementId);
    if (Entitlement && Entitlement->bOwned)
    {
        Entitlement->bConsumed = true;
        OnEntitlementUpdated.Broadcast(*Entitlement);
    }
}

bool UMGPlatformIntegrationSubsystem::HasDLC(FName DLCId) const
{
    const FMGEntitlement* Entitlement = Entitlements.Find(DLCId);
    return Entitlement && Entitlement->Type == EMGEntitlementType::DLC && Entitlement->IsValid();
}

bool UMGPlatformIntegrationSubsystem::HasSeasonPass() const
{
    for (const auto& Pair : Entitlements)
    {
        if (Pair.Value.Type == EMGEntitlementType::SeasonPass && Pair.Value.IsValid())
        {
            return true;
        }
    }
    return false;
}

// ===== Platform Stats =====

void UMGPlatformIntegrationSubsystem::SetPlatformStat(FName StatId, float Value)
{
    FMGPlatformStats* Stat = PlatformStats.Find(StatId);
    if (Stat)
    {
        Stat->Value = FMath::Clamp(Value, Stat->MinValue, Stat->MaxValue);
        Stat->LastUpdated = FDateTime::Now();
    }
    else
    {
        FMGPlatformStats NewStat;
        NewStat.StatId = StatId;
        NewStat.Value = Value;
        NewStat.LastUpdated = FDateTime::Now();
        PlatformStats.Add(StatId, NewStat);
    }
}

void UMGPlatformIntegrationSubsystem::IncrementPlatformStat(FName StatId, float Amount)
{
    float CurrentValue = GetPlatformStat(StatId);
    SetPlatformStat(StatId, CurrentValue + Amount);
}

float UMGPlatformIntegrationSubsystem::GetPlatformStat(FName StatId) const
{
    const FMGPlatformStats* Stat = PlatformStats.Find(StatId);
    return Stat ? Stat->Value : 0.0f;
}

void UMGPlatformIntegrationSubsystem::FlushPlatformStats()
{
    // In a real implementation, sync stats with platform
    UE_LOG(LogTemp, Log, TEXT("Flushing %d platform stats"), PlatformStats.Num());
}

// ===== Friends =====

TArray<FMGPlatformUser> UMGPlatformIntegrationSubsystem::GetFriendsList() const
{
    return FriendsList;
}

TArray<FMGPlatformUser> UMGPlatformIntegrationSubsystem::GetOnlineFriends() const
{
    TArray<FMGPlatformUser> OnlineFriends;
    for (const FMGPlatformUser& Friend : FriendsList)
    {
        if (Friend.bIsOnline)
        {
            OnlineFriends.Add(Friend);
        }
    }
    return OnlineFriends;
}

TArray<FMGPlatformUser> UMGPlatformIntegrationSubsystem::GetFriendsPlayingGame() const
{
    TArray<FMGPlatformUser> PlayingFriends;
    for (const FMGPlatformUser& Friend : FriendsList)
    {
        if (Friend.PresenceState == EMGPresenceState::InGame ||
            Friend.PresenceState == EMGPresenceState::InRace ||
            Friend.PresenceState == EMGPresenceState::InGarage)
        {
            PlayingFriends.Add(Friend);
        }
    }
    return PlayingFriends;
}

bool UMGPlatformIntegrationSubsystem::IsFriend(const FString& UserId) const
{
    for (const FMGPlatformUser& Friend : FriendsList)
    {
        if (Friend.PlatformUserId == UserId)
        {
            return true;
        }
    }
    return false;
}

void UMGPlatformIntegrationSubsystem::RefreshFriendsList()
{
    // In a real implementation, query platform for friends list
    UE_LOG(LogTemp, Log, TEXT("Refreshing friends list"));
}

void UMGPlatformIntegrationSubsystem::ShowFriendsUI()
{
    // In a real implementation, open platform friends overlay
    UE_LOG(LogTemp, Log, TEXT("Opening friends UI"));
}

// ===== Overlay =====

bool UMGPlatformIntegrationSubsystem::IsOverlayActive() const
{
    return bOverlayActive;
}

void UMGPlatformIntegrationSubsystem::ShowPlatformOverlay()
{
    // In a real implementation, activate platform overlay
    bOverlayActive = true;
    OnOverlayActivated.Broadcast();
    UE_LOG(LogTemp, Log, TEXT("Platform overlay activated"));
}

void UMGPlatformIntegrationSubsystem::ShowStorePageOverlay()
{
    UE_LOG(LogTemp, Log, TEXT("Opening store page"));
}

void UMGPlatformIntegrationSubsystem::ShowUserProfileOverlay(const FString& UserId)
{
    UE_LOG(LogTemp, Log, TEXT("Opening user profile: %s"), *UserId);
}

// ===== Screenshots/Video =====

void UMGPlatformIntegrationSubsystem::TriggerScreenshot()
{
    // In a real implementation, trigger platform screenshot
    UE_LOG(LogTemp, Log, TEXT("Screenshot triggered"));
}

void UMGPlatformIntegrationSubsystem::StartVideoCapture()
{
    if (!bVideoCapturing && Capabilities.bSupportsVideoCapture)
    {
        bVideoCapturing = true;
        UE_LOG(LogTemp, Log, TEXT("Video capture started"));
    }
}

void UMGPlatformIntegrationSubsystem::StopVideoCapture()
{
    if (bVideoCapturing)
    {
        bVideoCapturing = false;
        UE_LOG(LogTemp, Log, TEXT("Video capture stopped"));
    }
}

bool UMGPlatformIntegrationSubsystem::IsVideoCapturing() const
{
    return bVideoCapturing;
}

// ===== Internal Helpers =====

void UMGPlatformIntegrationSubsystem::DetectPlatform()
{
#if PLATFORM_WINDOWS
    CurrentPlatform = EMGPlatformType::PC_Steam; // Default to Steam on PC
#elif PLATFORM_PS4
    CurrentPlatform = EMGPlatformType::PlayStation4;
#elif PLATFORM_PS5
    CurrentPlatform = EMGPlatformType::PlayStation5;
#elif PLATFORM_XBOXONE
    CurrentPlatform = EMGPlatformType::XboxOne;
#elif PLATFORM_XSX
    CurrentPlatform = EMGPlatformType::XboxSeriesX;
#elif PLATFORM_SWITCH
    CurrentPlatform = EMGPlatformType::NintendoSwitch;
#elif PLATFORM_IOS
    CurrentPlatform = EMGPlatformType::Mobile_iOS;
#elif PLATFORM_ANDROID
    CurrentPlatform = EMGPlatformType::Mobile_Android;
#else
    CurrentPlatform = EMGPlatformType::Unknown;
#endif
}

void UMGPlatformIntegrationSubsystem::InitializePlatformServices()
{
    // Initialize capabilities based on platform
    Capabilities = FMGPlatformCapabilities();

    switch (CurrentPlatform)
    {
        case EMGPlatformType::PC_Steam:
            Capabilities.bSupportsUGC = true;
            Capabilities.MaxFriends = 2000;
            break;

        case EMGPlatformType::PlayStation4:
        case EMGPlatformType::PlayStation5:
            Capabilities.MaxPartySize = 16;
            break;

        case EMGPlatformType::XboxOne:
        case EMGPlatformType::XboxSeriesX:
            Capabilities.MaxPartySize = 12;
            break;

        case EMGPlatformType::NintendoSwitch:
            Capabilities.bSupportsStreaming = false;
            Capabilities.MaxPartySize = 8;
            break;

        case EMGPlatformType::Mobile_iOS:
        case EMGPlatformType::Mobile_Android:
            Capabilities.bSupportsOverlay = false;
            Capabilities.bSupportsVoiceChat = false;
            break;

        default:
            break;
    }

    // Refresh entitlements
    RefreshEntitlements();
}

void UMGPlatformIntegrationSubsystem::InitializeDefaultAchievements()
{
    // Racing achievements
    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("FirstRace");
        Achievement.DisplayName = FText::FromString(TEXT("First Steps"));
        Achievement.Description = FText::FromString(TEXT("Complete your first race"));
        Achievement.PointValue = 10;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("FirstWin");
        Achievement.DisplayName = FText::FromString(TEXT("Victory Lap"));
        Achievement.Description = FText::FromString(TEXT("Win your first race"));
        Achievement.PointValue = 15;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("Win100Races");
        Achievement.DisplayName = FText::FromString(TEXT("Century Racer"));
        Achievement.Description = FText::FromString(TEXT("Win 100 races"));
        Achievement.MaxProgress = 100.0f;
        Achievement.PointValue = 50;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("SpeedDemon");
        Achievement.DisplayName = FText::FromString(TEXT("Speed Demon"));
        Achievement.Description = FText::FromString(TEXT("Reach 200 MPH"));
        Achievement.PointValue = 25;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("DriftKing");
        Achievement.DisplayName = FText::FromString(TEXT("Drift King"));
        Achievement.Description = FText::FromString(TEXT("Score 100,000 drift points in a single race"));
        Achievement.PointValue = 30;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("Collector");
        Achievement.DisplayName = FText::FromString(TEXT("Car Collector"));
        Achievement.Description = FText::FromString(TEXT("Own 25 vehicles"));
        Achievement.MaxProgress = 25.0f;
        Achievement.PointValue = 40;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    {
        FMGPlatformAchievement Achievement;
        Achievement.AchievementId = FName("PlatinumTrophy");
        Achievement.DisplayName = FText::FromString(TEXT("Midnight Legend"));
        Achievement.Description = FText::FromString(TEXT("Unlock all other achievements"));
        Achievement.PointValue = 100;
        Achievement.bIsRare = true;
        Achievements.Add(Achievement.AchievementId, Achievement);
    }

    UE_LOG(LogTemp, Log, TEXT("Initialized %d platform achievements"), Achievements.Num());
}

void UMGPlatformIntegrationSubsystem::SyncAchievementsWithPlatform()
{
    // In a real implementation, sync achievement state with platform
    UE_LOG(LogTemp, Verbose, TEXT("Syncing achievements with platform"));
}

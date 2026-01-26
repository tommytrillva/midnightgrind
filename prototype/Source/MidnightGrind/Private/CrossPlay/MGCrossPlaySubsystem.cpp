// MidnightGrind - Arcade Street Racing Game
// Cross-Play Subsystem Implementation

#include "CrossPlay/MGCrossPlaySubsystem.h"

UMGCrossPlaySubsystem::UMGCrossPlaySubsystem()
    : LocalPlatform(EMGCrossPlayPlatform::Unknown)
    , LocalInputType(EMGInputType::Unknown)
    , bSettingsDirty(false)
{
}

void UMGCrossPlaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Detect local platform
    DetectLocalPlatform();

    // Detect input type
    DetectLocalInputType();

    // Load settings
    LoadCrossPlaySettings();

    // Initialize platform stats
    for (int32 i = 0; i <= static_cast<int32>(EMGCrossPlayPlatform::Mobile); i++)
    {
        EMGCrossPlayPlatform Platform = static_cast<EMGCrossPlayPlatform>(i);
        FMGPlatformStats Stats;
        Stats.Platform = Platform;
        PlatformStatsCache.Add(Platform, Stats);
    }

    UE_LOG(LogTemp, Log, TEXT("MGCrossPlaySubsystem initialized. Platform: %s, Input: %s"),
        *GetPlatformDisplayName(LocalPlatform), *GetInputTypeDisplayName(LocalInputType));
}

void UMGCrossPlaySubsystem::Deinitialize()
{
    // Save settings if dirty
    if (bSettingsDirty)
    {
        SaveCrossPlaySettings();
    }

    Super::Deinitialize();
}

// ===== Cross-Play Settings =====

void UMGCrossPlaySubsystem::SetCrossPlayEnabled(bool bEnabled)
{
    EMGCrossPlayStatus NewStatus = bEnabled ? EMGCrossPlayStatus::Enabled : EMGCrossPlayStatus::Disabled;
    if (Settings.Status != NewStatus)
    {
        Settings.Status = NewStatus;
        bSettingsDirty = true;
        OnCrossPlayStatusChanged.Broadcast(NewStatus);
        OnCrossPlaySettingsChanged.Broadcast(Settings);
    }
}

bool UMGCrossPlaySubsystem::IsCrossPlayEnabled() const
{
    return Settings.Status == EMGCrossPlayStatus::Enabled ||
           Settings.Status == EMGCrossPlayStatus::InputBased;
}

void UMGCrossPlaySubsystem::SetCrossPlaySettings(const FMGCrossPlaySettings& NewSettings)
{
    Settings = NewSettings;
    bSettingsDirty = true;
    OnCrossPlaySettingsChanged.Broadcast(Settings);
}

FMGCrossPlaySettings UMGCrossPlaySubsystem::GetCrossPlaySettings() const
{
    return Settings;
}

void UMGCrossPlaySubsystem::SetPoolingPreference(EMGCrossPlayPooling Pooling)
{
    Settings.Pooling = Pooling;
    bSettingsDirty = true;
    OnCrossPlaySettingsChanged.Broadcast(Settings);
}

EMGCrossPlayPooling UMGCrossPlaySubsystem::GetPoolingPreference() const
{
    return Settings.Pooling;
}

void UMGCrossPlaySubsystem::SetPlatformAllowed(EMGCrossPlayPlatform Platform, bool bAllowed)
{
    switch (Platform)
    {
        case EMGCrossPlayPlatform::PC:
            Settings.bAllowPCPlayers = bAllowed;
            break;
        case EMGCrossPlayPlatform::PlayStation:
        case EMGCrossPlayPlatform::Xbox:
        case EMGCrossPlayPlatform::Nintendo:
            Settings.bAllowConsolePlayers = bAllowed;
            break;
        case EMGCrossPlayPlatform::Mobile:
            Settings.bAllowMobilePlayers = bAllowed;
            break;
        default:
            break;
    }
    bSettingsDirty = true;
    OnCrossPlaySettingsChanged.Broadcast(Settings);
}

bool UMGCrossPlaySubsystem::IsPlatformAllowed(EMGCrossPlayPlatform Platform) const
{
    if (!IsCrossPlayEnabled())
    {
        return Platform == LocalPlatform;
    }

    switch (Platform)
    {
        case EMGCrossPlayPlatform::PC:
            return Settings.bAllowPCPlayers;
        case EMGCrossPlayPlatform::PlayStation:
        case EMGCrossPlayPlatform::Xbox:
        case EMGCrossPlayPlatform::Nintendo:
            return Settings.bAllowConsolePlayers;
        case EMGCrossPlayPlatform::Mobile:
            return Settings.bAllowMobilePlayers;
        default:
            return false;
    }
}

// ===== Platform Info =====

EMGCrossPlayPlatform UMGCrossPlaySubsystem::GetLocalPlatform() const
{
    return LocalPlatform;
}

EMGInputType UMGCrossPlaySubsystem::GetLocalInputType() const
{
    return LocalInputType;
}

void UMGCrossPlaySubsystem::SetLocalInputType(EMGInputType InputType)
{
    if (LocalInputType != InputType)
    {
        LocalInputType = InputType;

        // Notify about local input change if in session
        if (!CurrentSession.SessionId.IsEmpty())
        {
            // Find and update local player
            for (FMGCrossPlayPlayer& Player : CurrentSession.Players)
            {
                // In a real implementation, check if this is the local player
            }
        }
    }
}

FString UMGCrossPlaySubsystem::GetPlatformDisplayName(EMGCrossPlayPlatform Platform) const
{
    switch (Platform)
    {
        case EMGCrossPlayPlatform::PC: return TEXT("PC");
        case EMGCrossPlayPlatform::PlayStation: return TEXT("PlayStation");
        case EMGCrossPlayPlatform::Xbox: return TEXT("Xbox");
        case EMGCrossPlayPlatform::Nintendo: return TEXT("Nintendo Switch");
        case EMGCrossPlayPlatform::Mobile: return TEXT("Mobile");
        default: return TEXT("Unknown");
    }
}

FString UMGCrossPlaySubsystem::GetInputTypeDisplayName(EMGInputType InputType) const
{
    switch (InputType)
    {
        case EMGInputType::KeyboardMouse: return TEXT("Keyboard & Mouse");
        case EMGInputType::Controller: return TEXT("Controller");
        case EMGInputType::Touch: return TEXT("Touch");
        case EMGInputType::Wheel: return TEXT("Racing Wheel");
        default: return TEXT("Unknown");
    }
}

TSoftObjectPtr<UTexture2D> UMGCrossPlaySubsystem::GetPlatformIcon(EMGCrossPlayPlatform Platform) const
{
    // In a real implementation, return actual platform icon textures
    return TSoftObjectPtr<UTexture2D>();
}

// ===== Session Info =====

FMGCrossPlaySession UMGCrossPlaySubsystem::GetCurrentSession() const
{
    return CurrentSession;
}

bool UMGCrossPlaySubsystem::IsInCrossPlaySession() const
{
    return CurrentSession.bIsCrossPlaySession;
}

TArray<FMGCrossPlayPlayer> UMGCrossPlaySubsystem::GetSessionPlayers() const
{
    return CurrentSession.Players;
}

FMGCrossPlayPlayer UMGCrossPlaySubsystem::GetPlayer(const FString& PlayerId) const
{
    for (const FMGCrossPlayPlayer& Player : CurrentSession.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            return Player;
        }
    }
    return FMGCrossPlayPlayer();
}

int32 UMGCrossPlaySubsystem::GetPlatformPlayerCount(EMGCrossPlayPlatform Platform) const
{
    return CurrentSession.GetPlatformCount(Platform);
}

int32 UMGCrossPlaySubsystem::GetInputTypePlayerCount(EMGInputType InputType) const
{
    return CurrentSession.GetInputCount(InputType);
}

// ===== Matchmaking Support =====

TArray<EMGCrossPlayPlatform> UMGCrossPlaySubsystem::GetAllowedPlatforms() const
{
    TArray<EMGCrossPlayPlatform> Allowed;

    // Always allow own platform
    Allowed.Add(LocalPlatform);

    if (IsCrossPlayEnabled())
    {
        if (Settings.bAllowPCPlayers && LocalPlatform != EMGCrossPlayPlatform::PC)
        {
            Allowed.Add(EMGCrossPlayPlatform::PC);
        }

        if (Settings.bAllowConsolePlayers)
        {
            if (LocalPlatform != EMGCrossPlayPlatform::PlayStation)
                Allowed.Add(EMGCrossPlayPlatform::PlayStation);
            if (LocalPlatform != EMGCrossPlayPlatform::Xbox)
                Allowed.Add(EMGCrossPlayPlatform::Xbox);
            if (LocalPlatform != EMGCrossPlayPlatform::Nintendo)
                Allowed.Add(EMGCrossPlayPlatform::Nintendo);
        }

        if (Settings.bAllowMobilePlayers && LocalPlatform != EMGCrossPlayPlatform::Mobile)
        {
            Allowed.Add(EMGCrossPlayPlatform::Mobile);
        }
    }

    return Allowed;
}

bool UMGCrossPlaySubsystem::CanMatchWith(const FMGCrossPlayPlayer& Player) const
{
    // Check platform
    if (!CanMatchWithPlatform(Player.Platform))
    {
        return false;
    }

    // Check input preference
    if (Settings.bPreferSameInput && !CanMatchWithInput(Player.InputType))
    {
        return false;
    }

    // Check latency
    if (Player.Latency > Settings.MaxLatencyDifference)
    {
        return false;
    }

    // Check if player has cross-play enabled
    if (Player.Platform != LocalPlatform && !Player.bHasCrossPlayEnabled)
    {
        return false;
    }

    return true;
}

bool UMGCrossPlaySubsystem::CanMatchWithPlatform(EMGCrossPlayPlatform Platform) const
{
    return IsPlatformAllowed(Platform);
}

bool UMGCrossPlaySubsystem::CanMatchWithInput(EMGInputType InputType) const
{
    if (!Settings.bPreferSameInput)
    {
        return true;
    }

    // Group similar inputs
    bool bLocalIsController = LocalInputType == EMGInputType::Controller ||
                              LocalInputType == EMGInputType::Wheel;
    bool bOtherIsController = InputType == EMGInputType::Controller ||
                              InputType == EMGInputType::Wheel;

    return bLocalIsController == bOtherIsController;
}

void UMGCrossPlaySubsystem::SetPreferSameInput(bool bPrefer)
{
    Settings.bPreferSameInput = bPrefer;
    bSettingsDirty = true;
    OnCrossPlaySettingsChanged.Broadcast(Settings);
}

bool UMGCrossPlaySubsystem::GetPreferSameInput() const
{
    return Settings.bPreferSameInput;
}

// ===== Statistics =====

FMGPlatformStats UMGCrossPlaySubsystem::GetPlatformStats(EMGCrossPlayPlatform Platform) const
{
    const FMGPlatformStats* Stats = PlatformStatsCache.Find(Platform);
    return Stats ? *Stats : FMGPlatformStats();
}

TArray<FMGPlatformStats> UMGCrossPlaySubsystem::GetAllPlatformStats() const
{
    TArray<FMGPlatformStats> Result;
    PlatformStatsCache.GenerateValueArray(Result);
    return Result;
}

FMGCrossPlayReport UMGCrossPlaySubsystem::GetCrossPlayReport() const
{
    return CachedReport;
}

float UMGCrossPlaySubsystem::GetCrossPlayAdoptionRate() const
{
    return CachedReport.CrossPlayAdoptionRate;
}

// ===== Friends =====

bool UMGCrossPlaySubsystem::CanAddCrossPlatformFriend() const
{
    return Settings.bAllowCrossPlatformFriends;
}

void UMGCrossPlaySubsystem::SetAllowCrossPlatformFriends(bool bAllow)
{
    Settings.bAllowCrossPlatformFriends = bAllow;
    bSettingsDirty = true;
    OnCrossPlaySettingsChanged.Broadcast(Settings);
}

bool UMGCrossPlaySubsystem::GetAllowCrossPlatformFriends() const
{
    return Settings.bAllowCrossPlatformFriends;
}

// ===== Voice Chat =====

bool UMGCrossPlaySubsystem::CanUseCrossPlatformVoice() const
{
    return Settings.bAllowCrossPlatformVoice;
}

void UMGCrossPlaySubsystem::SetAllowCrossPlatformVoice(bool bAllow)
{
    Settings.bAllowCrossPlatformVoice = bAllow;
    bSettingsDirty = true;
    OnCrossPlaySettingsChanged.Broadcast(Settings);
}

bool UMGCrossPlaySubsystem::GetAllowCrossPlatformVoice() const
{
    return Settings.bAllowCrossPlatformVoice;
}

// ===== Session Management =====

void UMGCrossPlaySubsystem::RegisterPlayer(const FMGCrossPlayPlayer& Player)
{
    // Check if already registered
    for (const FMGCrossPlayPlayer& Existing : CurrentSession.Players)
    {
        if (Existing.PlayerId == Player.PlayerId)
        {
            return;
        }
    }

    CurrentSession.Players.Add(Player);

    // Update platform counts
    int32& PlatformCount = CurrentSession.PlatformCounts.FindOrAdd(Player.Platform);
    PlatformCount++;

    // Update input counts
    int32& InputCount = CurrentSession.InputCounts.FindOrAdd(Player.InputType);
    InputCount++;

    // Check if this makes it a cross-play session
    if (CurrentSession.PlatformCounts.Num() > 1)
    {
        CurrentSession.bIsCrossPlaySession = true;
    }

    UpdateSessionStats();
    OnCrossPlayPlayerJoined.Broadcast(Player);
}

void UMGCrossPlaySubsystem::UnregisterPlayer(const FString& PlayerId)
{
    int32 Index = CurrentSession.Players.IndexOfByPredicate([&PlayerId](const FMGCrossPlayPlayer& Player) {
        return Player.PlayerId == PlayerId;
    });

    if (Index != INDEX_NONE)
    {
        const FMGCrossPlayPlayer& Player = CurrentSession.Players[Index];

        // Update platform counts
        int32* PlatformCount = CurrentSession.PlatformCounts.Find(Player.Platform);
        if (PlatformCount)
        {
            (*PlatformCount)--;
            if (*PlatformCount <= 0)
            {
                CurrentSession.PlatformCounts.Remove(Player.Platform);
            }
        }

        // Update input counts
        int32* InputCount = CurrentSession.InputCounts.Find(Player.InputType);
        if (InputCount)
        {
            (*InputCount)--;
            if (*InputCount <= 0)
            {
                CurrentSession.InputCounts.Remove(Player.InputType);
            }
        }

        CurrentSession.Players.RemoveAt(Index);

        // Update cross-play status
        CurrentSession.bIsCrossPlaySession = CurrentSession.PlatformCounts.Num() > 1;

        UpdateSessionStats();
        OnCrossPlayPlayerLeft.Broadcast(PlayerId);
    }
}

void UMGCrossPlaySubsystem::UpdatePlayerInput(const FString& PlayerId, EMGInputType NewInput)
{
    for (FMGCrossPlayPlayer& Player : CurrentSession.Players)
    {
        if (Player.PlayerId == PlayerId)
        {
            EMGInputType OldInput = Player.InputType;
            Player.InputType = NewInput;

            // Update input counts
            int32* OldCount = CurrentSession.InputCounts.Find(OldInput);
            if (OldCount)
            {
                (*OldCount)--;
                if (*OldCount <= 0)
                {
                    CurrentSession.InputCounts.Remove(OldInput);
                }
            }

            int32& NewCount = CurrentSession.InputCounts.FindOrAdd(NewInput);
            NewCount++;

            OnInputTypeChanged.Broadcast(PlayerId, NewInput);
            break;
        }
    }
}

void UMGCrossPlaySubsystem::ClearSession()
{
    CurrentSession = FMGCrossPlaySession();
}

// ===== Persistence =====

void UMGCrossPlaySubsystem::SaveCrossPlaySettings()
{
    // In a real implementation, save to settings file
    UE_LOG(LogTemp, Log, TEXT("Cross-play settings saved"));
    bSettingsDirty = false;
}

void UMGCrossPlaySubsystem::LoadCrossPlaySettings()
{
    // In a real implementation, load from settings file
    Settings = FMGCrossPlaySettings();
    UE_LOG(LogTemp, Log, TEXT("Cross-play settings loaded"));
}

// ===== Internal Helpers =====

void UMGCrossPlaySubsystem::DetectLocalPlatform()
{
#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    LocalPlatform = EMGCrossPlayPlatform::PC;
#elif PLATFORM_PS4 || PLATFORM_PS5
    LocalPlatform = EMGCrossPlayPlatform::PlayStation;
#elif PLATFORM_XBOXONE || PLATFORM_XSX
    LocalPlatform = EMGCrossPlayPlatform::Xbox;
#elif PLATFORM_SWITCH
    LocalPlatform = EMGCrossPlayPlatform::Nintendo;
#elif PLATFORM_IOS || PLATFORM_ANDROID
    LocalPlatform = EMGCrossPlayPlatform::Mobile;
#else
    LocalPlatform = EMGCrossPlayPlatform::Unknown;
#endif
}

void UMGCrossPlaySubsystem::DetectLocalInputType()
{
    // Default based on platform
    switch (LocalPlatform)
    {
        case EMGCrossPlayPlatform::PC:
            LocalInputType = EMGInputType::KeyboardMouse;
            break;
        case EMGCrossPlayPlatform::PlayStation:
        case EMGCrossPlayPlatform::Xbox:
        case EMGCrossPlayPlatform::Nintendo:
            LocalInputType = EMGInputType::Controller;
            break;
        case EMGCrossPlayPlatform::Mobile:
            LocalInputType = EMGInputType::Touch;
            break;
        default:
            LocalInputType = EMGInputType::Unknown;
            break;
    }
}

void UMGCrossPlaySubsystem::UpdateSessionStats()
{
    if (CurrentSession.Players.Num() == 0)
    {
        CurrentSession.AverageLatency = 0.0f;
        CurrentSession.MaxLatency = 0;
        return;
    }

    float TotalLatency = 0.0f;
    int32 MaxLat = 0;

    for (const FMGCrossPlayPlayer& Player : CurrentSession.Players)
    {
        TotalLatency += Player.Latency;
        MaxLat = FMath::Max(MaxLat, static_cast<int32>(Player.Latency));
    }

    CurrentSession.AverageLatency = (CurrentSession.Players.Num() > 0) ? TotalLatency / CurrentSession.Players.Num() : 0;
    CurrentSession.MaxLatency = MaxLat;
}

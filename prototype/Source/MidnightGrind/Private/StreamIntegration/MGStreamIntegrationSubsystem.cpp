// MGStreamIntegrationSubsystem.cpp
// Stream Integration System - Implementation
// Midnight Grind - Y2K Arcade Street Racing

#include "StreamIntegration/MGStreamIntegrationSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGStreamIntegrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize settings
    Settings.bEnabled = true;
    Settings.bAllowViewerInteractions = true;
    Settings.bShowChatOverlay = true;
    Settings.bShowAlerts = true;
    Settings.bAllowPolls = true;
    Settings.bAllowPredictions = true;
    Settings.MaxEffectsPerMinute = 10;
    Settings.GlobalCooldownSeconds = 5.0f;
    Settings.ChatMessageLimit = 100;
    Settings.bFilterProfanity = true;

    EffectsThisMinute = 0;
    MinuteStartTime = FDateTime::Now();

    InitializeDefaultEffects();
    InitializeDefaultCommands();

    // Start timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(InteractionProcessTimer, this, &UMGStreamIntegrationSubsystem::ProcessInteractionQueue, 0.1f, true);
        TWeakObjectPtr<UMGStreamIntegrationSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(CooldownUpdateTimer, [WeakThis]() { if (WeakThis.IsValid()) WeakThis->UpdateCooldowns(1.0f); }, 1.0f, true);
    }

    UE_LOG(LogTemp, Log, TEXT("MGStreamIntegrationSubsystem initialized"));
}

void UMGStreamIntegrationSubsystem::Deinitialize()
{
    DisconnectAll();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(InteractionProcessTimer);
        World->GetTimerManager().ClearTimer(CooldownUpdateTimer);
        World->GetTimerManager().ClearTimer(ViewerCountUpdateTimer);
    }

    Super::Deinitialize();
}

void UMGStreamIntegrationSubsystem::InitializeDefaultEffects()
{
    // Spawn Obstacle
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("spawn_obstacle");
        Effect.DisplayName = TEXT("Spawn Obstacle");
        Effect.Description = TEXT("Spawns a random obstacle on the track");
        Effect.EffectType = EGameEffect::SpawnObstacle;
        Effect.Duration = 30.0f;
        Effect.PointsCost = 500;
        Effect.BitsCost = 100;
        Effect.CooldownSeconds = 120.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Speed Boost
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("speed_boost");
        Effect.DisplayName = TEXT("Speed Boost");
        Effect.Description = TEXT("Gives the streamer a temporary speed boost");
        Effect.EffectType = EGameEffect::SpeedBoost;
        Effect.Duration = 10.0f;
        Effect.Intensity = 1.25f;
        Effect.PointsCost = 300;
        Effect.BitsCost = 50;
        Effect.CooldownSeconds = 60.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Slow Down
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("slow_down");
        Effect.DisplayName = TEXT("Slow Down");
        Effect.Description = TEXT("Temporarily slows down the streamer");
        Effect.EffectType = EGameEffect::SlowDown;
        Effect.Duration = 5.0f;
        Effect.Intensity = 0.7f;
        Effect.PointsCost = 400;
        Effect.BitsCost = 75;
        Effect.CooldownSeconds = 90.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Change Weather
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("change_weather");
        Effect.DisplayName = TEXT("Weather Control");
        Effect.Description = TEXT("Changes the weather conditions");
        Effect.EffectType = EGameEffect::ChangeWeather;
        Effect.Duration = 60.0f;
        Effect.PointsCost = 1000;
        Effect.BitsCost = 200;
        Effect.CooldownSeconds = 300.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Nitro Refill
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("nitro_refill");
        Effect.DisplayName = TEXT("Nitro Refill");
        Effect.Description = TEXT("Refills the streamer's nitro tank");
        Effect.EffectType = EGameEffect::NitroRefill;
        Effect.Duration = 0.0f;
        Effect.PointsCost = 200;
        Effect.BitsCost = 25;
        Effect.CooldownSeconds = 45.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Random Event
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("random_event");
        Effect.DisplayName = TEXT("Random Event");
        Effect.Description = TEXT("Triggers a random game event");
        Effect.EffectType = EGameEffect::RandomEvent;
        Effect.Duration = 15.0f;
        Effect.PointsCost = 750;
        Effect.BitsCost = 150;
        Effect.CooldownSeconds = 180.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Traffic Increase
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("traffic_increase");
        Effect.DisplayName = TEXT("Traffic Chaos");
        Effect.Description = TEXT("Increases traffic density temporarily");
        Effect.EffectType = EGameEffect::TrafficIncrease;
        Effect.Duration = 30.0f;
        Effect.Intensity = 2.0f;
        Effect.PointsCost = 600;
        Effect.BitsCost = 100;
        Effect.CooldownSeconds = 120.0f;
        Effect.CooldownType = EInteractionCooldownType::Global;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }

    // Visual Effect
    {
        FGameEffectConfig Effect;
        Effect.EffectId = TEXT("visual_effect");
        Effect.DisplayName = TEXT("Screen Effect");
        Effect.Description = TEXT("Applies a fun visual effect to the screen");
        Effect.EffectType = EGameEffect::VisualEffect;
        Effect.Duration = 10.0f;
        Effect.PointsCost = 100;
        Effect.BitsCost = 10;
        Effect.CooldownSeconds = 30.0f;
        Effect.CooldownType = EInteractionCooldownType::PerUser;
        Effect.bEnabled = true;
        AvailableEffects.Add(Effect);
    }
}

void UMGStreamIntegrationSubsystem::InitializeDefaultCommands()
{
    // Stats command
    {
        FChatCommand Command;
        Command.CommandName = TEXT("stats");
        Command.Description = TEXT("Shows current race statistics");
        Command.Aliases = { TEXT("s"), TEXT("score") };
        Command.bEnabled = true;
        Command.CooldownSeconds = 10.0f;
        Command.ResponseTemplate = TEXT("Current Position: {position} | Lap: {lap} | Best Lap: {bestlap}");
        RegisteredCommands.Add(Command);
    }

    // Effects command
    {
        FChatCommand Command;
        Command.CommandName = TEXT("effects");
        Command.Description = TEXT("Lists available viewer effects");
        Command.Aliases = { TEXT("e"), TEXT("powers") };
        Command.bEnabled = true;
        Command.CooldownSeconds = 30.0f;
        Command.ResponseTemplate = TEXT("Available effects: !boost, !obstacle, !weather, !nitro");
        RegisteredCommands.Add(Command);
    }

    // Vehicle command
    {
        FChatCommand Command;
        Command.CommandName = TEXT("vehicle");
        Command.Description = TEXT("Shows current vehicle info");
        Command.Aliases = { TEXT("car"), TEXT("ride") };
        Command.bEnabled = true;
        Command.CooldownSeconds = 15.0f;
        Command.ResponseTemplate = TEXT("Currently driving: {vehiclename} | Top Speed: {topspeed}");
        RegisteredCommands.Add(Command);
    }

    // Track command
    {
        FChatCommand Command;
        Command.CommandName = TEXT("track");
        Command.Description = TEXT("Shows current track info");
        Command.Aliases = { TEXT("map"), TEXT("course") };
        Command.bEnabled = true;
        Command.CooldownSeconds = 15.0f;
        Command.ResponseTemplate = TEXT("Track: {trackname} | Laps: {totallaps}");
        RegisteredCommands.Add(Command);
    }
}

// ============================================================================
// Connection
// ============================================================================

void UMGStreamIntegrationSubsystem::ConnectToStream(EStreamPlatform Platform, const FStreamCredentials& InCredentials)
{
    if (ConnectionStatus.Contains(Platform) && ConnectionStatus[Platform] == EStreamStatus::Connected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already connected to %d"), static_cast<int32>(Platform));
        return;
    }

    ConnectionStatus.Add(Platform, EStreamStatus::Connecting);
    Credentials.Add(Platform, InCredentials);

    // Simulate connection
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGStreamIntegrationSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimerForNextTick([WeakThis, Platform]()
        {
            if (!WeakThis.IsValid())
            {
                return;
            }
            // Simulate successful connection
            WeakThis->ConnectionStatus[Platform] = EStreamStatus::Connected;

            FStreamInfo Info;
            Info.Platform = Platform;
            Info.StreamTitle = TEXT("Midnight Grind Stream");
            Info.Category = TEXT("Racing Games");
            Info.bIsLive = true;
            Info.StreamStartTime = FDateTime::Now();
            Info.ViewerCount = FMath::RandRange(50, 500);
            WeakThis->StreamInfos.Add(Platform, Info);

            WeakThis->OnStreamConnected.Broadcast(Platform);
            UE_LOG(LogTemp, Log, TEXT("Connected to stream platform: %d"), static_cast<int32>(Platform));
        });
    }
}

void UMGStreamIntegrationSubsystem::Disconnect(EStreamPlatform Platform)
{
    if (!ConnectionStatus.Contains(Platform) || ConnectionStatus[Platform] == EStreamStatus::Disconnected)
    {
        return;
    }

    ConnectionStatus[Platform] = EStreamStatus::Disconnected;
    StreamInfos.Remove(Platform);

    OnStreamDisconnected.Broadcast(Platform, TEXT("User disconnected"));
    UE_LOG(LogTemp, Log, TEXT("Disconnected from stream platform: %d"), static_cast<int32>(Platform));
}

void UMGStreamIntegrationSubsystem::DisconnectAll()
{
    TArray<EStreamPlatform> Platforms;
    ConnectionStatus.GetKeys(Platforms);

    for (EStreamPlatform Platform : Platforms)
    {
        Disconnect(Platform);
    }
}

EStreamStatus UMGStreamIntegrationSubsystem::GetConnectionStatus(EStreamPlatform Platform) const
{
    if (ConnectionStatus.Contains(Platform))
    {
        return ConnectionStatus[Platform];
    }
    return EStreamStatus::Disconnected;
}

bool UMGStreamIntegrationSubsystem::IsConnected(EStreamPlatform Platform) const
{
    return ConnectionStatus.Contains(Platform) && ConnectionStatus[Platform] == EStreamStatus::Connected;
}

bool UMGStreamIntegrationSubsystem::IsAnyStreamConnected() const
{
    for (const auto& Pair : ConnectionStatus)
    {
        if (Pair.Value == EStreamStatus::Connected)
        {
            return true;
        }
    }
    return false;
}

void UMGStreamIntegrationSubsystem::RefreshToken(EStreamPlatform Platform)
{
    if (Credentials.Contains(Platform))
    {
        FStreamCredentials& Creds = Credentials[Platform];
        Creds.TokenExpiry = FDateTime::Now() + FTimespan::FromHours(4);
        Creds.bIsValid = true;
    }
}

// ============================================================================
// Stream Info
// ============================================================================

FStreamInfo UMGStreamIntegrationSubsystem::GetStreamInfo(EStreamPlatform Platform) const
{
    if (StreamInfos.Contains(Platform))
    {
        return StreamInfos[Platform];
    }
    return FStreamInfo();
}

void UMGStreamIntegrationSubsystem::UpdateStreamTitle(EStreamPlatform Platform, const FString& NewTitle)
{
    if (StreamInfos.Contains(Platform))
    {
        StreamInfos[Platform].StreamTitle = NewTitle;
    }
}

void UMGStreamIntegrationSubsystem::UpdateStreamCategory(EStreamPlatform Platform, const FString& NewCategory)
{
    if (StreamInfos.Contains(Platform))
    {
        StreamInfos[Platform].Category = NewCategory;
    }
}

int32 UMGStreamIntegrationSubsystem::GetTotalViewerCount() const
{
    int32 Total = 0;
    for (const auto& Pair : StreamInfos)
    {
        Total += Pair.Value.ViewerCount;
    }
    return Total;
}

// ============================================================================
// Chat
// ============================================================================

void UMGStreamIntegrationSubsystem::SendChatMessage(EStreamPlatform Platform, const FString& Message)
{
    if (!IsConnected(Platform))
    {
        return;
    }

    // Would send via platform API
    UE_LOG(LogTemp, Log, TEXT("Sending chat message to %d: %s"), static_cast<int32>(Platform), *Message);
}

TArray<FChatMessage> UMGStreamIntegrationSubsystem::GetRecentMessages(int32 Count) const
{
    TArray<FChatMessage> Recent;

    int32 StartIndex = FMath::Max(0, ChatHistory.Num() - Count);
    for (int32 i = StartIndex; i < ChatHistory.Num(); i++)
    {
        Recent.Add(ChatHistory[i]);
    }

    return Recent;
}

void UMGStreamIntegrationSubsystem::ClearChat(EStreamPlatform Platform)
{
    // Would clear chat via platform API
    UE_LOG(LogTemp, Log, TEXT("Clearing chat for platform: %d"), static_cast<int32>(Platform));
}

void UMGStreamIntegrationSubsystem::TimeoutUser(EStreamPlatform Platform, const FString& UserId, int32 Seconds)
{
    // Would timeout user via platform API
    UE_LOG(LogTemp, Log, TEXT("Timing out user %s for %d seconds"), *UserId, Seconds);
}

void UMGStreamIntegrationSubsystem::BanUser(EStreamPlatform Platform, const FString& UserId, const FString& Reason)
{
    // Would ban user via platform API
    UE_LOG(LogTemp, Log, TEXT("Banning user %s: %s"), *UserId, *Reason);
}

// ============================================================================
// Commands
// ============================================================================

void UMGStreamIntegrationSubsystem::RegisterCommand(const FChatCommand& Command)
{
    // Check for duplicate
    for (const FChatCommand& Existing : RegisteredCommands)
    {
        if (Existing.CommandName == Command.CommandName)
        {
            return;
        }
    }

    RegisteredCommands.Add(Command);
}

void UMGStreamIntegrationSubsystem::UnregisterCommand(const FString& CommandName)
{
    for (int32 i = RegisteredCommands.Num() - 1; i >= 0; i--)
    {
        if (RegisteredCommands[i].CommandName == CommandName)
        {
            RegisteredCommands.RemoveAt(i);
            return;
        }
    }
}

void UMGStreamIntegrationSubsystem::SetCommandEnabled(const FString& CommandName, bool bEnabled)
{
    for (FChatCommand& Command : RegisteredCommands)
    {
        if (Command.CommandName == CommandName)
        {
            Command.bEnabled = bEnabled;
            return;
        }
    }
}

bool UMGStreamIntegrationSubsystem::ProcessChatCommand(const FChatMessage& Message)
{
    if (!Message.bIsCommand || Message.CommandName.IsEmpty())
    {
        return false;
    }

    for (const FChatCommand& Command : RegisteredCommands)
    {
        if (!Command.bEnabled)
        {
            continue;
        }

        bool bMatch = Command.CommandName.Equals(Message.CommandName, ESearchCase::IgnoreCase);
        if (!bMatch)
        {
            for (const FString& Alias : Command.Aliases)
            {
                if (Alias.Equals(Message.CommandName, ESearchCase::IgnoreCase))
                {
                    bMatch = true;
                    break;
                }
            }
        }

        if (bMatch)
        {
            // Check permissions
            if (Command.bModOnly && !Message.Sender.bIsModerator)
            {
                return false;
            }
            if (Command.bSubOnly && !Message.Sender.bIsSubscriber)
            {
                return false;
            }

            // Check cooldown
            if (!CheckCooldown(Command.CommandName, Message.Sender.ViewerId))
            {
                return false;
            }

            // Process command
            StartCooldown(Command.CommandName, Message.Sender.ViewerId, Command.CooldownSeconds);

            // Trigger linked effect if exists
            if (!Command.LinkedEffect.EffectId.IsEmpty())
            {
                TriggerEffect(Command.LinkedEffect.EffectId, Message.Sender);
            }

            Stats.TotalInteractions++;
            return true;
        }
    }

    return false;
}

// ============================================================================
// Game Effects
// ============================================================================

void UMGStreamIntegrationSubsystem::RegisterGameEffect(const FGameEffectConfig& Effect)
{
    for (const FGameEffectConfig& Existing : AvailableEffects)
    {
        if (Existing.EffectId == Effect.EffectId)
        {
            return;
        }
    }

    AvailableEffects.Add(Effect);
}

void UMGStreamIntegrationSubsystem::UnregisterGameEffect(const FString& EffectId)
{
    for (int32 i = AvailableEffects.Num() - 1; i >= 0; i--)
    {
        if (AvailableEffects[i].EffectId == EffectId)
        {
            AvailableEffects.RemoveAt(i);
            return;
        }
    }
}

bool UMGStreamIntegrationSubsystem::TriggerEffect(const FString& EffectId, const FViewerInfo& Viewer)
{
    if (!Settings.bEnabled || !Settings.bAllowViewerInteractions)
    {
        return false;
    }

    // Check rate limit
    if (FDateTime::Now() - MinuteStartTime > FTimespan::FromMinutes(1))
    {
        EffectsThisMinute = 0;
        MinuteStartTime = FDateTime::Now();
    }

    if (EffectsThisMinute >= Settings.MaxEffectsPerMinute)
    {
        return false;
    }

    // Find effect
    FGameEffectConfig* Effect = nullptr;
    for (FGameEffectConfig& E : AvailableEffects)
    {
        if (E.EffectId == EffectId)
        {
            Effect = &E;
            break;
        }
    }

    if (!Effect || !Effect->bEnabled)
    {
        return false;
    }

    // Check permissions
    if (Effect->bRequiresSubscriber && !Viewer.bIsSubscriber)
    {
        return false;
    }
    if (Effect->bRequiresModerator && !Viewer.bIsModerator)
    {
        return false;
    }

    // Check cooldown
    if (!CheckCooldown(EffectId, Viewer.ViewerId))
    {
        return false;
    }

    // Trigger effect
    StartCooldown(EffectId, Viewer.ViewerId, Effect->CooldownSeconds);
    EffectsThisMinute++;

    // Update stats
    Stats.TotalEffectsTriggered++;
    if (Stats.EffectUsageCounts.Contains(EffectId))
    {
        Stats.EffectUsageCounts[EffectId]++;
    }
    else
    {
        Stats.EffectUsageCounts.Add(EffectId, 1);
    }

    OnGameEffectTriggered.Broadcast(*Effect);

    UE_LOG(LogTemp, Log, TEXT("Effect triggered: %s by %s"), *EffectId, *Viewer.DisplayName);

    return true;
}

void UMGStreamIntegrationSubsystem::SetEffectEnabled(const FString& EffectId, bool bEnabled)
{
    for (FGameEffectConfig& Effect : AvailableEffects)
    {
        if (Effect.EffectId == EffectId)
        {
            Effect.bEnabled = bEnabled;
            return;
        }
    }
}

bool UMGStreamIntegrationSubsystem::IsEffectOnCooldown(const FString& EffectId) const
{
    return GlobalCooldowns.Contains(EffectId) && GlobalCooldowns[EffectId] > FDateTime::Now();
}

float UMGStreamIntegrationSubsystem::GetEffectCooldownRemaining(const FString& EffectId) const
{
    if (GlobalCooldowns.Contains(EffectId))
    {
        FTimespan Remaining = GlobalCooldowns[EffectId] - FDateTime::Now();
        return FMath::Max(0.0f, static_cast<float>(Remaining.GetTotalSeconds()));
    }
    return 0.0f;
}

// ============================================================================
// Polls
// ============================================================================

void UMGStreamIntegrationSubsystem::CreatePoll(const FString& Title, const TArray<FString>& Options, float Duration)
{
    if (!Settings.bAllowPolls || HasActivePoll())
    {
        return;
    }

    ActivePoll.PollId = FGuid::NewGuid();
    ActivePoll.Title = Title;
    ActivePoll.PollType = EPollType::Custom;
    ActivePoll.Options = Options;
    ActivePoll.Votes.Empty();
    ActivePoll.TotalVotes = 0;
    ActivePoll.DurationSeconds = Duration;
    ActivePoll.StartTime = FDateTime::Now();
    ActivePoll.EndTime = FDateTime::Now() + FTimespan::FromSeconds(Duration);
    ActivePoll.bIsActive = true;

    for (const FString& Option : Options)
    {
        ActivePoll.Votes.Add(Option, 0);
    }

    Stats.PollsCreated++;

    OnPollStarted.Broadcast(ActivePoll);

    // Set timer to end poll
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGStreamIntegrationSubsystem> WeakThis(this);
        FGuid PollId = ActivePoll.PollId;
        World->GetTimerManager().SetTimer(ViewerCountUpdateTimer, [WeakThis, PollId]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->EndPoll(PollId);
            }
        }, Duration, false);
    }
}

void UMGStreamIntegrationSubsystem::EndPoll(const FGuid& PollId)
{
    if (!ActivePoll.bIsActive || ActivePoll.PollId != PollId)
    {
        return;
    }

    ActivePoll.bIsActive = false;

    // Determine winner
    int32 HighestVotes = 0;
    for (const auto& Pair : ActivePoll.Votes)
    {
        if (Pair.Value > HighestVotes)
        {
            HighestVotes = Pair.Value;
            ActivePoll.WinningOption = Pair.Key;
        }
    }

    OnPollEnded.Broadcast(ActivePoll);
}

void UMGStreamIntegrationSubsystem::CancelPoll(const FGuid& PollId)
{
    if (ActivePoll.bIsActive && ActivePoll.PollId == PollId)
    {
        ActivePoll.bIsActive = false;
        ActivePoll.WinningOption = TEXT("");
    }
}

FStreamPoll UMGStreamIntegrationSubsystem::GetActivePoll() const
{
    return ActivePoll;
}

bool UMGStreamIntegrationSubsystem::HasActivePoll() const
{
    return ActivePoll.bIsActive;
}

void UMGStreamIntegrationSubsystem::Vote(const FGuid& PollId, const FString& Option, const FViewerInfo& Voter)
{
    if (!ActivePoll.bIsActive || ActivePoll.PollId != PollId)
    {
        return;
    }

    if (ActivePoll.Votes.Contains(Option))
    {
        ActivePoll.Votes[Option]++;
        ActivePoll.TotalVotes++;
    }
}

// ============================================================================
// Predictions
// ============================================================================

void UMGStreamIntegrationSubsystem::CreatePrediction(const FString& Title, const TArray<FString>& Outcomes, float WindowSeconds)
{
    if (!Settings.bAllowPredictions)
    {
        return;
    }

    ActivePrediction.PredictionId = FGuid::NewGuid();
    ActivePrediction.Title = Title;
    ActivePrediction.Outcomes = Outcomes;
    ActivePrediction.OutcomePoints.Empty();
    ActivePrediction.OutcomePredictors.Empty();
    ActivePrediction.TotalPoints = 0;
    ActivePrediction.WindowSeconds = WindowSeconds;
    ActivePrediction.StartTime = FDateTime::Now();
    ActivePrediction.bIsActive = true;
    ActivePrediction.bIsLocked = false;

    for (const FString& Outcome : Outcomes)
    {
        ActivePrediction.OutcomePoints.Add(Outcome, 0);
        ActivePrediction.OutcomePredictors.Add(Outcome, 0);
    }

    Stats.PredictionsCreated++;

    OnPredictionStarted.Broadcast(ActivePrediction);
}

void UMGStreamIntegrationSubsystem::LockPrediction(const FGuid& PredictionId)
{
    if (ActivePrediction.bIsActive && ActivePrediction.PredictionId == PredictionId)
    {
        ActivePrediction.bIsLocked = true;
    }
}

void UMGStreamIntegrationSubsystem::ResolvePrediction(const FGuid& PredictionId, const FString& WinningOutcome)
{
    if (!ActivePrediction.bIsActive || ActivePrediction.PredictionId != PredictionId)
    {
        return;
    }

    ActivePrediction.bIsActive = false;
    ActivePrediction.WinningOutcome = WinningOutcome;

    OnPredictionResolved.Broadcast(ActivePrediction);
}

void UMGStreamIntegrationSubsystem::CancelPrediction(const FGuid& PredictionId)
{
    if (ActivePrediction.bIsActive && ActivePrediction.PredictionId == PredictionId)
    {
        ActivePrediction.bIsActive = false;
        ActivePrediction.WinningOutcome = TEXT("");
    }
}

FStreamPrediction UMGStreamIntegrationSubsystem::GetActivePrediction() const
{
    return ActivePrediction;
}

void UMGStreamIntegrationSubsystem::PlacePrediction(const FGuid& PredictionId, const FString& Outcome, int32 Points, const FViewerInfo& Viewer)
{
    if (!ActivePrediction.bIsActive || ActivePrediction.bIsLocked || ActivePrediction.PredictionId != PredictionId)
    {
        return;
    }

    if (ActivePrediction.OutcomePoints.Contains(Outcome))
    {
        ActivePrediction.OutcomePoints[Outcome] += Points;
        ActivePrediction.OutcomePredictors[Outcome]++;
        ActivePrediction.TotalPoints += Points;
    }
}

// ============================================================================
// Alerts
// ============================================================================

void UMGStreamIntegrationSubsystem::QueueAlert(const FStreamAlert& Alert)
{
    if (!Settings.bShowAlerts)
    {
        return;
    }

    PendingAlerts.Add(Alert);
    OnStreamAlertReceived.Broadcast(Alert);
}

FStreamAlert UMGStreamIntegrationSubsystem::GetNextAlert()
{
    if (PendingAlerts.Num() > 0)
    {
        return PendingAlerts[0];
    }
    return FStreamAlert();
}

void UMGStreamIntegrationSubsystem::MarkAlertDisplayed(const FGuid& AlertId)
{
    for (int32 i = PendingAlerts.Num() - 1; i >= 0; i--)
    {
        if (PendingAlerts[i].AlertId == AlertId)
        {
            PendingAlerts.RemoveAt(i);
            return;
        }
    }
}

void UMGStreamIntegrationSubsystem::ClearAlerts()
{
    PendingAlerts.Empty();
}

// ============================================================================
// Settings
// ============================================================================

void UMGStreamIntegrationSubsystem::UpdateSettings(const FStreamIntegrationSettings& NewSettings)
{
    Settings = NewSettings;
}

void UMGStreamIntegrationSubsystem::SetViewerInteractionsEnabled(bool bEnabled)
{
    Settings.bAllowViewerInteractions = bEnabled;
}

void UMGStreamIntegrationSubsystem::SetSubOnlyMode(bool bEnabled)
{
    Settings.bSubOnlyMode = bEnabled;
}

// ============================================================================
// Statistics
// ============================================================================

void UMGStreamIntegrationSubsystem::ResetStats()
{
    Stats = FStreamIntegrationStats();
}

// ============================================================================
// Internal Helpers
// ============================================================================

void UMGStreamIntegrationSubsystem::ProcessInteractionQueue()
{
    if (InteractionQueue.Num() == 0)
    {
        return;
    }

    // Process one interaction per tick
    FViewerInteraction Interaction = InteractionQueue[0];
    InteractionQueue.RemoveAt(0);

    if (!Interaction.bProcessed)
    {
        Interaction.bProcessed = true;
        OnViewerInteraction.Broadcast(Interaction);
    }
}

void UMGStreamIntegrationSubsystem::UpdateCooldowns(float DeltaTime)
{
    // Clean up expired cooldowns
    FDateTime Now = FDateTime::Now();

    TArray<FString> ExpiredGlobal;
    for (const auto& Pair : GlobalCooldowns)
    {
        if (Pair.Value <= Now)
        {
            ExpiredGlobal.Add(Pair.Key);
        }
    }

    for (const FString& Key : ExpiredGlobal)
    {
        GlobalCooldowns.Remove(Key);
    }
}

bool UMGStreamIntegrationSubsystem::CheckCooldown(const FString& EffectId, const FString& ViewerId)
{
    FDateTime Now = FDateTime::Now();

    // Check global cooldown
    if (GlobalCooldowns.Contains(EffectId) && GlobalCooldowns[EffectId] > Now)
    {
        return false;
    }

    // Check user cooldown
    if (UserCooldowns.Contains(EffectId))
    {
        const TMap<FString, FDateTime>& UserCDs = UserCooldowns[EffectId];
        if (UserCDs.Contains(ViewerId) && UserCDs[ViewerId] > Now)
        {
            return false;
        }
    }

    return true;
}

void UMGStreamIntegrationSubsystem::StartCooldown(const FString& EffectId, const FString& ViewerId, float Duration)
{
    FDateTime EndTime = FDateTime::Now() + FTimespan::FromSeconds(Duration);

    // Find effect to check cooldown type
    EInteractionCooldownType CooldownType = EInteractionCooldownType::Global;
    for (const FGameEffectConfig& Effect : AvailableEffects)
    {
        if (Effect.EffectId == EffectId)
        {
            CooldownType = Effect.CooldownType;
            break;
        }
    }

    switch (CooldownType)
    {
        case EInteractionCooldownType::Global:
            GlobalCooldowns.Add(EffectId, EndTime);
            break;

        case EInteractionCooldownType::PerUser:
            if (!UserCooldowns.Contains(EffectId))
            {
                UserCooldowns.Add(EffectId, TMap<FString, FDateTime>());
            }
            UserCooldowns[EffectId].Add(ViewerId, EndTime);
            break;

        case EInteractionCooldownType::PerEffect:
            GlobalCooldowns.Add(EffectId, EndTime);
            break;
    }
}

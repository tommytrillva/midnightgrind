// MGStreamIntegrationSubsystem.h
// Stream Integration System - Twitch/YouTube integration, viewer interactions
// Midnight Grind - Y2K Arcade Street Racing

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStreamIntegrationSubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EStreamPlatform : uint8
{
    Twitch,
    YouTube,
    Kick,
    Facebook,
    TikTok,
    Custom
};

UENUM(BlueprintType)
enum class EStreamStatus : uint8
{
    Disconnected,
    Connecting,
    Connected,
    Streaming,
    Error
};

UENUM(BlueprintType)
enum class EViewerInteractionType : uint8
{
    ChatCommand,
    ChannelPoints,
    Bits,
    Subscription,
    Raid,
    Poll,
    Prediction,
    CustomReward
};

UENUM(BlueprintType)
enum class EGameEffect : uint8
{
    SpawnObstacle,
    SpawnPowerup,
    ChangeWeather,
    SpeedBoost,
    SlowDown,
    RandomEvent,
    NitroRefill,
    VehicleSwap,
    TrafficIncrease,
    VisualEffect,
    SoundEffect,
    CustomEffect
};

UENUM(BlueprintType)
enum class EPollType : uint8
{
    TrackSelection,
    WeatherChoice,
    VehicleChoice,
    EventTrigger,
    TeamSelection,
    Custom
};

UENUM(BlueprintType)
enum class EAlertType : uint8
{
    Follow,
    Subscribe,
    Bits,
    Donation,
    Raid,
    Host,
    ChannelPoints,
    Milestone
};

UENUM(BlueprintType)
enum class EInteractionCooldownType : uint8
{
    PerUser,
    Global,
    PerEffect
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FStreamCredentials
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStreamPlatform Platform = EStreamPlatform::Twitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClientId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AccessToken;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RefreshToken;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChannelId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ChannelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TokenExpiry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsValid = false;
};

USTRUCT(BlueprintType)
struct FStreamInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStreamPlatform Platform = EStreamPlatform::Twitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString StreamTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ViewerCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FollowerCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SubscriberCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StreamStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTimespan StreamDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;
};

USTRUCT(BlueprintType)
struct FViewerInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ViewerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EStreamPlatform Platform = EStreamPlatform::Twitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSubscriber = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsModerator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVIP = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SubscriptionMonths = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ChannelPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor NameColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Badges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastInteraction;
};

USTRUCT(BlueprintType)
struct FChatMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid MessageId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FViewerInfo Sender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCommand = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CommandName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> CommandArgs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Emotes;
};

USTRUCT(BlueprintType)
struct FViewerInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid InteractionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EViewerInteractionType Type = EViewerInteractionType::ChatCommand;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FViewerInfo Viewer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InteractionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGameEffect TriggeredEffect = EGameEffect::CustomEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bProcessed = false;
};

USTRUCT(BlueprintType)
struct FGameEffectConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EffectId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGameEffect EffectType = EGameEffect::CustomEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointsCost = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BitsCost = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EInteractionCooldownType CooldownType = EInteractionCooldownType::Global;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresSubscriber = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresModerator = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> CustomParams;
};

USTRUCT(BlueprintType)
struct FChatCommand
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CommandName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Aliases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bModOnly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSubOnly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CooldownSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResponseTemplate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameEffectConfig LinkedEffect;
};

USTRUCT(BlueprintType)
struct FStreamPoll
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid PollId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPollType PollType = EPollType::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Options;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> Votes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalVotes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DurationSeconds = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPointsVoting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointsPerVote = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WinningOption;
};

USTRUCT(BlueprintType)
struct FStreamPrediction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid PredictionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Outcomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> OutcomePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> OutcomePredictors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindowSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString WinningOutcome;
};

USTRUCT(BlueprintType)
struct FStreamAlert
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid AlertId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAlertType Type = EAlertType::Follow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FViewerInfo Viewer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDisplayed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration = 5.0f;
};

USTRUCT(BlueprintType)
struct FStreamIntegrationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowViewerInteractions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowChatOverlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowAlerts = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowPolls = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowPredictions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxEffectsPerMinute = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GlobalCooldownSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ChatMessageLimit = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFilterProfanity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSubOnlyMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> BlockedWords;
};

USTRUCT(BlueprintType)
struct FStreamIntegrationStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalChatMessages = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalEffectsTriggered = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalBitsReceived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalChannelPointsSpent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PollsCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PredictionsCreated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PeakViewers = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageViewers = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> EffectUsageCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> TopInteractors;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamConnected, EStreamPlatform, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStreamDisconnected, EStreamPlatform, Platform, const FString&, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChatMessageReceived, const FChatMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewerInteraction, const FViewerInteraction&, Interaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEffectTriggered, const FGameEffectConfig&, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPollStarted, const FStreamPoll&, Poll);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPollEnded, const FStreamPoll&, Poll);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPredictionStarted, const FStreamPrediction&, Prediction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPredictionResolved, const FStreamPrediction&, Prediction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStreamAlertReceived, const FStreamAlert&, Alert);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnViewerCountChanged, int32, NewCount);

// ============================================================================
// Main Subsystem
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGStreamIntegrationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // Connection
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Connection")
    void ConnectToStream(EStreamPlatform Platform, const FStreamCredentials& Credentials);

    UFUNCTION(BlueprintCallable, Category = "Stream|Connection")
    void Disconnect(EStreamPlatform Platform);

    UFUNCTION(BlueprintCallable, Category = "Stream|Connection")
    void DisconnectAll();

    UFUNCTION(BlueprintPure, Category = "Stream|Connection")
    EStreamStatus GetConnectionStatus(EStreamPlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "Stream|Connection")
    bool IsConnected(EStreamPlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "Stream|Connection")
    bool IsAnyStreamConnected() const;

    UFUNCTION(BlueprintCallable, Category = "Stream|Connection")
    void RefreshToken(EStreamPlatform Platform);

    // ========================================================================
    // Stream Info
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Stream|Info")
    FStreamInfo GetStreamInfo(EStreamPlatform Platform) const;

    UFUNCTION(BlueprintCallable, Category = "Stream|Info")
    void UpdateStreamTitle(EStreamPlatform Platform, const FString& NewTitle);

    UFUNCTION(BlueprintCallable, Category = "Stream|Info")
    void UpdateStreamCategory(EStreamPlatform Platform, const FString& NewCategory);

    UFUNCTION(BlueprintPure, Category = "Stream|Info")
    int32 GetTotalViewerCount() const;

    // ========================================================================
    // Chat
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Chat")
    void SendChatMessage(EStreamPlatform Platform, const FString& Message);

    UFUNCTION(BlueprintPure, Category = "Stream|Chat")
    TArray<FChatMessage> GetRecentMessages(int32 Count) const;

    UFUNCTION(BlueprintCallable, Category = "Stream|Chat")
    void ClearChat(EStreamPlatform Platform);

    UFUNCTION(BlueprintCallable, Category = "Stream|Chat")
    void TimeoutUser(EStreamPlatform Platform, const FString& UserId, int32 Seconds);

    UFUNCTION(BlueprintCallable, Category = "Stream|Chat")
    void BanUser(EStreamPlatform Platform, const FString& UserId, const FString& Reason);

    // ========================================================================
    // Commands
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Commands")
    void RegisterCommand(const FChatCommand& Command);

    UFUNCTION(BlueprintCallable, Category = "Stream|Commands")
    void UnregisterCommand(const FString& CommandName);

    UFUNCTION(BlueprintPure, Category = "Stream|Commands")
    TArray<FChatCommand> GetRegisteredCommands() const { return RegisteredCommands; }

    UFUNCTION(BlueprintCallable, Category = "Stream|Commands")
    void SetCommandEnabled(const FString& CommandName, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Stream|Commands")
    bool ProcessChatCommand(const FChatMessage& Message);

    // ========================================================================
    // Game Effects
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Effects")
    void RegisterGameEffect(const FGameEffectConfig& Effect);

    UFUNCTION(BlueprintCallable, Category = "Stream|Effects")
    void UnregisterGameEffect(const FString& EffectId);

    UFUNCTION(BlueprintPure, Category = "Stream|Effects")
    TArray<FGameEffectConfig> GetAvailableEffects() const { return AvailableEffects; }

    UFUNCTION(BlueprintCallable, Category = "Stream|Effects")
    bool TriggerEffect(const FString& EffectId, const FViewerInfo& Viewer);

    UFUNCTION(BlueprintCallable, Category = "Stream|Effects")
    void SetEffectEnabled(const FString& EffectId, bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Stream|Effects")
    bool IsEffectOnCooldown(const FString& EffectId) const;

    UFUNCTION(BlueprintPure, Category = "Stream|Effects")
    float GetEffectCooldownRemaining(const FString& EffectId) const;

    // ========================================================================
    // Polls
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Polls")
    void CreatePoll(const FString& Title, const TArray<FString>& Options, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Stream|Polls")
    void EndPoll(const FGuid& PollId);

    UFUNCTION(BlueprintCallable, Category = "Stream|Polls")
    void CancelPoll(const FGuid& PollId);

    UFUNCTION(BlueprintPure, Category = "Stream|Polls")
    FStreamPoll GetActivePoll() const;

    UFUNCTION(BlueprintPure, Category = "Stream|Polls")
    bool HasActivePoll() const;

    UFUNCTION(BlueprintCallable, Category = "Stream|Polls")
    void Vote(const FGuid& PollId, const FString& Option, const FViewerInfo& Voter);

    // ========================================================================
    // Predictions
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Predictions")
    void CreatePrediction(const FString& Title, const TArray<FString>& Outcomes, float WindowSeconds);

    UFUNCTION(BlueprintCallable, Category = "Stream|Predictions")
    void LockPrediction(const FGuid& PredictionId);

    UFUNCTION(BlueprintCallable, Category = "Stream|Predictions")
    void ResolvePrediction(const FGuid& PredictionId, const FString& WinningOutcome);

    UFUNCTION(BlueprintCallable, Category = "Stream|Predictions")
    void CancelPrediction(const FGuid& PredictionId);

    UFUNCTION(BlueprintPure, Category = "Stream|Predictions")
    FStreamPrediction GetActivePrediction() const;

    UFUNCTION(BlueprintCallable, Category = "Stream|Predictions")
    void PlacePrediction(const FGuid& PredictionId, const FString& Outcome, int32 Points, const FViewerInfo& Viewer);

    // ========================================================================
    // Alerts
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "Stream|Alerts")
    void QueueAlert(const FStreamAlert& Alert);

    UFUNCTION(BlueprintPure, Category = "Stream|Alerts")
    TArray<FStreamAlert> GetPendingAlerts() const { return PendingAlerts; }

    UFUNCTION(BlueprintCallable, Category = "Stream|Alerts")
    FStreamAlert GetNextAlert();

    UFUNCTION(BlueprintCallable, Category = "Stream|Alerts")
    void MarkAlertDisplayed(const FGuid& AlertId);

    UFUNCTION(BlueprintCallable, Category = "Stream|Alerts")
    void ClearAlerts();

    // ========================================================================
    // Settings
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Stream|Settings")
    FStreamIntegrationSettings GetSettings() const { return Settings; }

    UFUNCTION(BlueprintCallable, Category = "Stream|Settings")
    void UpdateSettings(const FStreamIntegrationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Stream|Settings")
    void SetViewerInteractionsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Stream|Settings")
    void SetSubOnlyMode(bool bEnabled);

    // ========================================================================
    // Statistics
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "Stream|Stats")
    FStreamIntegrationStats GetStats() const { return Stats; }

    UFUNCTION(BlueprintCallable, Category = "Stream|Stats")
    void ResetStats();

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnStreamConnected OnStreamConnected;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnStreamDisconnected OnStreamDisconnected;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnChatMessageReceived OnChatMessageReceived;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnViewerInteraction OnViewerInteraction;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnGameEffectTriggered OnGameEffectTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnPollStarted OnPollStarted;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnPollEnded OnPollEnded;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnPredictionStarted OnPredictionStarted;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnPredictionResolved OnPredictionResolved;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnStreamAlertReceived OnStreamAlertReceived;

    UPROPERTY(BlueprintAssignable, Category = "Stream|Events")
    FOnViewerCountChanged OnViewerCountChanged;

protected:
    void InitializeDefaultEffects();
    void InitializeDefaultCommands();
    void ProcessInteractionQueue();
    void UpdateCooldowns(float DeltaTime);
    bool CheckCooldown(const FString& EffectId, const FString& ViewerId);
    void StartCooldown(const FString& EffectId, const FString& ViewerId, float Duration);

private:
    UPROPERTY()
    TMap<EStreamPlatform, FStreamCredentials> Credentials;

    UPROPERTY()
    TMap<EStreamPlatform, EStreamStatus> ConnectionStatus;

    UPROPERTY()
    TMap<EStreamPlatform, FStreamInfo> StreamInfos;

    UPROPERTY()
    FStreamIntegrationSettings Settings;

    UPROPERTY()
    FStreamIntegrationStats Stats;

    UPROPERTY()
    TArray<FChatCommand> RegisteredCommands;

    UPROPERTY()
    TArray<FGameEffectConfig> AvailableEffects;

    UPROPERTY()
    TArray<FChatMessage> ChatHistory;

    UPROPERTY()
    TArray<FViewerInteraction> InteractionQueue;

    UPROPERTY()
    TArray<FStreamAlert> PendingAlerts;

    UPROPERTY()
    FStreamPoll ActivePoll;

    UPROPERTY()
    FStreamPrediction ActivePrediction;

    UPROPERTY()
    TMap<FString, FDateTime> GlobalCooldowns;

    UPROPERTY()
    TMap<FString, TMap<FString, FDateTime>> UserCooldowns;

    UPROPERTY()
    int32 EffectsThisMinute;

    UPROPERTY()
    FDateTime MinuteStartTime;

    FTimerHandle InteractionProcessTimer;
    FTimerHandle CooldownUpdateTimer;
    FTimerHandle ViewerCountUpdateTimer;
};

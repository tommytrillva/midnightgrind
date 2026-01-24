// MidnightGrind - Arcade Street Racing Game
// Cross-Play Subsystem - Cross-platform play management

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrossPlaySubsystem.generated.h"

// Forward declarations
class UMGCrossPlaySubsystem;

/**
 * EMGCrossPlayPlatform - Platform identifiers for cross-play
 */
UENUM(BlueprintType)
enum class EMGCrossPlayPlatform : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    PC              UMETA(DisplayName = "PC"),
    PlayStation     UMETA(DisplayName = "PlayStation"),
    Xbox            UMETA(DisplayName = "Xbox"),
    Nintendo        UMETA(DisplayName = "Nintendo"),
    Mobile          UMETA(DisplayName = "Mobile")
};

/**
 * EMGInputType - Input device types for cross-play balancing
 */
UENUM(BlueprintType)
enum class EMGInputType : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    KeyboardMouse   UMETA(DisplayName = "Keyboard & Mouse"),
    Controller      UMETA(DisplayName = "Controller"),
    Touch           UMETA(DisplayName = "Touch"),
    Wheel           UMETA(DisplayName = "Racing Wheel")
};

/**
 * EMGCrossPlayStatus - Cross-play connection status
 */
UENUM(BlueprintType)
enum class EMGCrossPlayStatus : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Enabled         UMETA(DisplayName = "Enabled"),
    PlatformOnly    UMETA(DisplayName = "Platform Only"),
    InputBased      UMETA(DisplayName = "Input Based")
};

/**
 * EMGCrossPlayPooling - Matchmaking pooling options
 */
UENUM(BlueprintType)
enum class EMGCrossPlayPooling : uint8
{
    AllPlatforms    UMETA(DisplayName = "All Platforms"),
    ConsoleOnly     UMETA(DisplayName = "Console Only"),
    PCOnly          UMETA(DisplayName = "PC Only"),
    SameFamily      UMETA(DisplayName = "Same Family"),
    InputMatched    UMETA(DisplayName = "Input Matched")
};

/**
 * FMGCrossPlayPlayer - Cross-play player information
 */
USTRUCT(BlueprintType)
struct FMGCrossPlayPlayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCrossPlayPlatform Platform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGInputType InputType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlatformDisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasCrossPlayEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Latency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Region;

    FMGCrossPlayPlayer()
        : Platform(EMGCrossPlayPlatform::Unknown)
        , InputType(EMGInputType::Unknown)
        , bHasCrossPlayEnabled(true)
        , SkillRating(1000)
        , Latency(0.0f)
    {}
};

/**
 * FMGCrossPlaySettings - Cross-play preference settings
 */
USTRUCT(BlueprintType)
struct FMGCrossPlaySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCrossPlayStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCrossPlayPooling Pooling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowPCPlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowConsolePlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowMobilePlayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPreferSameInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowPlatformIcons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowCrossPlatformVoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowCrossPlatformFriends;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLatencyDifference;

    FMGCrossPlaySettings()
        : Status(EMGCrossPlayStatus::Enabled)
        , Pooling(EMGCrossPlayPooling::AllPlatforms)
        , bAllowPCPlayers(true)
        , bAllowConsolePlayers(true)
        , bAllowMobilePlayers(true)
        , bPreferSameInput(false)
        , bShowPlatformIcons(true)
        , bAllowCrossPlatformVoice(true)
        , bAllowCrossPlatformFriends(true)
        , MaxLatencyDifference(150)
    {}
};

/**
 * FMGPlatformStats - Statistics by platform
 */
USTRUCT(BlueprintType)
struct FMGPlatformStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGCrossPlayPlatform Platform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OnlinePlayerCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MatchesPlayed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLatency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WinRate;

    FMGPlatformStats()
        : Platform(EMGCrossPlayPlatform::Unknown)
        , OnlinePlayerCount(0)
        , MatchesPlayed(0)
        , AverageLatency(0.0f)
        , WinRate(0.5f)
    {}
};

/**
 * FMGCrossPlaySession - Cross-play session information
 */
USTRUCT(BlueprintType)
struct FMGCrossPlaySession
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SessionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGCrossPlayPlayer> Players;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGCrossPlayPlatform, int32> PlatformCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGInputType, int32> InputCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCrossPlaySession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLatency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxLatency;

    FMGCrossPlaySession()
        : bIsCrossPlaySession(false)
        , AverageLatency(0.0f)
        , MaxLatency(0)
    {}

    int32 GetPlatformCount(EMGCrossPlayPlatform Platform) const
    {
        const int32* Count = PlatformCounts.Find(Platform);
        return Count ? *Count : 0;
    }

    int32 GetInputCount(EMGInputType Input) const
    {
        const int32* Count = InputCounts.Find(Input);
        return Count ? *Count : 0;
    }
};

/**
 * FMGCrossPlayReport - Report for cross-play analytics
 */
USTRUCT(BlueprintType)
struct FMGCrossPlayReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCrossPlayMatches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSamePlatformMatches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGPlatformStats> PlatformBreakdown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CrossPlayAdoptionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageQueueTimeWithCrossPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageQueueTimeWithoutCrossPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ReportGeneratedAt;

    FMGCrossPlayReport()
        : TotalCrossPlayMatches(0)
        , TotalSamePlatformMatches(0)
        , CrossPlayAdoptionRate(0.0f)
        , AverageQueueTimeWithCrossPlay(0.0f)
        , AverageQueueTimeWithoutCrossPlay(0.0f)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrossPlayStatusChanged, EMGCrossPlayStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrossPlaySettingsChanged, const FMGCrossPlaySettings&, NewSettings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrossPlaySessionJoined, const FMGCrossPlaySession&, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrossPlayPlayerJoined, const FMGCrossPlayPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrossPlayPlayerLeft, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnInputTypeChanged, const FString&, PlayerId, EMGInputType, NewInput);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrossPlayError, const FString&, ErrorMessage);

/**
 * UMGCrossPlaySubsystem
 *
 * Manages cross-platform play for Midnight Grind.
 * Features include:
 * - Cross-play enable/disable
 * - Platform filtering preferences
 * - Input-based matchmaking
 * - Cross-platform friends
 * - Platform statistics
 * - Session management
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCrossPlaySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGCrossPlaySubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ===== Cross-Play Settings =====

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Settings")
    void SetCrossPlayEnabled(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Settings")
    bool IsCrossPlayEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Settings")
    void SetCrossPlaySettings(const FMGCrossPlaySettings& Settings);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Settings")
    FMGCrossPlaySettings GetCrossPlaySettings() const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Settings")
    void SetPoolingPreference(EMGCrossPlayPooling Pooling);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Settings")
    EMGCrossPlayPooling GetPoolingPreference() const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Settings")
    void SetPlatformAllowed(EMGCrossPlayPlatform Platform, bool bAllowed);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Settings")
    bool IsPlatformAllowed(EMGCrossPlayPlatform Platform) const;

    // ===== Platform Info =====

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Platform")
    EMGCrossPlayPlatform GetLocalPlatform() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Platform")
    EMGInputType GetLocalInputType() const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Platform")
    void SetLocalInputType(EMGInputType InputType);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Platform")
    FString GetPlatformDisplayName(EMGCrossPlayPlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Platform")
    FString GetInputTypeDisplayName(EMGInputType InputType) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Platform")
    TSoftObjectPtr<UTexture2D> GetPlatformIcon(EMGCrossPlayPlatform Platform) const;

    // ===== Session Info =====

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Session")
    FMGCrossPlaySession GetCurrentSession() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Session")
    bool IsInCrossPlaySession() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Session")
    TArray<FMGCrossPlayPlayer> GetSessionPlayers() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Session")
    FMGCrossPlayPlayer GetPlayer(const FString& PlayerId) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Session")
    int32 GetPlatformPlayerCount(EMGCrossPlayPlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Session")
    int32 GetInputTypePlayerCount(EMGInputType InputType) const;

    // ===== Matchmaking Support =====

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Matchmaking")
    TArray<EMGCrossPlayPlatform> GetAllowedPlatforms() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Matchmaking")
    bool CanMatchWith(const FMGCrossPlayPlayer& Player) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Matchmaking")
    bool CanMatchWithPlatform(EMGCrossPlayPlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Matchmaking")
    bool CanMatchWithInput(EMGInputType InputType) const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Matchmaking")
    void SetPreferSameInput(bool bPrefer);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Matchmaking")
    bool GetPreferSameInput() const;

    // ===== Statistics =====

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Stats")
    FMGPlatformStats GetPlatformStats(EMGCrossPlayPlatform Platform) const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Stats")
    TArray<FMGPlatformStats> GetAllPlatformStats() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Stats")
    FMGCrossPlayReport GetCrossPlayReport() const;

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Stats")
    float GetCrossPlayAdoptionRate() const;

    // ===== Friends =====

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Friends")
    bool CanAddCrossPlatformFriend() const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Friends")
    void SetAllowCrossPlatformFriends(bool bAllow);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Friends")
    bool GetAllowCrossPlatformFriends() const;

    // ===== Voice Chat =====

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Voice")
    bool CanUseCrossPlatformVoice() const;

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Voice")
    void SetAllowCrossPlatformVoice(bool bAllow);

    UFUNCTION(BlueprintPure, Category = "CrossPlay|Voice")
    bool GetAllowCrossPlatformVoice() const;

    // ===== Session Management (Internal) =====

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Internal")
    void RegisterPlayer(const FMGCrossPlayPlayer& Player);

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Internal")
    void UnregisterPlayer(const FString& PlayerId);

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Internal")
    void UpdatePlayerInput(const FString& PlayerId, EMGInputType NewInput);

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Internal")
    void ClearSession();

    // ===== Persistence =====

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Persistence")
    void SaveCrossPlaySettings();

    UFUNCTION(BlueprintCallable, Category = "CrossPlay|Persistence")
    void LoadCrossPlaySettings();

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnCrossPlayStatusChanged OnCrossPlayStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnCrossPlaySettingsChanged OnCrossPlaySettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnCrossPlaySessionJoined OnCrossPlaySessionJoined;

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnCrossPlayPlayerJoined OnCrossPlayPlayerJoined;

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnCrossPlayPlayerLeft OnCrossPlayPlayerLeft;

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnInputTypeChanged OnInputTypeChanged;

    UPROPERTY(BlueprintAssignable, Category = "CrossPlay|Events")
    FMGOnCrossPlayError OnCrossPlayError;

protected:
    void DetectLocalPlatform();
    void DetectLocalInputType();
    void UpdateSessionStats();

private:
    // Cross-play settings
    UPROPERTY()
    FMGCrossPlaySettings Settings;

    // Local player info
    UPROPERTY()
    EMGCrossPlayPlatform LocalPlatform;

    UPROPERTY()
    EMGInputType LocalInputType;

    // Current session
    UPROPERTY()
    FMGCrossPlaySession CurrentSession;

    // Platform stats cache
    UPROPERTY()
    TMap<EMGCrossPlayPlatform, FMGPlatformStats> PlatformStatsCache;

    // Report cache
    UPROPERTY()
    FMGCrossPlayReport CachedReport;

    // Settings dirty flag
    bool bSettingsDirty;
};

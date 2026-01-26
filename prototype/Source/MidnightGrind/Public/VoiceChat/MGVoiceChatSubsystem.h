// MidnightGrind - Arcade Street Racing Game
// Voice Chat Subsystem - Voice communication for multiplayer

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGVoiceChatSubsystem.generated.h"

// Forward declarations
class UMGVoiceChatSubsystem;

/**
 * EMGVoiceChatMode - Voice chat transmission modes
 */
UENUM(BlueprintType)
enum class EMGVoiceChatMode : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    PushToTalk      UMETA(DisplayName = "Push to Talk"),
    OpenMic         UMETA(DisplayName = "Open Mic"),
    VoiceActivated  UMETA(DisplayName = "Voice Activated")
};

/**
 * EMGVoiceChannel - Voice chat channels
 */
UENUM(BlueprintType)
enum class EMGVoiceChannel : uint8
{
    None            UMETA(DisplayName = "None"),
    Global          UMETA(DisplayName = "Global"),
    Team            UMETA(DisplayName = "Team"),
    Party           UMETA(DisplayName = "Party"),
    Proximity       UMETA(DisplayName = "Proximity"),
    Private         UMETA(DisplayName = "Private"),
    Spectator       UMETA(DisplayName = "Spectator")
};

/**
 * EMGVoiceQuality - Voice quality settings
 */
UENUM(BlueprintType)
enum class EMGVoiceQuality : uint8
{
    Low             UMETA(DisplayName = "Low"),
    Medium          UMETA(DisplayName = "Medium"),
    High            UMETA(DisplayName = "High"),
    Ultra           UMETA(DisplayName = "Ultra")
};

/**
 * EMGMuteReason - Reasons for muting a player
 */
UENUM(BlueprintType)
enum class EMGMuteReason : uint8
{
    Manual          UMETA(DisplayName = "Manual"),
    Spam            UMETA(DisplayName = "Spam"),
    Toxic           UMETA(DisplayName = "Toxic"),
    Background      UMETA(DisplayName = "Background Noise"),
    AutoMuted       UMETA(DisplayName = "Auto Muted"),
    Reported        UMETA(DisplayName = "Reported")
};

/**
 * EMGSpeakerState - Visual state of a speaker
 */
UENUM(BlueprintType)
enum class EMGSpeakerState : uint8
{
    Silent          UMETA(DisplayName = "Silent"),
    Speaking        UMETA(DisplayName = "Speaking"),
    Muted           UMETA(DisplayName = "Muted"),
    Deafened        UMETA(DisplayName = "Deafened"),
    Connecting      UMETA(DisplayName = "Connecting"),
    Error           UMETA(DisplayName = "Error")
};

/**
 * FMGVoiceParticipant - A participant in voice chat
 */
USTRUCT(BlueprintType)
struct FMGVoiceParticipant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGVoiceChannel CurrentChannel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGSpeakerState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSpeaking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMuted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMutedByMe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDeafened;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpeakingLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceFromPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WorldPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TeamId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime JoinedAt;

    FMGVoiceParticipant()
        : CurrentChannel(EMGVoiceChannel::None)
        , State(EMGSpeakerState::Silent)
        , bIsSpeaking(false)
        , bIsMuted(false)
        , bIsMutedByMe(false)
        , bIsDeafened(false)
        , Volume(1.0f)
        , SpeakingLevel(0.0f)
        , DistanceFromPlayer(0.0f)
        , WorldPosition(FVector::ZeroVector)
        , TeamId(-1)
    {}
};

/**
 * FMGVoiceSettings - Voice chat settings
 */
USTRUCT(BlueprintType)
struct FMGVoiceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGVoiceChatMode Mode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGVoiceQuality Quality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MicrophoneVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OutputVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VoiceActivationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNoiseSuppression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEchoCancellation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoGainControl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSpatialAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProximityFalloffStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMuteWhenDeafened;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShowSpeakingIndicators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PreferredInputDevice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PreferredOutputDevice;

    FMGVoiceSettings()
        : Mode(EMGVoiceChatMode::PushToTalk)
        , Quality(EMGVoiceQuality::High)
        , MicrophoneVolume(1.0f)
        , OutputVolume(1.0f)
        , VoiceActivationThreshold(0.02f)
        , bNoiseSuppression(true)
        , bEchoCancellation(true)
        , bAutoGainControl(true)
        , bSpatialAudio(true)
        , ProximityRadius(5000.0f)
        , ProximityFalloffStart(2500.0f)
        , bMuteWhenDeafened(true)
        , bShowSpeakingIndicators(true)
        , PreferredInputDevice(NAME_None)
        , PreferredOutputDevice(NAME_None)
    {}
};

/**
 * FMGVoiceChannelInfo - Information about a voice channel
 */
USTRUCT(BlueprintType)
struct FMGVoiceChannelInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGVoiceChannel Channel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ParticipantCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxParticipants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsJoined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresPermission;

    FMGVoiceChannelInfo()
        : Channel(EMGVoiceChannel::None)
        , ParticipantCount(0)
        , MaxParticipants(100)
        , bIsJoined(false)
        , bIsLocked(false)
        , bRequiresPermission(false)
    {}
};

/**
 * FMGMuteEntry - Entry for a muted player
 */
USTRUCT(BlueprintType)
struct FMGMuteEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMuteReason Reason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime MutedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ExpiresAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPermanent;

    FMGMuteEntry()
        : Reason(EMGMuteReason::Manual)
        , bIsPermanent(false)
    {}

    bool IsExpired() const
    {
        return !bIsPermanent && FDateTime::Now() > ExpiresAt;
    }
};

/**
 * FMGAudioDevice - Audio device information
 */
USTRUCT(BlueprintType)
struct FMGAudioDevice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName DeviceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsDefault;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;

    FMGAudioDevice()
        : DeviceId(NAME_None)
        , bIsInput(false)
        , bIsDefault(false)
        , bIsActive(false)
    {}
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnVoiceChannelJoined, EMGVoiceChannel, Channel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnVoiceChannelLeft, EMGVoiceChannel, Channel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnParticipantJoined, EMGVoiceChannel, Channel, const FMGVoiceParticipant&, Participant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnParticipantLeft, EMGVoiceChannel, Channel, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnParticipantSpeakingChanged, const FString&, PlayerId, bool, bIsSpeaking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnParticipantMuteChanged, const FString&, PlayerId, bool, bIsMuted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLocalMuteChanged, bool, bIsMuted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnLocalDeafenChanged, bool, bIsDeafened);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnVoiceSettingsChanged, const FMGVoiceSettings&, Settings);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnVoiceError, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPushToTalkStateChanged, bool, bIsTransmitting);

/**
 * UMGVoiceChatSubsystem
 *
 * Manages voice communication for Midnight Grind multiplayer.
 * Features include:
 * - Multiple voice channels (team, party, proximity)
 * - Push-to-talk and voice activation
 * - Spatial audio support
 * - Mute/unmute controls
 * - Audio device management
 * - Voice quality settings
 * - Noise suppression and echo cancellation
 */
UCLASS()
class MIDNIGHTGRIND_API UMGVoiceChatSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGVoiceChatSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tick for voice processing
    void TickVoiceChat(float DeltaTime);

    // ===== Channel Management =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Channel")
    bool JoinChannel(EMGVoiceChannel Channel);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Channel")
    void LeaveChannel(EMGVoiceChannel Channel);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Channel")
    void LeaveAllChannels();

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Channel")
    bool IsInChannel(EMGVoiceChannel Channel) const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Channel")
    EMGVoiceChannel GetActiveChannel() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Channel")
    TArray<EMGVoiceChannel> GetJoinedChannels() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Channel")
    FMGVoiceChannelInfo GetChannelInfo(EMGVoiceChannel Channel) const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Channel")
    void SetActiveChannel(EMGVoiceChannel Channel);

    // ===== Transmission Control =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Transmit")
    void StartTransmitting();

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Transmit")
    void StopTransmitting();

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Transmit")
    bool IsTransmitting() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Transmit")
    float GetLocalSpeakingLevel() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Transmit")
    void SetPushToTalkPressed(bool bPressed);

    // ===== Mute Controls =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void MuteLocalMicrophone(bool bMute);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void ToggleLocalMute();

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Mute")
    bool IsLocalMuted() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void DeafenLocal(bool bDeafen);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void ToggleLocalDeafen();

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Mute")
    bool IsLocalDeafened() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void MuteParticipant(const FString& PlayerId, bool bMute, EMGMuteReason Reason = EMGMuteReason::Manual);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void UnmuteParticipant(const FString& PlayerId);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Mute")
    bool IsParticipantMuted(const FString& PlayerId) const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void MuteAllParticipants(bool bMute);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Mute")
    TArray<FMGMuteEntry> GetMuteList() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Mute")
    void ClearMuteList();

    // ===== Participant Management =====

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Participants")
    TArray<FMGVoiceParticipant> GetParticipants(EMGVoiceChannel Channel) const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Participants")
    TArray<FMGVoiceParticipant> GetAllParticipants() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Participants")
    FMGVoiceParticipant GetParticipant(const FString& PlayerId) const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Participants")
    TArray<FMGVoiceParticipant> GetSpeakingParticipants() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Participants")
    void SetParticipantVolume(const FString& PlayerId, float Volume);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Participants")
    float GetParticipantVolume(const FString& PlayerId) const;

    // ===== Volume Control =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Volume")
    void SetMicrophoneVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Volume")
    float GetMicrophoneVolume() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Volume")
    void SetOutputVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Volume")
    float GetOutputVolume() const;

    // ===== Settings =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetVoiceSettings(const FMGVoiceSettings& Settings);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Settings")
    FMGVoiceSettings GetVoiceSettings() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetVoiceChatMode(EMGVoiceChatMode Mode);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Settings")
    EMGVoiceChatMode GetVoiceChatMode() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetVoiceQuality(EMGVoiceQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetVoiceActivationThreshold(float Threshold);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetNoiseSuppression(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetEchoCancellation(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Settings")
    void SetSpatialAudio(bool bEnabled);

    // ===== Audio Devices =====

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Devices")
    TArray<FMGAudioDevice> GetInputDevices() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Devices")
    TArray<FMGAudioDevice> GetOutputDevices() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Devices")
    void SetInputDevice(FName DeviceId);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Devices")
    void SetOutputDevice(FName DeviceId);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Devices")
    FMGAudioDevice GetCurrentInputDevice() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Devices")
    FMGAudioDevice GetCurrentOutputDevice() const;

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Devices")
    void RefreshAudioDevices();

    // ===== Proximity Voice =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Proximity")
    void UpdateLocalPosition(FVector Position);

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Proximity")
    void SetProximityRadius(float Radius);

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Proximity")
    float GetProximityRadius() const;

    // ===== Testing =====

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Test")
    void StartMicrophoneTest();

    UFUNCTION(BlueprintCallable, Category = "VoiceChat|Test")
    void StopMicrophoneTest();

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Test")
    bool IsMicrophoneTesting() const;

    UFUNCTION(BlueprintPure, Category = "VoiceChat|Test")
    float GetMicrophoneTestLevel() const;

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnVoiceChannelJoined OnVoiceChannelJoined;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnVoiceChannelLeft OnVoiceChannelLeft;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnParticipantJoined OnParticipantJoined;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnParticipantLeft OnParticipantLeft;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnParticipantSpeakingChanged OnParticipantSpeakingChanged;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnParticipantMuteChanged OnParticipantMuteChanged;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnLocalMuteChanged OnLocalMuteChanged;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnLocalDeafenChanged OnLocalDeafenChanged;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnVoiceSettingsChanged OnVoiceSettingsChanged;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnVoiceError OnVoiceError;

    UPROPERTY(BlueprintAssignable, Category = "VoiceChat|Events")
    FMGOnPushToTalkStateChanged OnPushToTalkStateChanged;

protected:
    void UpdateProximityVolumes();
    void ProcessVoiceActivation(float DeltaTime);
    void UpdateParticipantStates();
    float CalculateProximityVolume(float Distance) const;

private:
    // Voice settings
    UPROPERTY()
    FMGVoiceSettings VoiceSettings;

    // Joined channels
    UPROPERTY()
    TArray<EMGVoiceChannel> JoinedChannels;

    // Active transmission channel
    UPROPERTY()
    EMGVoiceChannel ActiveChannel;

    // Participants by channel
    UPROPERTY()
    TMap<FString, FMGVoiceParticipant> Participants;

    // Mute list
    UPROPERTY()
    TArray<FMGMuteEntry> MuteList;

    // Audio devices
    UPROPERTY()
    TArray<FMGAudioDevice> InputDevices;

    UPROPERTY()
    TArray<FMGAudioDevice> OutputDevices;

    // Local state
    bool bLocalMuted;
    bool bLocalDeafened;
    bool bTransmitting;
    bool bPushToTalkPressed;
    float LocalSpeakingLevel;
    FVector LocalPosition;

    // Testing
    bool bMicrophoneTesting;
    float MicrophoneTestLevel;

    // Timer handle
    FTimerHandle TickTimerHandle;
};

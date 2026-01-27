// MidnightGrind - Arcade Street Racing Game
// Voice Chat Subsystem Implementation

#include "VoiceChat/MGVoiceChatSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMGVoiceChatSubsystem::UMGVoiceChatSubsystem()
    : ActiveChannel(EMGVoiceChannel::None)
    , bLocalMuted(false)
    , bLocalDeafened(false)
    , bTransmitting(false)
    , bPushToTalkPressed(false)
    , LocalSpeakingLevel(0.0f)
    , LocalPosition(FVector::ZeroVector)
    , bMicrophoneTesting(false)
    , MicrophoneTestLevel(0.0f)
{
}

void UMGVoiceChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Initialize with default settings
    VoiceSettings = FMGVoiceSettings();

    // Refresh audio devices
    RefreshAudioDevices();

    // Start tick timer
    if (UWorld* World = GetWorld())
    {
        TWeakObjectPtr<UMGVoiceChatSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [WeakThis]() { if (WeakThis.IsValid()) WeakThis->TickVoiceChat(0.033f); },
            0.033f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("MGVoiceChatSubsystem initialized"));
}

void UMGVoiceChatSubsystem::Deinitialize()
{
    // Leave all channels
    LeaveAllChannels();

    // Stop mic test if running
    if (bMicrophoneTesting)
    {
        StopMicrophoneTest();
    }

    // Clear timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGVoiceChatSubsystem::TickVoiceChat(float DeltaTime)
{
    // Process voice activation
    ProcessVoiceActivation(DeltaTime);

    // Update proximity volumes if spatial audio is enabled
    if (VoiceSettings.bSpatialAudio && JoinedChannels.Contains(EMGVoiceChannel::Proximity))
    {
        UpdateProximityVolumes();
    }

    // Update participant states
    UpdateParticipantStates();

    // Clean expired mutes
    for (int32 i = MuteList.Num() - 1; i >= 0; i--)
    {
        if (MuteList[i].IsExpired())
        {
            MuteList.RemoveAt(i);
        }
    }
}

// ===== Channel Management =====

bool UMGVoiceChatSubsystem::JoinChannel(EMGVoiceChannel Channel)
{
    if (Channel == EMGVoiceChannel::None)
    {
        return false;
    }

    if (JoinedChannels.Contains(Channel))
    {
        return true; // Already joined
    }

    // In a real implementation, connect to voice service
    JoinedChannels.Add(Channel);

    // Set as active if first channel
    if (ActiveChannel == EMGVoiceChannel::None)
    {
        ActiveChannel = Channel;
    }

    OnVoiceChannelJoined.Broadcast(Channel);
    UE_LOG(LogTemp, Log, TEXT("Joined voice channel: %d"), static_cast<int32>(Channel));

    return true;
}

void UMGVoiceChatSubsystem::LeaveChannel(EMGVoiceChannel Channel)
{
    if (!JoinedChannels.Contains(Channel))
    {
        return;
    }

    // Remove participants from this channel
    TArray<FString> ToRemove;
    for (const auto& Pair : Participants)
    {
        if (Pair.Value.CurrentChannel == Channel)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (const FString& PlayerId : ToRemove)
    {
        Participants.Remove(PlayerId);
        OnParticipantLeft.Broadcast(Channel, PlayerId);
    }

    JoinedChannels.Remove(Channel);

    // Update active channel
    if (ActiveChannel == Channel)
    {
        ActiveChannel = JoinedChannels.Num() > 0 ? JoinedChannels[0] : EMGVoiceChannel::None;
    }

    OnVoiceChannelLeft.Broadcast(Channel);
    UE_LOG(LogTemp, Log, TEXT("Left voice channel: %d"), static_cast<int32>(Channel));
}

void UMGVoiceChatSubsystem::LeaveAllChannels()
{
    TArray<EMGVoiceChannel> ChannelsCopy = JoinedChannels;
    for (EMGVoiceChannel Channel : ChannelsCopy)
    {
        LeaveChannel(Channel);
    }
}

bool UMGVoiceChatSubsystem::IsInChannel(EMGVoiceChannel Channel) const
{
    return JoinedChannels.Contains(Channel);
}

EMGVoiceChannel UMGVoiceChatSubsystem::GetActiveChannel() const
{
    return ActiveChannel;
}

TArray<EMGVoiceChannel> UMGVoiceChatSubsystem::GetJoinedChannels() const
{
    return JoinedChannels;
}

FMGVoiceChannelInfo UMGVoiceChatSubsystem::GetChannelInfo(EMGVoiceChannel Channel) const
{
    FMGVoiceChannelInfo Info;
    Info.Channel = Channel;
    Info.bIsJoined = JoinedChannels.Contains(Channel);

    // Count participants in channel
    for (const auto& Pair : Participants)
    {
        if (Pair.Value.CurrentChannel == Channel)
        {
            Info.ParticipantCount++;
        }
    }

    switch (Channel)
    {
        case EMGVoiceChannel::Global:
            Info.DisplayName = FText::FromString(TEXT("Global"));
            Info.MaxParticipants = 100;
            break;
        case EMGVoiceChannel::Team:
            Info.DisplayName = FText::FromString(TEXT("Team"));
            Info.MaxParticipants = 16;
            break;
        case EMGVoiceChannel::Party:
            Info.DisplayName = FText::FromString(TEXT("Party"));
            Info.MaxParticipants = 8;
            break;
        case EMGVoiceChannel::Proximity:
            Info.DisplayName = FText::FromString(TEXT("Proximity"));
            Info.MaxParticipants = 50;
            break;
        case EMGVoiceChannel::Private:
            Info.DisplayName = FText::FromString(TEXT("Private"));
            Info.MaxParticipants = 2;
            break;
        case EMGVoiceChannel::Spectator:
            Info.DisplayName = FText::FromString(TEXT("Spectator"));
            Info.MaxParticipants = 100;
            break;
        default:
            break;
    }

    return Info;
}

void UMGVoiceChatSubsystem::SetActiveChannel(EMGVoiceChannel Channel)
{
    if (JoinedChannels.Contains(Channel) || Channel == EMGVoiceChannel::None)
    {
        ActiveChannel = Channel;
    }
}

// ===== Transmission Control =====

void UMGVoiceChatSubsystem::StartTransmitting()
{
    if (bLocalMuted || bLocalDeafened) return;
    if (VoiceSettings.Mode == EMGVoiceChatMode::Disabled) return;

    if (!bTransmitting)
    {
        bTransmitting = true;
        OnPushToTalkStateChanged.Broadcast(true);
        UE_LOG(LogTemp, Verbose, TEXT("Voice transmission started"));
    }
}

void UMGVoiceChatSubsystem::StopTransmitting()
{
    if (bTransmitting)
    {
        bTransmitting = false;
        OnPushToTalkStateChanged.Broadcast(false);
        UE_LOG(LogTemp, Verbose, TEXT("Voice transmission stopped"));
    }
}

bool UMGVoiceChatSubsystem::IsTransmitting() const
{
    return bTransmitting;
}

float UMGVoiceChatSubsystem::GetLocalSpeakingLevel() const
{
    return LocalSpeakingLevel;
}

void UMGVoiceChatSubsystem::SetPushToTalkPressed(bool bPressed)
{
    bPushToTalkPressed = bPressed;

    if (VoiceSettings.Mode == EMGVoiceChatMode::PushToTalk)
    {
        if (bPressed)
        {
            StartTransmitting();
        }
        else
        {
            StopTransmitting();
        }
    }
}

// ===== Mute Controls =====

void UMGVoiceChatSubsystem::MuteLocalMicrophone(bool bMute)
{
    if (bLocalMuted != bMute)
    {
        bLocalMuted = bMute;

        if (bMute && bTransmitting)
        {
            StopTransmitting();
        }

        OnLocalMuteChanged.Broadcast(bMute);
        UE_LOG(LogTemp, Log, TEXT("Local mute: %s"), bMute ? TEXT("On") : TEXT("Off"));
    }
}

void UMGVoiceChatSubsystem::ToggleLocalMute()
{
    MuteLocalMicrophone(!bLocalMuted);
}

bool UMGVoiceChatSubsystem::IsLocalMuted() const
{
    return bLocalMuted;
}

void UMGVoiceChatSubsystem::DeafenLocal(bool bDeafen)
{
    if (bLocalDeafened != bDeafen)
    {
        bLocalDeafened = bDeafen;

        if (bDeafen && VoiceSettings.bMuteWhenDeafened)
        {
            if (bTransmitting)
            {
                StopTransmitting();
            }
        }

        OnLocalDeafenChanged.Broadcast(bDeafen);
        UE_LOG(LogTemp, Log, TEXT("Local deafen: %s"), bDeafen ? TEXT("On") : TEXT("Off"));
    }
}

void UMGVoiceChatSubsystem::ToggleLocalDeafen()
{
    DeafenLocal(!bLocalDeafened);
}

bool UMGVoiceChatSubsystem::IsLocalDeafened() const
{
    return bLocalDeafened;
}

void UMGVoiceChatSubsystem::MuteParticipant(const FString& PlayerId, bool bMute, EMGMuteReason Reason)
{
    FMGVoiceParticipant* Participant = Participants.Find(PlayerId);
    if (Participant)
    {
        Participant->bIsMutedByMe = bMute;

        if (bMute)
        {
            // Add to mute list
            FMGMuteEntry Entry;
            Entry.PlayerId = PlayerId;
            Entry.DisplayName = Participant->DisplayName;
            Entry.Reason = Reason;
            Entry.MutedAt = FDateTime::Now();
            Entry.bIsPermanent = true;
            MuteList.Add(Entry);
        }
        else
        {
            // Remove from mute list
            MuteList.RemoveAll([&PlayerId](const FMGMuteEntry& Entry) {
                return Entry.PlayerId == PlayerId;
            });
        }

        OnParticipantMuteChanged.Broadcast(PlayerId, bMute);
    }
}

void UMGVoiceChatSubsystem::UnmuteParticipant(const FString& PlayerId)
{
    MuteParticipant(PlayerId, false);
}

bool UMGVoiceChatSubsystem::IsParticipantMuted(const FString& PlayerId) const
{
    const FMGVoiceParticipant* Participant = Participants.Find(PlayerId);
    if (Participant)
    {
        return Participant->bIsMuted || Participant->bIsMutedByMe;
    }

    // Check mute list for players not currently in session
    for (const FMGMuteEntry& Entry : MuteList)
    {
        if (Entry.PlayerId == PlayerId && !Entry.IsExpired())
        {
            return true;
        }
    }

    return false;
}

void UMGVoiceChatSubsystem::MuteAllParticipants(bool bMute)
{
    for (auto& Pair : Participants)
    {
        MuteParticipant(Pair.Key, bMute);
    }
}

TArray<FMGMuteEntry> UMGVoiceChatSubsystem::GetMuteList() const
{
    return MuteList;
}

void UMGVoiceChatSubsystem::ClearMuteList()
{
    for (const FMGMuteEntry& Entry : MuteList)
    {
        FMGVoiceParticipant* Participant = Participants.Find(Entry.PlayerId);
        if (Participant)
        {
            Participant->bIsMutedByMe = false;
            OnParticipantMuteChanged.Broadcast(Entry.PlayerId, false);
        }
    }
    MuteList.Empty();
}

// ===== Participant Management =====

TArray<FMGVoiceParticipant> UMGVoiceChatSubsystem::GetParticipants(EMGVoiceChannel Channel) const
{
    TArray<FMGVoiceParticipant> Result;
    for (const auto& Pair : Participants)
    {
        if (Pair.Value.CurrentChannel == Channel)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FMGVoiceParticipant> UMGVoiceChatSubsystem::GetAllParticipants() const
{
    TArray<FMGVoiceParticipant> Result;
    Participants.GenerateValueArray(Result);
    return Result;
}

FMGVoiceParticipant UMGVoiceChatSubsystem::GetParticipant(const FString& PlayerId) const
{
    const FMGVoiceParticipant* Participant = Participants.Find(PlayerId);
    return Participant ? *Participant : FMGVoiceParticipant();
}

TArray<FMGVoiceParticipant> UMGVoiceChatSubsystem::GetSpeakingParticipants() const
{
    TArray<FMGVoiceParticipant> Result;
    for (const auto& Pair : Participants)
    {
        if (Pair.Value.bIsSpeaking)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

void UMGVoiceChatSubsystem::SetParticipantVolume(const FString& PlayerId, float Volume)
{
    FMGVoiceParticipant* Participant = Participants.Find(PlayerId);
    if (Participant)
    {
        Participant->Volume = FMath::Clamp(Volume, 0.0f, 2.0f);
    }
}

float UMGVoiceChatSubsystem::GetParticipantVolume(const FString& PlayerId) const
{
    const FMGVoiceParticipant* Participant = Participants.Find(PlayerId);
    return Participant ? Participant->Volume : 1.0f;
}

// ===== Volume Control =====

void UMGVoiceChatSubsystem::SetMicrophoneVolume(float Volume)
{
    VoiceSettings.MicrophoneVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

float UMGVoiceChatSubsystem::GetMicrophoneVolume() const
{
    return VoiceSettings.MicrophoneVolume;
}

void UMGVoiceChatSubsystem::SetOutputVolume(float Volume)
{
    VoiceSettings.OutputVolume = FMath::Clamp(Volume, 0.0f, 2.0f);
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

float UMGVoiceChatSubsystem::GetOutputVolume() const
{
    return VoiceSettings.OutputVolume;
}

// ===== Settings =====

void UMGVoiceChatSubsystem::SetVoiceSettings(const FMGVoiceSettings& Settings)
{
    VoiceSettings = Settings;
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
    UE_LOG(LogTemp, Log, TEXT("Voice settings updated"));
}

FMGVoiceSettings UMGVoiceChatSubsystem::GetVoiceSettings() const
{
    return VoiceSettings;
}

void UMGVoiceChatSubsystem::SetVoiceChatMode(EMGVoiceChatMode Mode)
{
    VoiceSettings.Mode = Mode;

    // Stop transmitting if switching to disabled or PTT without key pressed
    if (Mode == EMGVoiceChatMode::Disabled)
    {
        StopTransmitting();
    }
    else if (Mode == EMGVoiceChatMode::PushToTalk && !bPushToTalkPressed)
    {
        StopTransmitting();
    }

    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

EMGVoiceChatMode UMGVoiceChatSubsystem::GetVoiceChatMode() const
{
    return VoiceSettings.Mode;
}

void UMGVoiceChatSubsystem::SetVoiceQuality(EMGVoiceQuality Quality)
{
    VoiceSettings.Quality = Quality;
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

void UMGVoiceChatSubsystem::SetVoiceActivationThreshold(float Threshold)
{
    VoiceSettings.VoiceActivationThreshold = FMath::Clamp(Threshold, 0.0f, 1.0f);
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

void UMGVoiceChatSubsystem::SetNoiseSuppression(bool bEnabled)
{
    VoiceSettings.bNoiseSuppression = bEnabled;
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

void UMGVoiceChatSubsystem::SetEchoCancellation(bool bEnabled)
{
    VoiceSettings.bEchoCancellation = bEnabled;
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

void UMGVoiceChatSubsystem::SetSpatialAudio(bool bEnabled)
{
    VoiceSettings.bSpatialAudio = bEnabled;
    OnVoiceSettingsChanged.Broadcast(VoiceSettings);
}

// ===== Audio Devices =====

TArray<FMGAudioDevice> UMGVoiceChatSubsystem::GetInputDevices() const
{
    return InputDevices;
}

TArray<FMGAudioDevice> UMGVoiceChatSubsystem::GetOutputDevices() const
{
    return OutputDevices;
}

void UMGVoiceChatSubsystem::SetInputDevice(FName DeviceId)
{
    VoiceSettings.PreferredInputDevice = DeviceId;
    // In a real implementation, switch the audio device
    UE_LOG(LogTemp, Log, TEXT("Input device set to: %s"), *DeviceId.ToString());
}

void UMGVoiceChatSubsystem::SetOutputDevice(FName DeviceId)
{
    VoiceSettings.PreferredOutputDevice = DeviceId;
    // In a real implementation, switch the audio device
    UE_LOG(LogTemp, Log, TEXT("Output device set to: %s"), *DeviceId.ToString());
}

FMGAudioDevice UMGVoiceChatSubsystem::GetCurrentInputDevice() const
{
    for (const FMGAudioDevice& Device : InputDevices)
    {
        if (Device.DeviceId == VoiceSettings.PreferredInputDevice || Device.bIsDefault)
        {
            return Device;
        }
    }
    return FMGAudioDevice();
}

FMGAudioDevice UMGVoiceChatSubsystem::GetCurrentOutputDevice() const
{
    for (const FMGAudioDevice& Device : OutputDevices)
    {
        if (Device.DeviceId == VoiceSettings.PreferredOutputDevice || Device.bIsDefault)
        {
            return Device;
        }
    }
    return FMGAudioDevice();
}

void UMGVoiceChatSubsystem::RefreshAudioDevices()
{
    // In a real implementation, query system for audio devices
    InputDevices.Empty();
    OutputDevices.Empty();

    // Add default devices
    FMGAudioDevice DefaultInput;
    DefaultInput.DeviceId = FName("Default");
    DefaultInput.DisplayName = FText::FromString(TEXT("Default Microphone"));
    DefaultInput.bIsInput = true;
    DefaultInput.bIsDefault = true;
    InputDevices.Add(DefaultInput);

    FMGAudioDevice DefaultOutput;
    DefaultOutput.DeviceId = FName("Default");
    DefaultOutput.DisplayName = FText::FromString(TEXT("Default Speakers"));
    DefaultOutput.bIsInput = false;
    DefaultOutput.bIsDefault = true;
    OutputDevices.Add(DefaultOutput);

    UE_LOG(LogTemp, Log, TEXT("Audio devices refreshed"));
}

// ===== Proximity Voice =====

void UMGVoiceChatSubsystem::UpdateLocalPosition(FVector Position)
{
    LocalPosition = Position;
}

void UMGVoiceChatSubsystem::SetProximityRadius(float Radius)
{
    VoiceSettings.ProximityRadius = FMath::Max(100.0f, Radius);
}

float UMGVoiceChatSubsystem::GetProximityRadius() const
{
    return VoiceSettings.ProximityRadius;
}

// ===== Testing =====

void UMGVoiceChatSubsystem::StartMicrophoneTest()
{
    bMicrophoneTesting = true;
    MicrophoneTestLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Microphone test started"));
}

void UMGVoiceChatSubsystem::StopMicrophoneTest()
{
    bMicrophoneTesting = false;
    MicrophoneTestLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("Microphone test stopped"));
}

bool UMGVoiceChatSubsystem::IsMicrophoneTesting() const
{
    return bMicrophoneTesting;
}

float UMGVoiceChatSubsystem::GetMicrophoneTestLevel() const
{
    return MicrophoneTestLevel;
}

// ===== Internal Helpers =====

void UMGVoiceChatSubsystem::UpdateProximityVolumes()
{
    for (auto& Pair : Participants)
    {
        if (Pair.Value.CurrentChannel == EMGVoiceChannel::Proximity)
        {
            float Distance = FVector::Dist(LocalPosition, Pair.Value.WorldPosition);
            Pair.Value.DistanceFromPlayer = Distance;

            // Calculate distance-based volume
            float ProximityVolume = CalculateProximityVolume(Distance);
            // Apply to participant (would be applied in audio processing)
        }
    }
}

void UMGVoiceChatSubsystem::ProcessVoiceActivation(float DeltaTime)
{
    if (VoiceSettings.Mode == EMGVoiceChatMode::VoiceActivated ||
        VoiceSettings.Mode == EMGVoiceChatMode::OpenMic)
    {
        // Simulate voice level detection (in real implementation, analyze mic input)
        // LocalSpeakingLevel would be set by audio engine

        if (VoiceSettings.Mode == EMGVoiceChatMode::VoiceActivated)
        {
            if (LocalSpeakingLevel > VoiceSettings.VoiceActivationThreshold)
            {
                StartTransmitting();
            }
            else
            {
                StopTransmitting();
            }
        }
        else if (VoiceSettings.Mode == EMGVoiceChatMode::OpenMic)
        {
            if (!bTransmitting && !bLocalMuted)
            {
                StartTransmitting();
            }
        }
    }

    // Update mic test level
    if (bMicrophoneTesting)
    {
        MicrophoneTestLevel = LocalSpeakingLevel;
    }
}

void UMGVoiceChatSubsystem::UpdateParticipantStates()
{
    for (auto& Pair : Participants)
    {
        FMGVoiceParticipant& Participant = Pair.Value;

        // Update state based on speaking/mute status
        if (Participant.bIsMuted || Participant.bIsMutedByMe)
        {
            Participant.State = EMGSpeakerState::Muted;
        }
        else if (Participant.bIsDeafened)
        {
            Participant.State = EMGSpeakerState::Deafened;
        }
        else if (Participant.bIsSpeaking)
        {
            Participant.State = EMGSpeakerState::Speaking;
        }
        else
        {
            Participant.State = EMGSpeakerState::Silent;
        }
    }
}

float UMGVoiceChatSubsystem::CalculateProximityVolume(float Distance) const
{
    if (Distance <= VoiceSettings.ProximityFalloffStart)
    {
        return 1.0f;
    }
    else if (Distance >= VoiceSettings.ProximityRadius)
    {
        return 0.0f;
    }
    else
    {
        float FalloffRange = VoiceSettings.ProximityRadius - VoiceSettings.ProximityFalloffStart;
        float FalloffDistance = Distance - VoiceSettings.ProximityFalloffStart;
        return 1.0f - (FalloffDistance / FalloffRange);
    }
}

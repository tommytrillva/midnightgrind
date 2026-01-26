// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStreamerModeSubsystem.generated.h"

/**
 * Streamer Mode System - For Content Creators
 * - Twitch/YouTube integration
 * - Viewer interaction features
 * - Stream-safe options (hide personal info)
 * - Chat voting on races
 * - Subscriber perks (race as ghost, etc.)
 * - Clip-friendly UI modes
 */

UENUM(BlueprintType)
enum class EMGStreamPlatform : uint8
{
	None,
	Twitch,
	YouTube,
	Kick,
	Custom
};

UENUM(BlueprintType)
enum class EMGViewerInteractionType : uint8
{
	VoteNextTrack,
	VoteWeather,
	TriggerEvent,
	SpawnObstacle,
	BoostPlayer,
	SlowPlayer,
	ChangeMusic,
	CustomMessage
};

USTRUCT(BlueprintType)
struct FMGStreamerSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStreamerModeEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHidePlayerNames = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideCrewName = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHideOnlineStatus = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDisableProfanityFilter = false; // For adults-only streams

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowViewerOverlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowViewerInteractions = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InteractionCooldownSeconds = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSubOnlyInteractions = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomStreamerName;
};

USTRUCT(BlueprintType)
struct FMGViewerPoll
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PollID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Question;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Options;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> Votes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DurationSeconds = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WinningOptionIndex = -1;
};

USTRUCT(BlueprintType)
struct FMGViewerInteraction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGViewerInteractionType Type = EMGViewerInteractionType::VoteNextTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ViewerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSubscriber = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CustomData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

USTRUCT(BlueprintType)
struct FMGStreamStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentViewers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalInteractions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalVotes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTimespan StreamDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacesCompleted = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnViewerInteraction, const FMGViewerInteraction&, Interaction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPollStarted, const FMGViewerPoll&, Poll);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPollEnded, const FMGViewerPoll&, Poll, int32, WinningIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStreamConnected, EMGStreamPlatform, Platform);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnStreamDisconnected);

UCLASS()
class MIDNIGHTGRIND_API UMGStreamerModeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Streamer Mode
	UFUNCTION(BlueprintCallable, Category = "Streamer")
	void EnableStreamerMode(const FMGStreamerSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Streamer")
	void DisableStreamerMode();

	UFUNCTION(BlueprintPure, Category = "Streamer")
	bool IsStreamerModeEnabled() const { return StreamerSettings.bStreamerModeEnabled; }

	UFUNCTION(BlueprintPure, Category = "Streamer")
	FMGStreamerSettings GetStreamerSettings() const { return StreamerSettings; }

	UFUNCTION(BlueprintCallable, Category = "Streamer")
	void UpdateStreamerSettings(const FMGStreamerSettings& Settings);

	// Platform Connection
	UFUNCTION(BlueprintCallable, Category = "Streamer|Platform")
	void ConnectToPlatform(EMGStreamPlatform Platform, const FString& AuthToken);

	UFUNCTION(BlueprintCallable, Category = "Streamer|Platform")
	void DisconnectFromPlatform();

	UFUNCTION(BlueprintPure, Category = "Streamer|Platform")
	bool IsConnectedToPlatform() const { return ConnectedPlatform != EMGStreamPlatform::None; }

	UFUNCTION(BlueprintPure, Category = "Streamer|Platform")
	EMGStreamPlatform GetConnectedPlatform() const { return ConnectedPlatform; }

	// Viewer Interactions
	UFUNCTION(BlueprintCallable, Category = "Streamer|Interaction")
	void ProcessViewerCommand(const FString& ViewerName, const FString& Command, bool bIsSubscriber);

	UFUNCTION(BlueprintCallable, Category = "Streamer|Interaction")
	void TriggerInteraction(const FMGViewerInteraction& Interaction);

	UFUNCTION(BlueprintPure, Category = "Streamer|Interaction")
	bool CanTriggerInteraction() const;

	UFUNCTION(BlueprintPure, Category = "Streamer|Interaction")
	float GetInteractionCooldownRemaining() const;

	// Polls
	UFUNCTION(BlueprintCallable, Category = "Streamer|Poll")
	void StartPoll(const FText& Question, const TArray<FText>& Options, int32 DurationSeconds = 30);

	UFUNCTION(BlueprintCallable, Category = "Streamer|Poll")
	void RegisterVote(const FString& ViewerName, int32 OptionIndex);

	UFUNCTION(BlueprintCallable, Category = "Streamer|Poll")
	void EndPollEarly();

	UFUNCTION(BlueprintPure, Category = "Streamer|Poll")
	FMGViewerPoll GetActivePoll() const { return ActivePoll; }

	UFUNCTION(BlueprintPure, Category = "Streamer|Poll")
	bool IsPollActive() const { return ActivePoll.bIsActive; }

	// Quick Actions
	UFUNCTION(BlueprintCallable, Category = "Streamer|Quick")
	void StartTrackVote(const TArray<FName>& TrackOptions);

	UFUNCTION(BlueprintCallable, Category = "Streamer|Quick")
	void StartWeatherVote();

	UFUNCTION(BlueprintCallable, Category = "Streamer|Quick")
	void AllowChaosMode(int32 DurationSeconds);

	// Stream Stats
	UFUNCTION(BlueprintPure, Category = "Streamer|Stats")
	FMGStreamStats GetStreamStats() const { return StreamStats; }

	UFUNCTION(BlueprintCallable, Category = "Streamer|Stats")
	void UpdateViewerCount(int32 Count);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Streamer|Events")
	FMGOnViewerInteraction OnViewerInteraction;

	UPROPERTY(BlueprintAssignable, Category = "Streamer|Events")
	FMGOnPollStarted OnPollStarted;

	UPROPERTY(BlueprintAssignable, Category = "Streamer|Events")
	FMGOnPollEnded OnPollEnded;

	UPROPERTY(BlueprintAssignable, Category = "Streamer|Events")
	FMGOnStreamConnected OnStreamConnected;

	UPROPERTY(BlueprintAssignable, Category = "Streamer|Events")
	FMGOnStreamDisconnected OnStreamDisconnected;

protected:
	void UpdatePoll(float DeltaTime);
	void FinishPoll();
	void ProcessChatCommand(const FString& ViewerName, const FString& Command, bool bIsSub);
	void LoadSettings();
	void SaveSettings();

private:
	FMGStreamerSettings StreamerSettings;
	EMGStreamPlatform ConnectedPlatform = EMGStreamPlatform::None;
	FMGViewerPoll ActivePoll;
	FMGStreamStats StreamStats;
	TSet<FString> VotedViewers; // Prevent double voting
	FDateTime LastInteractionTime;
	FDateTime StreamStartTime;
	FTimerHandle PollTimerHandle;
	bool bChaosModeActive = false;
};

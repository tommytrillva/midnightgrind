// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGStreamerModeSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * This file defines the Streamer Mode Subsystem for Midnight Grind. It provides
 * a streamlined interface for content creators (streamers) to manage privacy
 * settings, viewer interactions, and stream-friendly features. This is a
 * lighter-weight alternative to MGStreamIntegrationSubsystem, focused on
 * streamer convenience rather than deep platform integration.
 *
 * KEY CONCEPTS FOR ENTRY-LEVEL DEVELOPERS:
 *
 * 1. WHAT IS "STREAMER MODE"?
 *    - A special game mode designed for people broadcasting their gameplay.
 *    - Hides personal information (player names, crew tags, online status).
 *    - Enables viewer interaction features (polls, commands).
 *    - Optimizes UI for stream viewers (readable text, clean overlays).
 *
 * 2. WHY HIDE INFORMATION?
 *    - Streamers play with real usernames visible to thousands.
 *    - Other players might not want their names on stream.
 *    - Privacy protection prevents harassment (stream sniping, etc.).
 *    - bHidePlayerNames, bHideCrewName, bHideOnlineStatus control this.
 *
 * 3. STREAMER SETTINGS (FMGStreamerSettings):
 *    - bStreamerModeEnabled: Master toggle for all streamer features.
 *    - bShowViewerOverlay: Display stream-specific UI elements.
 *    - bAllowViewerInteractions: Let chat affect the game.
 *    - InteractionCooldownSeconds: Prevents spam (default 30s between effects).
 *    - bSubOnlyInteractions: Restrict interactions to subscribers.
 *    - CustomStreamerName: Override displayed name on stream.
 *
 * 4. VIEWER INTERACTIONS (EMGViewerInteractionType):
 *    - VoteNextTrack: Chat votes on which track to race next.
 *    - VoteWeather: Chat chooses weather conditions.
 *    - TriggerEvent: Spawn random events during race.
 *    - SpawnObstacle: Drop obstacles on the track.
 *    - BoostPlayer/SlowPlayer: Affect the streamer's speed.
 *    - ChangeMusic: Let chat pick the soundtrack.
 *    - CustomMessage: Display viewer messages on screen.
 *
 * 5. VIEWER POLLS (FMGViewerPoll):
 *    - Time-limited voting where chat chooses from options.
 *    - Each viewer can vote once (tracked in VotedViewers set).
 *    - WinningOptionIndex determined when poll ends.
 *    - Used for track selection, weather, and custom choices.
 *
 * 6. PLATFORM CONNECTION:
 *    - Simplified connection to Twitch, YouTube, Kick.
 *    - ConnectToPlatform() with auth token establishes link.
 *    - IsConnectedToPlatform() checks connection status.
 *    - Less complex than MGStreamIntegrationSubsystem (no channel points, etc.).
 *
 * 7. CHAT COMMANDS:
 *    - ProcessViewerCommand() handles incoming chat commands.
 *    - Commands typically start with "!" (e.g., "!vote 1").
 *    - bIsSubscriber flag enables sub-only commands.
 *    - Commands trigger interactions via TriggerInteraction().
 *
 * 8. COOLDOWNS:
 *    - CanTriggerInteraction() checks if cooldown has elapsed.
 *    - GetInteractionCooldownRemaining() shows time until next interaction.
 *    - LastInteractionTime tracks when last interaction occurred.
 *    - Prevents chat from overwhelming the game with effects.
 *
 * 9. CHAOS MODE:
 *    - AllowChaosMode() enables a period of rapid viewer interactions.
 *    - Cooldowns are reduced or removed during chaos mode.
 *    - bChaosModeActive flag tracks current state.
 *    - Fun for special stream events but can be overwhelming.
 *
 * 10. STREAM STATS (FMGStreamStats):
 *     - CurrentViewers: Live viewer count (updated via UpdateViewerCount).
 *     - TotalInteractions: How many viewer effects triggered.
 *     - TotalVotes: Participation in polls.
 *     - StreamDuration: How long the stream has been live.
 *     - RacesCompleted: Races finished during this stream.
 *
 * DELEGATES (Events):
 * - OnViewerInteraction: Viewer triggered an interaction.
 * - OnPollStarted: New poll is now active.
 * - OnPollEnded: Poll finished with WinningIndex.
 * - OnStreamConnected: Successfully connected to platform.
 * - OnStreamDisconnected: Lost connection to platform.
 *
 * DIFFERENCE FROM MGStreamIntegrationSubsystem:
 * - StreamerMode is SIMPLER: basic polls, interactions, privacy.
 * - StreamIntegration is DEEPER: channel points, predictions, alerts, full chat.
 * - StreamerMode is for casual streamers who want quick setup.
 * - StreamIntegration is for professional streamers wanting full control.
 * - Both can coexist; StreamerMode can use StreamIntegration under the hood.
 *
 * WORKFLOW EXAMPLE:
 * 1. Streamer opens settings, enables Streamer Mode
 * 2. Configures: hide player names, allow interactions, 30s cooldown
 * 3. Connects to Twitch via ConnectToPlatform()
 * 4. OnStreamConnected fires, UI shows "Live on Twitch"
 * 5. Before race, StartTrackVote() creates poll for track selection
 * 6. Chat votes via "!vote 1", "!vote 2", etc.
 * 7. Poll ends, OnPollEnded fires with winning track
 * 8. Race starts on viewer-chosen track
 * 9. During race, subscriber types "!boost"
 * 10. System checks cooldown, triggers BoostPlayer interaction
 * 11. Streamer gets speed boost, chat sees "Boost from ViewerName!"
 *
 * SETTINGS PERSISTENCE:
 * - LoadSettings()/SaveSettings() store preferences between sessions.
 * - Streamers don't need to reconfigure each time they play.
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGStreamerModeSubsystem.generated.h"

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

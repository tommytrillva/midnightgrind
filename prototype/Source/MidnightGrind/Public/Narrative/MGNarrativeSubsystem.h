// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGNarrativeSubsystem.h - Live World Storytelling & Dynamic Narrative System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Narrative Subsystem, which creates emergent storytelling
 * in Midnight Grind's multiplayer world. Unlike the Campaign Subsystem (which handles
 * scripted story missions), this system generates dynamic narrative moments based on
 * what's actually happening in the game world.
 *
 * Think of this as a "sports commentator" combined with a "documentary filmmaker" -
 * it observes what players are doing and creates narrative context around it.
 *
 *
 * KEY CONCEPTS & TERMINOLOGY:
 * ---------------------------
 *
 * 1. NARRATIVE EVENT: A significant moment that becomes part of the game's "history."
 *    - PersonalMilestone: Your individual achievements (first win, new record, etc.)
 *    - RivalShowdown: When you face off against a designated rival player
 *    - CrewMoment: Achievements by your crew as a whole
 *    - CommunityEvent: Server-wide happenings (tournaments, special events)
 *    - SeasonStory: Narrative beats tied to the current season
 *    - LegendaryRace: Historic moments that become part of server lore
 *
 * 2. STORY THREAD: An ongoing narrative arc that develops over time.
 *    - Example: "The Rivalry with CrashKing42" - tracks your ongoing competition
 *    - Threads have progress (0-100) and can be completed
 *    - Multiple threads can be active simultaneously
 *
 * 3. CONTEXTUAL DIALOGUE: Dynamic voice lines that react to in-game situations.
 *    - The RadioHost provides commentary during races and in the world
 *    - Different sources (CrewLeader, Rival, etc.) have different personalities
 *    - Context determines what lines can play (in race? winning? losing?)
 *
 * 4. WITNESS SYSTEM: Tracks whether other players saw an event happen.
 *    - Events witnessed by more players become more "legendary"
 *    - Affects how the event is remembered and referenced later
 *
 * 5. NARRATIVE CONTEXT: The current situation that affects dialogue selection.
 *    - Includes: in-race status, position, streak, rivals present, etc.
 *    - Used to pick appropriate dialogue that feels relevant
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 * This is a UGameInstanceSubsystem that:
 * - Persists throughout the game session
 * - Receives events from various gameplay systems
 * - Maintains a history of narrative events
 * - Drives contextual audio/dialogue playback
 *
 * Key relationships:
 * - MGCampaignSubsystem: Handles scripted story; Narrative handles emergent story
 * - Race System: Reports race events (position changes, finishes, etc.)
 * - Multiplayer System: Notifies about other players, crew activities
 * - Audio System: Plays the dialogue lines this system triggers
 *
 *
 * DESIGN PHILOSOPHY:
 * ------------------
 * The goal is to make multiplayer feel like it has a "story" even though it's
 * player-driven. When you beat a rival, the game acknowledges it. When your
 * crew dominates a tournament, that becomes part of the world's narrative.
 *
 * This creates:
 * - Personal investment (your story matters)
 * - Social dynamics (rivalries, crew loyalty)
 * - Memorable moments (events become "history")
 *
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // After a race finishes, create a narrative event:
 * FMGNarrativeEvent Event;
 * Event.EventID = "FirstWin_2024_01_15";
 * Event.Type = EMGNarrativeEventType::PersonalMilestone;
 * Event.Title = LOCTEXT("FirstWin", "First Victory!");
 * NarrativeSubsystem->TriggerEvent(Event);
 *
 * // Trigger contextual dialogue during a race:
 * FMGNarrativeContext Context;
 * Context.bInRace = true;
 * Context.bRivalInRace = true;
 * Context.CurrentPosition = 2;
 * NarrativeSubsystem->TriggerContextualDialogue(Context);
 *
 * // Advance a story thread when something significant happens:
 * NarrativeSubsystem->AdvanceThread("Rivalry_CrashKing42", 10);
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNarrativeSubsystem.generated.h"

/**
 * =============================================================================
 * DESIGN PHILOSOPHY: Live World Storytelling
 * =============================================================================
 *
 * Unlike scripted story campaigns, the Narrative system creates emergent
 * storytelling from actual gameplay. Key principles:
 *
 * - Story events happen in the LIVE multiplayer world (not instanced)
 * - Dynamic narrative adapts to player actions and outcomes
 * - Crew-based storylines evolve based on collective achievements
 * - Community events create shared memories across the server
 * - Personal story threads develop within the larger world narrative
 *
 * This creates a "living history" of the game world where player
 * achievements become part of the ongoing narrative.
 * =============================================================================
 */

/**
 * Narrative Event Type - Categories of story-worthy moments
 *
 * These types determine how events are presented, stored, and referenced
 * in the game's ongoing narrative. Different types have different
 * visibility (personal vs public) and persistence.
 */
UENUM(BlueprintType)
enum class EMGNarrativeEventType : uint8
{
	/** Your individual achievements (first win, personal bests, milestones). */
	PersonalMilestone,

	/** Significant encounters with rival players (especially repeat matchups). */
	RivalShowdown,

	/** Achievements involving your crew (group wins, territory control). */
	CrewMoment,

	/** Server-wide events (tournaments, seasonal events, world records). */
	CommunityEvent,

	/** Story beats tied to the current season's narrative arc. */
	SeasonStory,

	/** Historic races that become server lore (photo finishes, upsets). */
	LegendaryRace
};

/**
 * Dialogue Source - Who is speaking contextual dialogue
 *
 * Each source has a distinct personality and triggers under different
 * circumstances. The audio system uses this to select appropriate
 * voice actors and UI presentation style.
 */
UENUM(BlueprintType)
enum class EMGDialogueSource : uint8
{
	/** The in-game radio DJ. Provides commentary and race updates. Most common. */
	RadioHost,

	/** Your crew's leader. Talks about crew objectives and rivalries. */
	CrewLeader,

	/** Your designated rival. Taunts, challenges, grudging respect. */
	Rival,

	/** Cryptic informant. Hints at secrets and hidden content. */
	MysteriousStranger,

	/** Your mechanic contact. Car advice, part recommendations. */
	MechanicContact,

	/** Underground racing network. Event announcements, street cred talk. */
	Underground
};

/**
 * Narrative Event - A recorded moment in the game's "history"
 *
 * Events represent significant moments that become part of the game world's
 * ongoing story. They can be referenced later in dialogue, displayed in
 * player profiles, and used to build rivalry/crew narratives.
 */
USTRUCT(BlueprintType)
struct FMGNarrativeEvent
{
	GENERATED_BODY()

	/** Unique identifier for this event (often includes date for uniqueness). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	/** What kind of narrative moment this represents. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNarrativeEventType Type = EMGNarrativeEventType::PersonalMilestone;

	/** Headline for the event (e.g., "First Victory!", "Upset of the Year"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	/** Detailed description for event logs and history screens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** When this event occurred. Used for sorting and "on this day" features. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/** Player IDs who were part of this event (for tagging and notifications). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedPlayers;

	/** Which track/route this happened on (for location-based history). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RelatedTrack;

	/** Which crew was involved (for crew history displays). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RelatedCrew;

	/** True if other players saw this happen (adds weight to the story). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasWitnessed = false;

	/** How many players witnessed this. Higher = more "legendary". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WitnessCount = 0;
};

/**
 * Dialogue Line (Narrative version) - Contextual voice line data
 *
 * NOTE: This is different from Campaign's FMGDialogueLine. This version
 * is for dynamic, contextual dialogue that plays during gameplay (like
 * radio commentary), not scripted story conversations.
 */
USTRUCT(BlueprintType)
struct FMGDialogueLine
{
	GENERATED_BODY()

	/** Who is speaking (RadioHost, Rival, etc.). Determines voice and UI style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDialogueSource Source = EMGDialogueSource::RadioHost;

	/** Display name for the speaker (e.g., "DJ Nitro", "Your Mechanic"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpeakerName;

	/** The actual text that appears in subtitles (if enabled). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DialogueText;

	/** Reference to the audio file in the voice-over system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VoiceLineID;

	/** Length of the audio clip (for timing next dialogue or UI display). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;
};

/**
 * Narrative Context - Current game state for dialogue selection
 *
 * The narrative system uses context to pick appropriate dialogue lines.
 * For example, the radio host will say different things if you're winning
 * vs losing, or if your rival is in the race.
 *
 * This struct is a "snapshot" of what's happening that dialogue can react to.
 */
USTRUCT(BlueprintType)
struct FMGNarrativeContext
{
	GENERATED_BODY()

	/** True if player is currently in an active race (vs free roam). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInRace = false;

	/** True if player's designated rival is in this race. Enables trash talk. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRivalInRace = false;

	/** True if player's nemesis (worst enemy) is in this race. Extra drama. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNemesisInRace = false;

	/** Current race position (1 = first, 0 = not in race). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPosition = 0;

	/** True if player has won multiple races in a row. "Hot streak" dialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnWinStreak = false;

	/** True immediately after winning. Celebration dialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJustWon = false;

	/** True immediately after losing. Consolation/motivation dialogue. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJustLost = false;

	/** Which track player is on. Location-specific commentary. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentTrackID;
};

/**
 * Story Thread - An ongoing narrative arc that develops over time
 *
 * Story threads are long-running narratives that develop across multiple
 * gameplay sessions. They track things like rivalries, crew storylines,
 * and personal journeys.
 *
 * Example: "The Rivalry with CrashKing42" starts at 0 progress and advances
 * each time you race them, culminating in a final showdown at 100.
 */
USTRUCT(BlueprintType)
struct FMGStoryThread
{
	GENERATED_BODY()

	/** Unique identifier (e.g., "Rivalry_CrashKing42", "CrewTakeover_Downtown"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ThreadID;

	/** Player-facing name for the journal/narrative log. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ThreadName;

	/** Description of current story state ("The tension is building..."). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CurrentState;

	/** Current progress value (0 to MaxProgress). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Progress = 0;

	/** When Progress reaches this value, thread is complete. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxProgress = 100;

	/** Events that contributed to this thread (for history display). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGNarrativeEvent> RelatedEvents;

	/** True when thread has reached its conclusion. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

/**
 * =============================================================================
 * DELEGATE DECLARATIONS
 * =============================================================================
 *
 * These events allow UI and other systems to react to narrative moments.
 * Bind to these in your widgets to update displays when narrative changes.
 */

/** Broadcast when a significant narrative moment occurs. Show notification/log it. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNarrativeEventTriggered, const FMGNarrativeEvent&, Event);

/** Broadcast when contextual dialogue should play. Audio system listens to this. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDialogueTriggered, const FMGDialogueLine&, Dialogue);

/** Broadcast when a story thread progresses. Update journal/narrative UI. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStoryThreadAdvanced, const FMGStoryThread&, Thread);

/** Broadcast when a story thread reaches its conclusion. Major narrative moment! */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStoryThreadCompleted, const FMGStoryThread&, Thread);

/**
 * =============================================================================
 * UMGNarrativeSubsystem - Dynamic Storytelling & Commentary System
 * =============================================================================
 *
 * This subsystem creates emergent narrative from gameplay events. It:
 * - Records significant moments as "narrative events"
 * - Maintains ongoing "story threads" that develop over time
 * - Triggers contextual dialogue based on game state
 * - Provides race commentary and world building
 *
 * As a UGameInstanceSubsystem:
 * - Automatically created when game starts
 * - Access via: GetGameInstance()->GetSubsystem<UMGNarrativeSubsystem>()
 * - Persists across level loads
 *
 * The class is organized into functional sections:
 * - Narrative Events: Recording and querying significant moments
 * - Contextual Dialogue: Reactive voice lines during gameplay
 * - Story Threads: Long-running narrative arcs
 * - Race Commentary: Specific hooks for race events
 * - World Events: Server/community narrative moments
 */
UCLASS()
class MIDNIGHTGRIND_API UMGNarrativeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Called by engine when subsystem is created. Sets up initial state. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Called by engine when subsystem is destroyed. Cleans up and saves. */
	virtual void Deinitialize() override;

	// Narrative Events
	UFUNCTION(BlueprintCallable, Category = "Narrative")
	void TriggerEvent(const FMGNarrativeEvent& Event);

	UFUNCTION(BlueprintPure, Category = "Narrative")
	TArray<FMGNarrativeEvent> GetRecentEvents(int32 Count = 10) const;

	UFUNCTION(BlueprintPure, Category = "Narrative")
	TArray<FMGNarrativeEvent> GetEventsByType(EMGNarrativeEventType Type) const;

	// Contextual Dialogue
	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void TriggerContextualDialogue(const FMGNarrativeContext& Context);

	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void PlayDialogue(const FMGDialogueLine& Line);

	UFUNCTION(BlueprintCallable, Category = "Narrative|Dialogue")
	void QueueDialogue(const FMGDialogueLine& Line);

	UFUNCTION(BlueprintPure, Category = "Narrative|Dialogue")
	bool IsDialoguePlaying() const { return bDialoguePlaying; }

	// Story Threads
	UFUNCTION(BlueprintPure, Category = "Narrative|Story")
	TArray<FMGStoryThread> GetActiveThreads() const;

	UFUNCTION(BlueprintCallable, Category = "Narrative|Story")
	void AdvanceThread(FName ThreadID, int32 Progress);

	UFUNCTION(BlueprintPure, Category = "Narrative|Story")
	FMGStoryThread GetThread(FName ThreadID) const;

	// Race Commentary
	UFUNCTION(BlueprintCallable, Category = "Narrative|Race")
	void OnRaceStart(const TArray<FString>& RacerIDs, FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "Narrative|Race")
	void OnPositionChange(int32 OldPosition, int32 NewPosition);

	UFUNCTION(BlueprintCallable, Category = "Narrative|Race")
	void OnRivalPassed(const FString& RivalID);

	UFUNCTION(BlueprintCallable, Category = "Narrative|Race")
	void OnRaceFinish(int32 FinalPosition, bool bWasCloseRace);

	// World Events
	UFUNCTION(BlueprintCallable, Category = "Narrative|World")
	void OnCommunityEventStarted(FName EventID, const FText& EventName);

	UFUNCTION(BlueprintCallable, Category = "Narrative|World")
	void OnCrewAchievement(FName CrewID, const FText& Achievement);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
	FMGOnNarrativeEventTriggered OnNarrativeEventTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
	FMGOnDialogueTriggered OnDialogueTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
	FMGOnStoryThreadAdvanced OnStoryThreadAdvanced;

	UPROPERTY(BlueprintAssignable, Category = "Narrative|Events")
	FMGOnStoryThreadCompleted OnStoryThreadCompleted;

protected:
	void LoadNarrativeData();
	void SaveNarrativeData();
	void InitializeStoryThreads();
	void ProcessDialogueQueue();
	FMGDialogueLine GenerateContextualLine(const FMGNarrativeContext& Context);
	FMGStoryThread* FindThread(FName ThreadID);

private:
	UPROPERTY()
	TArray<FMGNarrativeEvent> EventHistory;

	UPROPERTY()
	TArray<FMGStoryThread> StoryThreads;

	UPROPERTY()
	TArray<FMGDialogueLine> DialogueQueue;

	bool bDialoguePlaying = false;
	FTimerHandle DialogueTimerHandle;
	int32 MaxEventHistory = 100;
};

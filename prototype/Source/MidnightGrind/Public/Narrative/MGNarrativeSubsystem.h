// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGNarrativeSubsystem.generated.h"

/**
 * Narrative System - Live World Storytelling
 * - Story events happen in the live multiplayer world
 * - Dynamic narrative based on player actions
 * - Crew-based storylines evolve together
 * - Community events create shared narrative moments
 * - Personal story threads within the larger world
 */

UENUM(BlueprintType)
enum class EMGNarrativeEventType : uint8
{
	PersonalMilestone,   // Your individual story beat
	RivalShowdown,       // Major rival encounter
	CrewMoment,          // Crew achievement
	CommunityEvent,      // Server-wide event
	SeasonStory,         // Season narrative beat
	LegendaryRace        // Historic race moment
};

UENUM(BlueprintType)
enum class EMGDialogueSource : uint8
{
	RadioHost,          // In-game radio personality
	CrewLeader,
	Rival,
	MysteriousStranger,
	MechanicContact,
	Underground
};

USTRUCT(BlueprintType)
struct FMGNarrativeEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EventID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGNarrativeEventType Type = EMGNarrativeEventType::PersonalMilestone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> InvolvedPlayers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RelatedTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RelatedCrew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasWitnessed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WitnessCount = 0;
};

USTRUCT(BlueprintType)
struct FMGDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDialogueSource Source = EMGDialogueSource::RadioHost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VoiceLineID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGNarrativeContext
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bInRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRivalInRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNemesisInRace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPosition = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOnWinStreak = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJustWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bJustLost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentTrackID;
};

USTRUCT(BlueprintType)
struct FMGStoryThread
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ThreadID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ThreadName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText CurrentState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Progress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxProgress = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGNarrativeEvent> RelatedEvents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnNarrativeEventTriggered, const FMGNarrativeEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDialogueTriggered, const FMGDialogueLine&, Dialogue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStoryThreadAdvanced, const FMGStoryThread&, Thread);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnStoryThreadCompleted, const FMGStoryThread&, Thread);

UCLASS()
class MIDNIGHTGRIND_API UMGNarrativeSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
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

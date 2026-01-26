// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCampaignSubsystem.generated.h"

/**
 * Mission type
 */
UENUM(BlueprintType)
enum class EMGMissionType : uint8
{
	Story UMETA(DisplayName = "Story Mission"),
	Side UMETA(DisplayName = "Side Mission"),
	Crew UMETA(DisplayName = "Crew Mission"),
	Rival UMETA(DisplayName = "Rival Showdown"),
	Boss UMETA(DisplayName = "Boss Battle"),
	Tutorial UMETA(DisplayName = "Tutorial"),
	Challenge UMETA(DisplayName = "Challenge"),
	Secret UMETA(DisplayName = "Secret Mission")
};

/**
 * Mission status
 */
UENUM(BlueprintType)
enum class EMGMissionStatus : uint8
{
	Locked UMETA(DisplayName = "Locked"),
	Available UMETA(DisplayName = "Available"),
	InProgress UMETA(DisplayName = "In Progress"),
	Completed UMETA(DisplayName = "Completed"),
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Objective type
 */
UENUM(BlueprintType)
enum class EMGObjectiveType : uint8
{
	WinRace UMETA(DisplayName = "Win Race"),
	FinishRace UMETA(DisplayName = "Finish Race"),
	ReachPosition UMETA(DisplayName = "Reach Position"),
	DriftScore UMETA(DisplayName = "Achieve Drift Score"),
	TopSpeed UMETA(DisplayName = "Reach Top Speed"),
	EvadePolice UMETA(DisplayName = "Evade Police"),
	DeliverCar UMETA(DisplayName = "Deliver Vehicle"),
	ReachLocation UMETA(DisplayName = "Reach Location"),
	TailTarget UMETA(DisplayName = "Tail Target"),
	EscapeTarget UMETA(DisplayName = "Escape Pursuer"),
	EarnCash UMETA(DisplayName = "Earn Cash"),
	BuyPart UMETA(DisplayName = "Buy Part"),
	BuyCar UMETA(DisplayName = "Buy Car"),
	TuneVehicle UMETA(DisplayName = "Tune Vehicle"),
	DefeatRival UMETA(DisplayName = "Defeat Rival"),
	JoinCrew UMETA(DisplayName = "Join Crew"),
	WinPinkSlip UMETA(DisplayName = "Win Pink Slip"),
	TimeLimit UMETA(DisplayName = "Beat Time Limit"),
	SurvivePursuit UMETA(DisplayName = "Survive Pursuit"),
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Character role in story
 */
UENUM(BlueprintType)
enum class EMGCharacterRole : uint8
{
	Player UMETA(DisplayName = "Player"),
	Ally UMETA(DisplayName = "Ally"),
	Mentor UMETA(DisplayName = "Mentor"),
	Rival UMETA(DisplayName = "Rival"),
	Boss UMETA(DisplayName = "Boss"),
	Contact UMETA(DisplayName = "Contact"),
	LoveInterest UMETA(DisplayName = "Love Interest"),
	Antagonist UMETA(DisplayName = "Antagonist"),
	NPC UMETA(DisplayName = "NPC")
};

/**
 * Dialogue line
 */
USTRUCT(BlueprintType)
struct FMGDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName CharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Line;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TSoftObjectPtr<USoundBase> VoiceOver;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float Duration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName EmotionTag; // For character animation

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bIsPhoneCall = false;
};

/**
 * Story character
 */
USTRUCT(BlueprintType)
struct FMGStoryCharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FText Nickname;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EMGCharacterRole Role = EMGCharacterRole::NPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TSoftObjectPtr<UTexture2D> Portrait;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TSoftObjectPtr<UTexture2D> PhoneIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (MultiLine = true))
	FText Bio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName SignatureVehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 RelationshipLevel = 0; // -100 to 100

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool bUnlocked = false;
};

/**
 * Mission objective
 */
USTRUCT(BlueprintType)
struct FMGMissionObjective
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName ObjectiveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	EMGObjectiveType Type = EMGObjectiveType::WinRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 TargetValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 CurrentValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetRaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetLocationID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetCharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsOptional = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsHidden = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int64 BonusCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 BonusREP = 0;
};

/**
 * Mission reward
 */
USTRUCT(BlueprintType)
struct FMGMissionReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int64 Cash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 REP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 XP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockVehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockPartID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockAreaID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockCharacterID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockAchievementID;
};

/**
 * Mission definition
 */
USTRUCT(BlueprintType)
struct FMGMissionDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	EMGMissionType Type = EMGMissionType::Story;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 ChapterNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MissionNumber = 1;

	// Prerequisites
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	TArray<FName> RequiredMissions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredREP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	FName RequiredVehicleID; // Must own specific car

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredPI = 0; // Minimum PI

	// Content
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGMissionObjective> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGDialogueLine> IntroDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGDialogueLine> OutroDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGDialogueLine> FailDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FName MissionGiverID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FVector StartLocation = FVector::ZeroVector;

	// Rewards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FMGMissionReward CompletionReward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FMGMissionReward PerfectReward; // All optional objectives

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsReplayable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float TimeLimit = 0.0f; // 0 = no limit

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bPoliceEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bTrafficEnabled = true;

	// Visuals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> MissionIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> MissionBanner;
};

/**
 * Chapter definition
 */
USTRUCT(BlueprintType)
struct FMGChapterDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	int32 ChapterNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TArray<FName> MissionIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FName BossMissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FName MainAntagonistID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TArray<FName> UnlockedAreas;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TSoftObjectPtr<UTexture2D> ChapterArt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TSoftObjectPtr<USoundBase> ChapterTheme;
};

/**
 * Mission progress state
 */
USTRUCT(BlueprintType)
struct FMGMissionProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FName MissionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	EMGMissionStatus Status = EMGMissionStatus::Locked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FMGMissionObjective> ObjectiveProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 AttemptCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CompletionCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bPerfectCompletion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime FirstCompletionTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime LastAttemptTime;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, FName, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionCompleted, FName, MissionID, bool, bPerfect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, FName, MissionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, MissionID, FName, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, FName, MissionID, FName, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChapterUnlocked, int32, ChapterNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChapterCompleted, int32, ChapterNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const TArray<FMGDialogueLine>&, Dialogue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterRelationshipChanged, FName, CharacterID, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionAvailable, FName, MissionID);

/**
 * Campaign/Story Subsystem
 *
 * Manages the game's story campaign, missions,
 * objectives, characters, and narrative progression.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGCampaignSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMissionStarted OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMissionCompleted OnMissionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMissionFailed OnMissionFailed;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveUpdated OnObjectiveUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnObjectiveCompleted OnObjectiveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChapterUnlocked OnChapterUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnChapterCompleted OnChapterCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueStarted OnDialogueStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDialogueEnded OnDialogueEnded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCharacterRelationshipChanged OnCharacterRelationshipChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMissionAvailable OnMissionAvailable;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Registration")
	void RegisterMission(const FMGMissionDefinition& Mission);

	UFUNCTION(BlueprintCallable, Category = "Registration")
	void RegisterChapter(const FMGChapterDefinition& Chapter);

	UFUNCTION(BlueprintCallable, Category = "Registration")
	void RegisterCharacter(const FMGStoryCharacter& Character);

	// Mission Management
	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool StartMission(FName MissionID);

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void CompleteMission(bool bPerfect = false);

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void FailMission();

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void AbandonMission();

	UFUNCTION(BlueprintPure, Category = "Missions")
	bool IsInMission() const { return bInMission; }

	UFUNCTION(BlueprintPure, Category = "Missions")
	FName GetCurrentMissionID() const { return CurrentMissionID; }

	UFUNCTION(BlueprintPure, Category = "Missions")
	FMGMissionDefinition GetCurrentMission() const;

	UFUNCTION(BlueprintPure, Category = "Missions")
	FMGMissionDefinition GetMission(FName MissionID) const;

	UFUNCTION(BlueprintPure, Category = "Missions")
	FMGMissionProgress GetMissionProgress(FName MissionID) const;

	UFUNCTION(BlueprintPure, Category = "Missions")
	TArray<FMGMissionDefinition> GetAvailableMissions() const;

	UFUNCTION(BlueprintPure, Category = "Missions")
	TArray<FMGMissionDefinition> GetMissionsByType(EMGMissionType Type) const;

	UFUNCTION(BlueprintPure, Category = "Missions")
	bool IsMissionAvailable(FName MissionID) const;

	UFUNCTION(BlueprintPure, Category = "Missions")
	bool IsMissionCompleted(FName MissionID) const;

	// Objectives
	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void UpdateObjective(FName ObjectiveID, int32 NewValue);

	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void IncrementObjective(FName ObjectiveID, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Objectives")
	void CompleteObjective(FName ObjectiveID);

	UFUNCTION(BlueprintPure, Category = "Objectives")
	TArray<FMGMissionObjective> GetCurrentObjectives() const;

	UFUNCTION(BlueprintPure, Category = "Objectives")
	FMGMissionObjective GetObjective(FName ObjectiveID) const;

	// Chapters
	UFUNCTION(BlueprintPure, Category = "Chapters")
	int32 GetCurrentChapter() const { return CurrentChapter; }

	UFUNCTION(BlueprintPure, Category = "Chapters")
	FMGChapterDefinition GetChapter(int32 ChapterNumber) const;

	UFUNCTION(BlueprintPure, Category = "Chapters")
	bool IsChapterUnlocked(int32 ChapterNumber) const;

	UFUNCTION(BlueprintPure, Category = "Chapters")
	bool IsChapterCompleted(int32 ChapterNumber) const;

	UFUNCTION(BlueprintPure, Category = "Chapters")
	float GetChapterProgress(int32 ChapterNumber) const;

	// Characters
	UFUNCTION(BlueprintPure, Category = "Characters")
	FMGStoryCharacter GetCharacter(FName CharacterID) const;

	UFUNCTION(BlueprintPure, Category = "Characters")
	TArray<FMGStoryCharacter> GetAllCharacters() const;

	UFUNCTION(BlueprintPure, Category = "Characters")
	TArray<FMGStoryCharacter> GetCharactersByRole(EMGCharacterRole Role) const;

	UFUNCTION(BlueprintCallable, Category = "Characters")
	void ModifyRelationship(FName CharacterID, int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Characters")
	int32 GetRelationship(FName CharacterID) const;

	// Dialogue
	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(const TArray<FMGDialogueLine>& Dialogue);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SkipDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsInDialogue() const { return bInDialogue; }

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	FMGDialogueLine GetCurrentDialogueLine() const;

	// Progress/Stats
	UFUNCTION(BlueprintPure, Category = "Progress")
	float GetOverallStoryProgress() const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetTotalMissionsCompleted() const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetTotalPerfectCompletions() const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Persistence")
	TArray<FMGMissionProgress> GetAllMissionProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void LoadMissionProgress(const TArray<FMGMissionProgress>& Progress);

protected:
	void CheckMissionAvailability();
	void CheckChapterCompletion();
	bool CheckPrerequisites(const FMGMissionDefinition& Mission) const;

private:
	// Registered content
	UPROPERTY()
	TMap<FName, FMGMissionDefinition> RegisteredMissions;

	UPROPERTY()
	TMap<int32, FMGChapterDefinition> RegisteredChapters;

	UPROPERTY()
	TMap<FName, FMGStoryCharacter> RegisteredCharacters;

	// Progress tracking
	UPROPERTY()
	TMap<FName, FMGMissionProgress> MissionProgress;

	UPROPERTY()
	TSet<int32> CompletedChapters;

	// Current state
	UPROPERTY()
	bool bInMission = false;

	UPROPERTY()
	FName CurrentMissionID;

	UPROPERTY()
	TArray<FMGMissionObjective> CurrentObjectives;

	UPROPERTY()
	int32 CurrentChapter = 1;

	// Dialogue state
	UPROPERTY()
	bool bInDialogue = false;

	UPROPERTY()
	TArray<FMGDialogueLine> CurrentDialogue;

	UPROPERTY()
	int32 CurrentDialogueIndex = 0;
};

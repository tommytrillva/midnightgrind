// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGCampaignSubsystem.h - Story Campaign & Mission Management System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Campaign Subsystem, which is the backbone of Midnight Grind's
 * single-player story experience. It manages everything related to the narrative-driven
 * gameplay: missions, objectives, story chapters, character relationships, and dialogue.
 *
 * Think of this as the "story director" - it keeps track of where you are in the game's
 * narrative, what missions are available, and what happens when you complete (or fail) them.
 *
 *
 * KEY CONCEPTS & TERMINOLOGY:
 * ---------------------------
 *
 * 1. MISSION: A discrete gameplay task with specific objectives and rewards.
 *    - Example: "Race Johnny Thunder on the highway and win to prove yourself"
 *    - Missions have prerequisites (what you need to unlock them) and rewards (what you earn)
 *
 * 2. OBJECTIVE: A specific goal within a mission that must be completed.
 *    - Primary objectives: MUST be completed to finish the mission
 *    - Optional objectives: Bonus goals for extra rewards (marked with bIsOptional)
 *    - Example: Primary = "Win the race", Optional = "Finish with 50,000+ drift score"
 *
 * 3. CHAPTER: A grouping of related missions that form an "act" of the story.
 *    - Each chapter has a main antagonist and culminates in a boss battle
 *    - Completing a chapter unlocks new areas and story content
 *
 * 4. STORY CHARACTER: An NPC (non-player character) in the game's narrative.
 *    - Characters have ROLES (Ally, Rival, Mentor, etc.) that define their relationship
 *    - RELATIONSHIP LEVEL (-100 to 100) tracks how friendly/hostile they are to the player
 *    - Characters can give missions, appear in dialogue, and have signature vehicles
 *
 * 5. DIALOGUE: Conversations between characters that advance the story.
 *    - Can include voice-over audio, character portraits, and emotion tags for animation
 *    - Phone calls are a special type of dialogue (bIsPhoneCall flag)
 *
 * 6. REP (Reputation): A currency-like value earned through gameplay.
 *    - Many missions require a minimum REP to unlock
 *    - REP represents your standing in the street racing community
 *
 * 7. PI (Performance Index): A measure of your vehicle's overall capability.
 *    - Some missions require a minimum PI to ensure fair challenge
 *
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 *
 * This subsystem is a UGameInstanceSubsystem, meaning:
 * - There's ONE instance that persists for the entire game session
 * - It survives level transitions (unlike actors in a level)
 * - It's automatically created when the game starts
 *
 * It works closely with:
 * - MGNarrativeSubsystem: For dynamic story events in the live world
 * - MGCareerSubsystem: For tracking overall player progression
 * - MGSaveSubsystem (if exists): For persisting mission progress to disk
 * - UI System: Broadcasts events that UI widgets listen to for updates
 *
 *
 * COMMON USAGE PATTERNS:
 * ----------------------
 *
 * // Starting a mission from the map UI:
 * if (CampaignSubsystem->IsMissionAvailable("Mission_Ch1_Race1"))
 * {
 *     CampaignSubsystem->StartMission("Mission_Ch1_Race1");
 * }
 *
 * // Updating objective progress during gameplay:
 * CampaignSubsystem->IncrementObjective("DriftScore", DriftPoints);
 *
 * // Listening for mission completion in UI:
 * CampaignSubsystem->OnMissionCompleted.AddDynamic(this, &UMyWidget::HandleMissionComplete);
 *
 *
 * DATA FLOW:
 * ----------
 * 1. Game designers register missions/chapters/characters at startup
 * 2. Player selects available mission from UI
 * 3. Subsystem validates prerequisites and starts mission
 * 4. Gameplay systems report objective progress
 * 5. Subsystem broadcasts events (UI updates, rewards granted)
 * 6. Mission completes/fails, progress is saved
 *
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCampaignSubsystem.generated.h"

/**
 * Mission Type - Categorizes missions by their narrative purpose
 *
 * Different mission types have different unlock conditions, rewards, and
 * map markers. The type determines how the mission appears in the UI and
 * how it affects story progression.
 */
UENUM(BlueprintType)
enum class EMGMissionType : uint8
{
	/** Main story missions - Required for campaign progression. These advance the plot. */
	Story UMETA(DisplayName = "Story Mission"),

	/** Optional missions - Extra content for rewards and world-building. */
	Side UMETA(DisplayName = "Side Mission"),

	/** Crew-related missions - Involve your racing crew members. */
	Crew UMETA(DisplayName = "Crew Mission"),

	/** Rival showdowns - Head-to-head races against named antagonists. */
	Rival UMETA(DisplayName = "Rival Showdown"),

	/** Boss battles - Chapter-ending confrontations with major antagonists. */
	Boss UMETA(DisplayName = "Boss Battle"),

	/** Tutorial missions - Teach game mechanics (usually in Chapter 1). */
	Tutorial UMETA(DisplayName = "Tutorial"),

	/** Challenge missions - Repeatable skill-based challenges. */
	Challenge UMETA(DisplayName = "Challenge"),

	/** Secret missions - Hidden content discovered through exploration. */
	Secret UMETA(DisplayName = "Secret Mission")
};

/**
 * Mission Status - The current state of a mission in the player's progress
 *
 * Missions transition through these states as the player progresses.
 * State machine: Locked -> Available -> InProgress -> Completed/Failed
 */
UENUM(BlueprintType)
enum class EMGMissionStatus : uint8
{
	/** Prerequisites not met - Mission is visible but cannot be started. */
	Locked UMETA(DisplayName = "Locked"),

	/** Ready to play - Prerequisites met, mission can be started. */
	Available UMETA(DisplayName = "Available"),

	/** Currently active - Player has started this mission. */
	InProgress UMETA(DisplayName = "In Progress"),

	/** Successfully finished - All required objectives completed. */
	Completed UMETA(DisplayName = "Completed"),

	/** Did not complete - Player quit or failed objectives. Can retry. */
	Failed UMETA(DisplayName = "Failed")
};

/**
 * Objective Type - What the player must do to complete a mission objective
 *
 * Each objective has a type that determines how progress is measured and
 * what gameplay systems report completion. The type also affects UI display.
 */
UENUM(BlueprintType)
enum class EMGObjectiveType : uint8
{
	/** Come in 1st place in a race. */
	WinRace UMETA(DisplayName = "Win Race"),

	/** Cross the finish line (any position). */
	FinishRace UMETA(DisplayName = "Finish Race"),

	/** Achieve a specific position (e.g., top 3). Uses TargetValue. */
	ReachPosition UMETA(DisplayName = "Reach Position"),

	/** Accumulate drift points. Uses TargetValue for required score. */
	DriftScore UMETA(DisplayName = "Achieve Drift Score"),

	/** Hit a speed threshold (km/h or mph). Uses TargetValue. */
	TopSpeed UMETA(DisplayName = "Reach Top Speed"),

	/** Escape from police pursuit without getting busted. */
	EvadePolice UMETA(DisplayName = "Evade Police"),

	/** Transport a vehicle to a destination without damage. */
	DeliverCar UMETA(DisplayName = "Deliver Vehicle"),

	/** Navigate to a specific map location. Uses TargetLocationID. */
	ReachLocation UMETA(DisplayName = "Reach Location"),

	/** Follow a target without being detected. Stealth mission type. */
	TailTarget UMETA(DisplayName = "Tail Target"),

	/** Get away from someone chasing you. */
	EscapeTarget UMETA(DisplayName = "Escape Pursuer"),

	/** Accumulate in-game currency. Uses TargetValue for amount. */
	EarnCash UMETA(DisplayName = "Earn Cash"),

	/** Purchase any performance or visual part. */
	BuyPart UMETA(DisplayName = "Buy Part"),

	/** Purchase any vehicle. */
	BuyCar UMETA(DisplayName = "Buy Car"),

	/** Modify vehicle performance (install parts, tune settings). */
	TuneVehicle UMETA(DisplayName = "Tune Vehicle"),

	/** Beat a specific rival character. Uses TargetCharacterID. */
	DefeatRival UMETA(DisplayName = "Defeat Rival"),

	/** Become a member of a racing crew. */
	JoinCrew UMETA(DisplayName = "Join Crew"),

	/** Win a pink slip race (winner takes loser's car). */
	WinPinkSlip UMETA(DisplayName = "Win Pink Slip"),

	/** Complete something within a time limit. Uses TargetValue (seconds). */
	TimeLimit UMETA(DisplayName = "Beat Time Limit"),

	/** Stay uncaught for a duration during a pursuit. */
	SurvivePursuit UMETA(DisplayName = "Survive Pursuit"),

	/** Designer-defined objective with custom completion logic. */
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Character Role - Defines a character's relationship to the player in the story
 *
 * Roles determine how characters behave in cutscenes, what dialogue options
 * are available, and how the relationship system treats them.
 */
UENUM(BlueprintType)
enum class EMGCharacterRole : uint8
{
	/** The player's character representation (rarely used - player is implicit). */
	Player UMETA(DisplayName = "Player"),

	/** Friendly characters who help the player. Can become crew members. */
	Ally UMETA(DisplayName = "Ally"),

	/** Experienced racers who teach the player. Often give tutorial missions. */
	Mentor UMETA(DisplayName = "Mentor"),

	/** Competitive opponents with recurring storylines. Recurring antagonists. */
	Rival UMETA(DisplayName = "Rival"),

	/** Chapter-ending antagonists. Major story obstacles. */
	Boss UMETA(DisplayName = "Boss"),

	/** Information providers (mechanics, dealers, insiders). Mission givers. */
	Contact UMETA(DisplayName = "Contact"),

	/** Romantic subplot characters. May have special missions. */
	LoveInterest UMETA(DisplayName = "Love Interest"),

	/** Main villains who drive the overarching plot. */
	Antagonist UMETA(DisplayName = "Antagonist"),

	/** Generic characters without special story significance. */
	NPC UMETA(DisplayName = "NPC")
};

/**
 * Dialogue Line - A single line of character dialogue
 *
 * Dialogue lines are the building blocks of conversations in the game.
 * They include the text, optional voice-over, timing, and presentation info.
 * The UI system uses this data to display dialogue boxes with character portraits.
 */
USTRUCT(BlueprintType)
struct FMGDialogueLine
{
	GENERATED_BODY()

	/** Which character is speaking (references FMGStoryCharacter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName CharacterID;

	/** The actual dialogue text displayed on screen. Supports localization. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Line;

	/** Optional voice-over audio. TSoftObjectPtr = loaded on demand, saves memory. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	TSoftObjectPtr<USoundBase> VoiceOver;

	/** How long to display this line (seconds). Auto-advance timer or VO length. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	float Duration = 3.0f;

	/** Animation tag for the character (e.g., "Angry", "Happy", "Worried"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName EmotionTag;

	/** If true, display with phone call UI (smaller portrait, phone frame). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	bool bIsPhoneCall = false;
};

/**
 * Story Character - Definition of an NPC in the game's narrative
 *
 * Characters are the people you interact with in Midnight Grind's story.
 * They give missions, appear in cutscenes, race against you, and have
 * dynamic relationships that change based on your actions.
 */
USTRUCT(BlueprintType)
struct FMGStoryCharacter
{
	GENERATED_BODY()

	/** Unique identifier used to reference this character in code/data. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CharacterID;

	/** Full name shown in dialogue and character info screens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FText DisplayName;

	/** Short name or alias (e.g., "Ghost", "The Mechanic"). Often used in radio chatter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FText Nickname;

	/** How this character relates to the player (Ally, Rival, Mentor, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	EMGCharacterRole Role = EMGCharacterRole::NPC;

	/** Main portrait image for dialogue boxes and character info. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TSoftObjectPtr<UTexture2D> Portrait;

	/** Smaller icon used for phone call UI and contact lists. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TSoftObjectPtr<UTexture2D> PhoneIcon;

	/** Background story shown in character bio/dossier screens. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (MultiLine = true))
	FText Bio;

	/** Their iconic car (references vehicle database). Shown in races against them. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName SignatureVehicleID;

	/** Which crew they belong to (references crew system). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	FName CrewID;

	/** Current relationship score (-100 = hostile, 0 = neutral, 100 = best friend). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	int32 RelationshipLevel = 0;

	/** Whether player has "discovered" this character (appears in contacts). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	bool bUnlocked = false;
};

/**
 * Mission Objective - A specific goal within a mission
 *
 * Objectives define what the player must accomplish. Missions typically have
 * one primary objective (required) and several optional objectives (bonus).
 * Progress is tracked using CurrentValue vs TargetValue.
 */
USTRUCT(BlueprintType)
struct FMGMissionObjective
{
	GENERATED_BODY()

	/** Unique ID for this objective within the mission. Used for progress updates. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName ObjectiveID;

	/** Player-facing text (e.g., "Win the race", "Score 50,000 drift points"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FText Description;

	/** What kind of action completes this objective. Determines tracking logic. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	EMGObjectiveType Type = EMGObjectiveType::WinRace;

	/** Goal value for completion (e.g., 1 for "win", 50000 for "drift score"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 TargetValue = 1;

	/** Current progress toward the target. Updated during gameplay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 CurrentValue = 0;

	/** For race objectives: which specific race must be completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetRaceID;

	/** For location objectives: destination point on the map. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetLocationID;

	/** For defeat/tail objectives: which character is the target. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	FName TargetCharacterID;

	/** If true, this is a bonus objective - mission can complete without it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsOptional = false;

	/** If true, don't show this objective until it's revealed (secret objectives). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsHidden = false;

	/** Runtime state: has this objective been completed? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	bool bIsComplete = false;

	/** Extra cash reward for completing this specific objective. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int64 BonusCash = 0;

	/** Extra reputation for completing this specific objective. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Objective")
	int32 BonusREP = 0;
};

/**
 * Mission Reward - What the player earns for completing a mission
 *
 * Rewards are granted when a mission is completed. They can include
 * currency, progression points, and content unlocks. Missions have
 * both a CompletionReward (basic) and PerfectReward (all optionals done).
 */
USTRUCT(BlueprintType)
struct FMGMissionReward
{
	GENERATED_BODY()

	/** In-game currency (GrindCash) awarded. Used to buy cars and parts. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int64 Cash = 0;

	/** Reputation points awarded. Affects your standing and unlocks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 REP = 0;

	/** Experience points for player level progression. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 XP = 0;

	/** If set, unlocks this vehicle for purchase/ownership. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockVehicleID;

	/** If set, unlocks this performance or visual part. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockPartID;

	/** If set, unlocks a new map area for exploration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockAreaID;

	/** If set, unlocks this character (adds to contacts/rivals list). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockCharacterID;

	/** If set, grants this achievement/trophy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName UnlockAchievementID;
};

/**
 * Mission Definition - Complete data for a single mission
 *
 * This is the "blueprint" for a mission - all the data needed to set up,
 * run, and reward a mission. Game designers create these in data assets
 * or Blueprints, and the Campaign subsystem uses them at runtime.
 *
 * Missions are the core gameplay unit of the story campaign. Each one
 * is a self-contained experience with objectives, dialogue, and rewards.
 */
USTRUCT(BlueprintType)
struct FMGMissionDefinition
{
	GENERATED_BODY()

	//-------------------------------------------------------------------------
	// Identity
	//-------------------------------------------------------------------------

	/** Unique ID (e.g., "Mission_Ch1_Race1"). Used to reference this mission. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FName MissionID;

	/** Display name in mission select UI (e.g., "Proving Ground"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FText Title;

	/** Multi-line description explaining the mission's story context. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission", meta = (MultiLine = true))
	FText Description;

	/** What kind of mission this is (Story, Side, Boss, etc.). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	EMGMissionType Type = EMGMissionType::Story;

	/** Which chapter this mission belongs to (1-based). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 ChapterNumber = 1;

	/** Order within the chapter for display purposes. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MissionNumber = 1;

	//-------------------------------------------------------------------------
	// Prerequisites - What must be true before this mission is available
	//-------------------------------------------------------------------------

	/** MissionIDs that must be completed first. Empty = no mission prereqs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	TArray<FName> RequiredMissions;

	/** Minimum reputation level required (0 = no requirement). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredREP = 0;

	/** Minimum player level required (1 = effectively no requirement). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredLevel = 1;

	/** Player must own this vehicle to start (empty = any car is fine). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	FName RequiredVehicleID;

	/** Minimum Performance Index required (0 = no restriction). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredPI = 0;

	//-------------------------------------------------------------------------
	// Content - What happens during the mission
	//-------------------------------------------------------------------------

	/** All objectives for this mission (primary and optional). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGMissionObjective> Objectives;

	/** Dialogue played when mission starts (briefing). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGDialogueLine> IntroDialogue;

	/** Dialogue played upon successful completion (celebration). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGDialogueLine> OutroDialogue;

	/** Dialogue played upon failure (consolation/retry prompt). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	TArray<FMGDialogueLine> FailDialogue;

	/** Which character gives this mission (for portrait/contact display). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FName MissionGiverID;

	/** World location where mission starts (for map marker). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FVector StartLocation = FVector::ZeroVector;

	//-------------------------------------------------------------------------
	// Rewards - What player earns for completion
	//-------------------------------------------------------------------------

	/** Rewards for completing all required objectives. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FMGMissionReward CompletionReward;

	/** BONUS rewards if ALL objectives (including optional) are completed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FMGMissionReward PerfectReward;

	//-------------------------------------------------------------------------
	// Settings - Mission configuration
	//-------------------------------------------------------------------------

	/** Can this mission be replayed after completion? (Most can.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bIsReplayable = true;

	/** Mission time limit in seconds (0 = unlimited). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float TimeLimit = 0.0f;

	/** Are police pursuit mechanics active during this mission? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bPoliceEnabled = true;

	/** Is ambient traffic present during this mission? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	bool bTrafficEnabled = true;

	//-------------------------------------------------------------------------
	// Visuals - UI presentation
	//-------------------------------------------------------------------------

	/** Small icon for mission list and map markers. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> MissionIcon;

	/** Large banner image for mission detail screen. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> MissionBanner;
};

/**
 * Chapter Definition - An "act" of the story campaign
 *
 * Chapters are major story divisions that group related missions together.
 * Each chapter typically introduces a new area, new antagonist, and
 * culminates in a boss battle. Completing a chapter is a major milestone.
 */
USTRUCT(BlueprintType)
struct FMGChapterDefinition
{
	GENERATED_BODY()

	/** Chapter number (1, 2, 3, etc.). Used as the unique identifier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	int32 ChapterNumber = 1;

	/** Display title (e.g., "Chapter 1: The Underground"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FText Title;

	/** Story summary for this chapter (shown in chapter select). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter", meta = (MultiLine = true))
	FText Description;

	/** All missions included in this chapter (references FMGMissionDefinition). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TArray<FName> MissionIDs;

	/** The final "boss" mission that completes this chapter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FName BossMissionID;

	/** The main villain/antagonist for this chapter (references FMGStoryCharacter). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	FName MainAntagonistID;

	/** Map areas that become accessible when this chapter is reached. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TArray<FName> UnlockedAreas;

	/** Key art for chapter title cards and menus. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TSoftObjectPtr<UTexture2D> ChapterArt;

	/** Music theme that plays during this chapter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
	TSoftObjectPtr<USoundBase> ChapterTheme;
};

/**
 * Mission Progress State - Tracks player's progress/history for one mission
 *
 * This is the "save data" for a single mission. It stores whether the mission
 * is completed, how many times it's been attempted, best scores, etc.
 * This gets persisted to the save file.
 */
USTRUCT(BlueprintType)
struct FMGMissionProgress
{
	GENERATED_BODY()

	/** Which mission this progress is for. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FName MissionID;

	/** Current state (Locked, Available, InProgress, Completed, Failed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	EMGMissionStatus Status = EMGMissionStatus::Locked;

	/** Current/best progress on each objective. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	TArray<FMGMissionObjective> ObjectiveProgress;

	/** How many times player has started this mission. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 AttemptCount = 0;

	/** How many times player has successfully completed this mission. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CompletionCount = 0;

	/** Best completion time (for timed missions and leaderboards). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float BestTime = 0.0f;

	/** True if player has completed with all optional objectives at least once. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bPerfectCompletion = false;

	/** When the player first completed this mission (for "on this day" features). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime FirstCompletionTime;

	/** When the player last played this mission (for "continue" features). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime LastAttemptTime;
};

/**
 * =============================================================================
 * DELEGATE DECLARATIONS
 * =============================================================================
 *
 * Delegates are Unreal's event system. They allow other parts of the game
 * (especially UI widgets and other subsystems) to "subscribe" to notifications
 * when something happens in the Campaign system.
 *
 * Usage example in Blueprint or C++:
 *   CampaignSubsystem->OnMissionCompleted.AddDynamic(this, &MyClass::HandleMissionComplete);
 *
 * DYNAMIC_MULTICAST means:
 * - DYNAMIC: Can bind to Blueprint functions
 * - MULTICAST: Multiple listeners can subscribe
 */

/** Broadcast when a mission begins. UI should show mission HUD. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, FName, MissionID);

/** Broadcast when mission ends successfully. bPerfect = all optionals completed. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionCompleted, FName, MissionID, bool, bPerfect);

/** Broadcast when mission ends in failure. UI should show failure screen. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionFailed, FName, MissionID);

/** Broadcast when objective progress changes. UI should update objective tracker. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, MissionID, FName, ObjectiveID);

/** Broadcast when a specific objective is completed. Play celebration effect. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, FName, MissionID, FName, ObjectiveID);

/** Broadcast when a new chapter becomes available. Major story moment! */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChapterUnlocked, int32, ChapterNumber);

/** Broadcast when all missions in a chapter are done. Show chapter complete screen. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChapterCompleted, int32, ChapterNumber);

/** Broadcast when dialogue sequence begins. UI should display dialogue widget. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const TArray<FMGDialogueLine>&, Dialogue);

/** Broadcast when dialogue sequence ends. UI should hide dialogue widget. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

/** Broadcast when relationship with a character changes. Update character UI. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterRelationshipChanged, FName, CharacterID, int32, NewLevel);

/** Broadcast when a locked mission becomes available. Show map notification. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionAvailable, FName, MissionID);

/**
 * =============================================================================
 * UMGCampaignSubsystem - Main Campaign/Story Management Class
 * =============================================================================
 *
 * This is the central subsystem that manages the game's story campaign.
 * It handles mission lifecycle, objective tracking, character relationships,
 * dialogue playback, and overall story progression.
 *
 * As a UGameInstanceSubsystem:
 * - Automatically created when game starts (no manual instantiation needed)
 * - Access via: GetGameInstance()->GetSubsystem<UMGCampaignSubsystem>()
 * - Persists across level loads (survives map changes)
 * - Only one instance exists at a time
 *
 * The class is organized into functional sections:
 * - Events/Delegates: For other systems to listen to campaign events
 * - Registration: For loading mission/chapter/character data
 * - Mission Management: Starting, completing, failing missions
 * - Objectives: Tracking and updating objective progress
 * - Chapters: Story progression through campaign acts
 * - Characters: NPC relationships and information
 * - Dialogue: Conversation playback system
 * - Progress/Stats: Overall campaign completion tracking
 * - Save/Load: Persistence of player progress
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

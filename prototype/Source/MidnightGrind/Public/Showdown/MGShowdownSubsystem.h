// Copyright Epic Games, Inc. All Rights Reserved.

/**
 * =============================================================================
 * MGShowdownSubsystem.h
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the Showdown Subsystem, which manages boss battles, story
 * finale events, and epic 1-on-1 confrontations in Midnight Grind. Unlike regular
 * races or multiplayer tournaments, showdowns are dramatic, story-driven encounters
 * against powerful AI opponents (bosses) with special mechanics and phases.
 *
 * WHAT IS A SHOWDOWN?
 * -------------------
 * A showdown is a climactic racing encounter that combines:
 *   - Boss characters with unique personalities and vehicles
 *   - Multiple phases (introduction, chase, race, battle, finale)
 *   - Special modifiers and challenges
 *   - Checkpoint systems for longer encounters
 *   - Unique rewards (vehicles, parts, titles)
 *   - Story dialogue and cinematics
 *
 * Think of showdowns as the "boss fights" of a racing game - they're the
 * memorable encounters that punctuate your career progression.
 *
 * KEY CONCEPTS AND TERMINOLOGY:
 * -----------------------------
 *
 * 1. SHOWDOWN TYPES (EMGShowdownType):
 *    - BossRace: A race against a boss character
 *    - FinalShowdown: End of a story chapter
 *    - RivalBattle: Recurring rival encounters
 *    - Championship: End-of-season finale
 *    - GauntletFinale: End of a gauntlet challenge series
 *    - LegendChallenge: Face legendary retired racers
 *    - StreakDefense: Defend your winning streak
 *    - TerritoryWar: Fight for territory control
 *    - CommunityBoss: Limited-time community events
 *    - SeasonFinale: Seasonal championship conclusion
 *
 * 2. PHASES (EMGBossPhaseType):
 *    Showdowns are divided into phases, each with different gameplay:
 *    - Introduction: Cinematic/dialogue before the action
 *    - ChasePhase: Catch up to or escape from the boss
 *    - RacePhase: Traditional racing competition
 *    - BattlePhase: Combat-style interaction (ramming, blocking)
 *    - FinalPhase: Climactic final challenge
 *    - Escape: Get away after victory
 *
 * 3. BOSS ENCOUNTER:
 *    The AI opponent with:
 *    - Unique identity (name, backstory, title)
 *    - Signature vehicle
 *    - Skill level and aggression settings
 *    - Special abilities and weaknesses
 *    - Phase-specific dialogue
 *
 * 4. MODIFIERS (EMGShowdownModifier):
 *    Special conditions that change the showdown:
 *    - NoNitro: Can't use boost
 *    - RubberBanding: AI stays competitive regardless of skill
 *    - AggresiveAI: Boss plays dirty
 *    - LimitedTime: Must finish within time limit
 *    - PolicePursuit: Cops join the chase
 *    - WeatherHazard: Rain, snow, or fog
 *
 * 5. CHECKPOINTS:
 *    Save points within long showdowns so you don't restart from the
 *    beginning if you fail. RetryShowdown(bFromCheckpoint) uses these.
 *
 * 6. BOSS HEALTH:
 *    Some showdowns feature a health bar for the boss that depletes
 *    through various actions (ramming, winning phases, etc.).
 *
 * HOW IT FITS INTO THE GAME ARCHITECTURE:
 * ---------------------------------------
 * - This is a UGameInstanceSubsystem, persisting across level loads
 * - Works with the Story/Progression system to unlock showdowns
 * - Showdown rewards feed into the player's inventory and garage
 * - Results update player stats and can trigger achievements
 * - Independent from multiplayer/tournament systems (PvE focused)
 *
 * TYPICAL SHOWDOWN WORKFLOW:
 * --------------------------
 * 1. Check available showdowns (GetAvailableShowdowns)
 * 2. Select and start a showdown (StartShowdown)
 * 3. Play through phases (OnPhaseStarted, OnPhaseCompleted)
 * 4. Damage boss or meet phase objectives (DamageBoss)
 * 5. Reach checkpoints for safety (SetCheckpoint)
 * 6. Complete or fail the showdown (CompleteShowdown, FailShowdown)
 * 7. Retry from checkpoint if failed (RetryShowdown)
 * 8. Receive rewards on completion (FMGShowdownResult)
 *
 * DATA STRUCTURES:
 * ----------------
 * - FMGShowdownDefinition: The template for a showdown (static data)
 * - FMGActiveShowdown: A player's current instance (runtime state)
 * - FMGShowdownResult: Outcome after completion
 * - FMGBossPhaseDefinition: Configuration for each phase
 * - FMGBossEncounter: Boss character data
 * - FMGShowdownRecord: Personal best times and completion stats
 * - FMGShowdownPlayerStats: Aggregate stats across all showdowns
 *
 * DELEGATES (Event Notifications):
 * --------------------------------
 * - OnShowdownStarted: Player begins a showdown
 * - OnShowdownCompleted: Player successfully completes
 * - OnShowdownFailed: Player fails or abandons
 * - OnPhaseStarted/Completed: Phase transitions
 * - OnBossHealthChanged: Boss takes damage
 * - OnCheckpointReached: Safety checkpoint hit
 * - OnShowdownUnlocked: New showdown becomes available
 * - OnBossDefeated: A boss is beaten (for achievements)
 * - OnNewShowdownRecord: Personal best time achieved
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ---------------------------
 * - TSet: Unreal's hash set (unique values, fast lookup)
 * - TSoftObjectPtr: Reference to an asset that may not be loaded yet
 * - FTimerHandle: Handle to a scheduled timer callback
 * - FDateTime: Date and time value
 * - GENERATED_BODY(): Macro that adds Unreal reflection boilerplate
 * - UMETA(DisplayName): Custom name shown in editor/Blueprints
 *
 * PERSISTENCE:
 * ------------
 * The subsystem saves/loads:
 * - Which showdowns are unlocked/completed
 * - Personal best times (records)
 * - Aggregate player stats
 * - Active showdown state (for resume after crash)
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGShowdownSubsystem.generated.h"

//=============================================================================
// ENUMS - Showdown Configuration Types
//=============================================================================

/**
 * Showdown Type
 *
 * Different categories of showdown events, each with unique context and rewards.
 * The type helps determine presentation, difficulty scaling, and unlocks.
 *
 * Story/Career Related:
 * - BossRace: Standard boss fight at end of a story section
 * - FinalShowdown: Climactic end-of-chapter confrontation
 * - RivalBattle: Recurring encounters with your rival throughout the story
 *
 * Competition Related:
 * - Championship: End of a ranked season
 * - GauntletFinale: Final boss after completing a gauntlet challenge
 *
 * Special Events:
 * - LegendChallenge: Race against legendary retired racers (Easter eggs)
 * - StreakDefense: Defend your winning streak against a challenger
 * - TerritoryWar: Fight for control of a territory
 * - CommunityBoss: Limited-time community events with special rewards
 * - SeasonFinale: Seasonal story conclusion with major rewards
 */
UENUM(BlueprintType)
enum class EMGShowdownType : uint8
{
	None				UMETA(DisplayName = "None"),
	BossRace			UMETA(DisplayName = "Boss Race"),
	FinalShowdown		UMETA(DisplayName = "Final Showdown"),
	RivalBattle			UMETA(DisplayName = "Rival Battle"),
	Championship		UMETA(DisplayName = "Championship Final"),
	GauntletFinale		UMETA(DisplayName = "Gauntlet Finale"),
	LegendChallenge		UMETA(DisplayName = "Legend Challenge"),
	StreakDefense		UMETA(DisplayName = "Streak Defense"),
	TerritoryWar		UMETA(DisplayName = "Territory War"),
	CommunityBoss		UMETA(DisplayName = "Community Boss"),
	SeasonFinale		UMETA(DisplayName = "Season Finale")
};

/**
 * Showdown Status
 *
 * The current state of a showdown for a specific player.
 * Determines UI display and available actions.
 *
 * - Locked: Not yet unlocked. Requirements not met (story progress, level, etc.)
 * - Available: Unlocked and ready to attempt. "Start Showdown" enabled.
 * - InProgress: Currently being played. Can pause, retry, or abandon.
 * - Completed: Successfully finished. Rewards claimed. May be replayable.
 * - Failed: Player lost/abandoned. Can retry if attempts remain.
 * - Expired: Time-limited showdown that is no longer available.
 */
UENUM(BlueprintType)
enum class EMGShowdownStatus : uint8
{
	Locked				UMETA(DisplayName = "Locked"),
	Available			UMETA(DisplayName = "Available"),
	InProgress			UMETA(DisplayName = "In Progress"),
	Completed			UMETA(DisplayName = "Completed"),
	Failed				UMETA(DisplayName = "Failed"),
	Expired				UMETA(DisplayName = "Expired")
};

/**
 * Showdown Difficulty
 *
 * Difficulty tiers that affect boss AI, timing requirements, and rewards.
 * Higher difficulties = tougher challenges but better rewards.
 *
 * Scaling (approximate):
 * - Normal: Standard challenge. Expected win rate ~70%
 * - Hard: Tougher AI, tighter margins. Expected win rate ~50%
 * - Extreme: Very challenging. For skilled players. ~30%
 * - Nightmare: Near-perfect execution required. ~10%
 * - Impossible: Bragging rights tier. <5% expected. Massive rewards.
 *
 * Difficulty affects:
 * - Boss skill level and aggression
 * - Time limits (if applicable)
 * - Margin for error
 * - Reward multipliers
 */
UENUM(BlueprintType)
enum class EMGShowdownDifficulty : uint8
{
	Normal				UMETA(DisplayName = "Normal"),
	Hard				UMETA(DisplayName = "Hard"),
	Extreme				UMETA(DisplayName = "Extreme"),
	Nightmare			UMETA(DisplayName = "Nightmare"),
	Impossible			UMETA(DisplayName = "Impossible")
};

/**
 * Boss Phase Type
 *
 * Showdowns are divided into distinct phases, each with different gameplay.
 * This creates variety and dramatic pacing in boss encounters.
 *
 * Phase Types:
 * - Introduction: Cinematic intro, dialogue, boss reveals themselves.
 *   Usually not interactive - sets the scene.
 *
 * - ChasePhase: Catch up to the boss (or escape from them).
 *   Often has a distance meter you need to close.
 *
 * - RacePhase: Traditional racing. First to finish wins.
 *   The core gameplay of most showdowns.
 *
 * - BattlePhase: Vehicular combat. Ram, block, damage the boss.
 *   May involve boss "health" that you whittle down.
 *
 * - FinalPhase: Climactic final challenge. Often harder than earlier phases.
 *   This is where the boss "gets serious."
 *
 * - Escape: Victory achieved, now get away. A cooldown phase.
 *   May involve evading police or environmental hazards.
 */
UENUM(BlueprintType)
enum class EMGBossPhaseType : uint8
{
	Introduction		UMETA(DisplayName = "Introduction"),
	ChasePhase			UMETA(DisplayName = "Chase Phase"),
	RacePhase			UMETA(DisplayName = "Race Phase"),
	BattlePhase			UMETA(DisplayName = "Battle Phase"),
	FinalPhase			UMETA(DisplayName = "Final Phase"),
	Escape				UMETA(DisplayName = "Escape")
};

/**
 * Showdown Modifier
 *
 * Special rules or conditions that modify the showdown experience.
 * Multiple modifiers can be active at once to create unique challenges.
 *
 * Modifiers:
 * - None: Standard rules.
 * - NoNitro: Nitro boost disabled. Tests pure driving skill.
 * - RubberBanding: AI stays competitive regardless of skill gap.
 *   Keeps the race exciting but can feel unfair.
 * - AggressiveAI: Boss uses dirty tactics (ramming, blocking).
 * - LimitedTime: Must complete within a time limit.
 * - NoDamage: Any collision = failure. Precision required.
 * - PolicePursuit: Cops join the chase, complicating things.
 * - WeatherHazard: Rain, snow, or fog affecting visibility/grip.
 * - NightOnly: Takes place at night with limited visibility.
 *
 * Modifiers increase difficulty but often boost rewards.
 */
UENUM(BlueprintType)
enum class EMGShowdownModifier : uint8
{
	None				UMETA(DisplayName = "None"),
	NoNitro				UMETA(DisplayName = "No Nitro"),
	RubberBanding		UMETA(DisplayName = "Rubber Banding"),
	AggresiveAI			UMETA(DisplayName = "Aggressive AI"),
	LimitedTime			UMETA(DisplayName = "Limited Time"),
	NoDamage			UMETA(DisplayName = "No Damage"),
	PolicePursuit		UMETA(DisplayName = "Police Pursuit"),
	WeatherHazard		UMETA(DisplayName = "Weather Hazard"),
	NightOnly			UMETA(DisplayName = "Night Only")
};

//=============================================================================
// STRUCTS - Showdown Data Containers
//=============================================================================

/**
 * Showdown Definition
 *
 * The "blueprint" for a showdown - static data that defines what it is.
 * This is configured by designers and doesn't change during gameplay.
 *
 * Contains:
 * - Identity: ID, name, description, dialogue
 * - Boss Info: Who you're facing, their vehicle, skill level
 * - Structure: Phases, modifiers, track
 * - Requirements: Level, story progress, prerequisites
 * - Rewards: Currency, XP, reputation, items, vehicles, titles
 * - Settings: Repeatability, time limits, attempt limits
 * - Assets: Portraits, banners, music
 */
USTRUCT(BlueprintType)
struct FMGShowdownDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShowdownId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText IntroDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VictoryDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DefeatDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShowdownType Type = EMGShowdownType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShowdownDifficulty Difficulty = EMGShowdownDifficulty::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BossId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BossVehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BossSkillLevel = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PhaseIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGShowdownModifier> Modifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TrackId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RequiredStoryProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RequiredCompletedShowdowns;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardCurrency = 100000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardExperience = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RewardReputation = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RewardVehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RewardPartId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RewardTitleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRepeatable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAttempts = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ExpirationHours = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> BossPortraitAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> ShowdownBannerAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> MusicAsset;
};

/**
 * Boss Phase Definition
 *
 * Defines a single phase within a showdown. Showdowns are made up of
 * multiple sequential phases, each with different objectives.
 *
 * Key properties:
 * - PhaseId/PhaseName: Identification and display
 * - Type: What kind of gameplay (chase, race, battle, etc.)
 * - Objective: What the player needs to do (display in UI)
 * - Duration: Time limit for this phase (0 = no limit)
 * - Boss Settings: Health %, speed multiplier, aggression level
 * - PhaseModifiers: Modifiers specific to this phase
 * - TransitionDialogue: Boss dialogue when phase changes
 * - CheckpointIndex: Save point at the start of this phase
 * - bCanBeSkipped: Allow skipping (for cinematics)
 */
USTRUCT(BlueprintType)
struct FMGBossPhaseDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PhaseId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText PhaseName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBossPhaseType Type = EMGBossPhaseType::RacePhase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PhaseNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Objective;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BossHealthPercent = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BossSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BossAggressionLevel = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGShowdownModifier> PhaseModifiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TransitionDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanBeSkipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CheckpointIndex = -1;
};

/**
 * Active Showdown
 *
 * Runtime state for a showdown that a player is currently attempting.
 * Created when a player starts a showdown, destroyed on completion/abandonment.
 *
 * This is the "instance" vs the Definition being the "template."
 * Tracks dynamic information that changes during gameplay:
 * - Current phase progress
 * - Time elapsed
 * - Attempts used
 * - Checkpoint reached
 * - Boss health remaining
 * - Player score
 * - Phase completion status
 */
USTRUCT(BlueprintType)
struct FMGActiveShowdown
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InstanceId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShowdownId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGShowdownStatus Status = EMGShowdownStatus::Available;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPhaseIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CurrentPhaseId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhaseTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptsUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LastCheckpoint = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BossHealthRemaining = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlayerScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, bool> PhaseCompletions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpirationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFirstAttempt = true;
};

/**
 * Showdown Result
 *
 * The outcome of a completed showdown attempt. Contains all data needed
 * for the results screen, statistics, and reward distribution.
 *
 * Captures:
 * - Outcome: Victory or defeat
 * - Performance: Phases completed, time, attempts, score
 * - Rewards: Currency, XP, reputation, items earned
 * - Achievements: Perfect run, first completion, new record
 * - Metadata: Timestamp for history
 */
USTRUCT(BlueprintType)
struct FMGShowdownResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShowdownId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVictory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PhasesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPhases = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AttemptsUsed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FinalScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrencyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReputationEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleRewardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PartRewardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TitleRewardId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPerfectRun = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFirstCompletion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bNewRecord = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Boss Encounter
 *
 * Defines a boss character that can appear in showdowns.
 * Bosses are memorable antagonists with personality and backstory.
 *
 * Contains:
 * - Identity: ID, name, title (e.g., "The Midnight King")
 * - Backstory: Why they're important, their history
 * - Vehicle: Their signature car
 * - Skill: Base skill level and aggression
 * - Abilities: Special abilities they can use (e.g., "EMP Burst")
 * - Weaknesses: Exploitable weaknesses for skilled players
 * - Dialogue: Phase-specific taunts and reactions
 * - Assets: Portrait and vehicle 3D model references
 */
USTRUCT(BlueprintType)
struct FMGBossEncounter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BossId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText BossName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Backstory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VehicleId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseSkillLevel = 80;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionFactor = 0.7f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SpecialAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Weaknesses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FText> PhaseDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> PortraitAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VehicleAsset;
};

/**
 * Showdown Player Stats
 *
 * Aggregate statistics tracking a player's showdown career.
 * Used for profile displays, achievements, and matchmaking difficulty.
 *
 * Tracked metrics:
 * - Attempts: Total showdowns attempted
 * - Success: Completions vs failures
 * - Performance: Perfect runs, bosses defeated, fastest times
 * - Progression: Total retries (measure of perseverance)
 * - Earnings: Total currency from showdowns
 * - Breakdowns: Completions by type and difficulty
 * - Records: Best times per showdown
 * - Unlocks: Which bosses have been encountered
 */
USTRUCT(BlueprintType)
struct FMGShowdownPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalShowdownsAttempted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalShowdownsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalShowdownsFailed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerfectRuns = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BossesDefeated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FastestBossDefeat = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRetries = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 TotalCurrencyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGShowdownType, int32> CompletionsByType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGShowdownDifficulty, int32> CompletionsByDifficulty;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, float> BestTimesByShowdown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> UnlockedBosses;
};

/**
 * Showdown Record
 *
 * Personal best and historical data for a specific showdown.
 * Enables leaderboard competition and "beat your best" gameplay.
 *
 * Contains:
 * - Personal bests: Best time and best score
 * - Completion stats: Times completed, total attempts
 * - Achievements: Perfect run achieved or not
 * - Leaderboard: World record time and holder
 * - History: First completion date, best time date
 */
USTRUCT(BlueprintType)
struct FMGShowdownRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ShowdownId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PersonalBestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PersonalBestScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalAttempts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPerfectRunAchieved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WorldRecordTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WorldRecordHolder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime FirstCompletionDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BestTimeDate;
};

//=============================================================================
// DELEGATES - Event Notifications
//=============================================================================

/**
 * Delegates for showdown events. Subscribe to these to react to gameplay events.
 * In Blueprints, these appear in the Events section of the subsystem.
 */

/** Fired when a player begins a showdown. Use for analytics, UI updates. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShowdownStarted, const FString&, PlayerId, const FString&, ShowdownId, EMGShowdownType, Type);

/** Fired on successful completion. Contains all result data for rewards/display. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowdownCompleted, const FString&, PlayerId, const FMGShowdownResult&, Result);

/** Fired when player fails or abandons. PhaseReached shows how far they got. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShowdownFailed, const FString&, PlayerId, const FString&, ShowdownId, int32, PhaseReached);

/** Fired when entering a new phase. Update UI, play transition effects. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPhaseStarted, const FString&, PlayerId, const FString&, ShowdownId, int32, PhaseIndex, const FString&, PhaseId);

/** Fired when a phase is completed. PhaseTime useful for records. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPhaseCompleted, const FString&, PlayerId, const FString&, ShowdownId, int32, PhaseIndex, float, PhaseTime);

/** Fired when boss takes damage. Update boss health bar UI. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossHealthChanged, const FString&, ShowdownId, int32, CurrentHealth, int32, MaxHealth);

/** Fired when player reaches a checkpoint. Update checkpoint indicator. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointReached, const FString&, PlayerId, const FString&, ShowdownId, int32, CheckpointIndex);

/** Fired when a new showdown becomes available. Notify player in UI. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowdownUnlocked, const FString&, PlayerId, const FString&, ShowdownId);

/** Fired when a boss is defeated. For achievements, stat tracking. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossDefeated, const FString&, PlayerId, const FString&, BossId, float, DefeatTime);

/** Fired when player sets a new personal best time. Celebration moment! */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewShowdownRecord, const FString&, PlayerId, const FString&, ShowdownId, float, NewBestTime);

//=============================================================================
// SUBSYSTEM CLASS
//=============================================================================

/**
 * Showdown Subsystem
 *
 * The main subsystem managing boss battles, finale events, and epic showdowns.
 * This is a GameInstanceSubsystem, meaning one instance persists for the game session.
 *
 * Access via: GetGameInstance()->GetSubsystem<UMGShowdownSubsystem>()
 *
 * Key Responsibilities:
 * - Registration of showdown definitions, phases, and bosses
 * - Starting, progressing, and completing showdowns
 * - Phase management and checkpoint handling
 * - Boss interactions (damage, health, aggression)
 * - Record tracking and leaderboards
 * - Player statistics aggregation
 * - Unlock requirement checking
 * - Persistence (save/load showdown progress)
 *
 * This subsystem is PvE (Player vs Environment) focused, unlike the tournament
 * and esports systems which are PvP (Player vs Player).
 */
UCLASS()
class MIDNIGHTGRIND_API UMGShowdownSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Initialize subsystem, load saved data, set up tick timer. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Clean up timers and save any pending data. */
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnShowdownStarted OnShowdownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnShowdownCompleted OnShowdownCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnShowdownFailed OnShowdownFailed;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnPhaseStarted OnPhaseStarted;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnPhaseCompleted OnPhaseCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnBossHealthChanged OnBossHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnCheckpointReached OnCheckpointReached;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnShowdownUnlocked OnShowdownUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnBossDefeated OnBossDefeated;

	UPROPERTY(BlueprintAssignable, Category = "Showdown|Events")
	FOnNewShowdownRecord OnNewShowdownRecord;

	// Registration
	UFUNCTION(BlueprintCallable, Category = "Showdown|Registration")
	void RegisterShowdown(const FMGShowdownDefinition& Showdown);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Registration")
	void RegisterPhase(const FMGBossPhaseDefinition& Phase);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Registration")
	void RegisterBoss(const FMGBossEncounter& Boss);

	// Showdown Actions
	UFUNCTION(BlueprintCallable, Category = "Showdown|Actions")
	bool StartShowdown(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Actions")
	void AbandonShowdown(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Actions")
	FMGShowdownResult CompleteShowdown(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Actions")
	void FailShowdown(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Actions")
	bool RetryShowdown(const FString& PlayerId, const FString& ShowdownId, bool bFromCheckpoint = false);

	// Phase Management
	UFUNCTION(BlueprintCallable, Category = "Showdown|Phase")
	void AdvancePhase(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Phase")
	void CompleteCurrentPhase(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Phase")
	void SetCheckpoint(const FString& PlayerId, const FString& ShowdownId, int32 CheckpointIndex);

	UFUNCTION(BlueprintPure, Category = "Showdown|Phase")
	FMGBossPhaseDefinition GetCurrentPhase(const FString& PlayerId, const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Phase")
	int32 GetCurrentPhaseIndex(const FString& PlayerId, const FString& ShowdownId) const;

	// Boss Interactions
	UFUNCTION(BlueprintCallable, Category = "Showdown|Boss")
	void DamageBoss(const FString& ShowdownId, int32 Damage);

	UFUNCTION(BlueprintPure, Category = "Showdown|Boss")
	int32 GetBossHealth(const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Boss")
	FMGBossEncounter GetBoss(const FString& BossId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Boss")
	float GetBossAggressionLevel(const FString& ShowdownId) const;

	// Queries
	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	FMGShowdownDefinition GetShowdownDefinition(const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	FMGActiveShowdown GetActiveShowdown(const FString& PlayerId, const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	TArray<FMGShowdownDefinition> GetAvailableShowdowns(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	TArray<FMGShowdownDefinition> GetShowdownsByType(EMGShowdownType Type) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	bool IsShowdownUnlocked(const FString& PlayerId, const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	bool IsShowdownCompleted(const FString& PlayerId, const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Query")
	bool HasActiveShowdown(const FString& PlayerId) const;

	// Records
	UFUNCTION(BlueprintPure, Category = "Showdown|Records")
	FMGShowdownRecord GetShowdownRecord(const FString& PlayerId, const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Records")
	float GetPersonalBestTime(const FString& PlayerId, const FString& ShowdownId) const;

	UFUNCTION(BlueprintPure, Category = "Showdown|Records")
	float GetWorldRecordTime(const FString& ShowdownId) const;

	UFUNCTION(BlueprintCallable, Category = "Showdown|Records")
	void SetWorldRecord(const FString& ShowdownId, float Time, const FString& PlayerName);

	// Stats
	UFUNCTION(BlueprintPure, Category = "Showdown|Stats")
	FMGShowdownPlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Showdown|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Unlocks
	UFUNCTION(BlueprintCallable, Category = "Showdown|Unlock")
	void UnlockShowdown(const FString& PlayerId, const FString& ShowdownId);

	UFUNCTION(BlueprintCallable, Category = "Showdown|Unlock")
	void CheckUnlockRequirements(const FString& PlayerId);

	// Update
	UFUNCTION(BlueprintCallable, Category = "Showdown|Update")
	void UpdateShowdownSystem(float DeltaTime);

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "Showdown|Persistence")
	void SaveShowdownData();

	UFUNCTION(BlueprintCallable, Category = "Showdown|Persistence")
	void LoadShowdownData();

protected:
	void TickShowdowns(float DeltaTime);
	void UpdateActiveShowdowns(float DeltaTime);
	void UpdatePlayerStats(const FString& PlayerId, const FMGShowdownResult& Result);
	void UpdateRecords(const FString& PlayerId, const FString& ShowdownId, const FMGShowdownResult& Result);
	bool CheckShowdownRequirements(const FString& PlayerId, const FMGShowdownDefinition& Showdown) const;
	float GetDifficultyMultiplier(EMGShowdownDifficulty Difficulty) const;
	FString GenerateInstanceId() const;

private:
	UPROPERTY()
	TMap<FString, FMGShowdownDefinition> ShowdownDefinitions;

	UPROPERTY()
	TMap<FString, FMGBossPhaseDefinition> PhaseDefinitions;

	UPROPERTY()
	TMap<FString, FMGBossEncounter> BossEncounters;

	UPROPERTY()
	TMap<FString, FMGActiveShowdown> ActiveShowdowns;

	UPROPERTY()
	TMap<FString, TSet<FString>> UnlockedShowdowns;

	UPROPERTY()
	TMap<FString, TSet<FString>> CompletedShowdowns;

	UPROPERTY()
	TMap<FString, TMap<FString, FMGShowdownRecord>> PlayerRecords;

	UPROPERTY()
	TMap<FString, FMGShowdownPlayerStats> PlayerStats;

	UPROPERTY()
	TMap<FString, float> WorldRecords;

	UPROPERTY()
	TMap<FString, FString> WorldRecordHolders;

	UPROPERTY()
	int32 InstanceCounter = 0;

	FTimerHandle ShowdownTickTimer;
};

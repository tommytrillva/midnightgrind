// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGDynamicDifficultySubsystem.h
 * @brief Dynamic Difficulty Adjustment (DDA) - Adaptive AI and challenge scaling system
 * @author Midnight Grind Team
 * @version 1.0
 *
 * @section overview Overview
 * ============================================================================
 * MGDynamicDifficultySubsystem.h
 * Midnight Grind - Dynamic Difficulty Adjustment (DDA) System
 * ============================================================================
 *
 * This subsystem implements Dynamic Difficulty Adjustment (DDA) for Midnight Grind.
 * DDA is a technique used in modern games to automatically adjust the game's
 * challenge level based on the player's performance. The goal is to keep players
 * in the "flow state" - challenged enough to stay engaged, but not so frustrated
 * that they want to quit.
 *
 * This is one of the most important systems for player retention and satisfaction
 * in a racing game!
 *
 * @section features What This System Does
 *
 * @subsection tracking 1. Tracks Player Performance
 * - Win rate, podium finishes, DNFs (Did Not Finish)
 * - Lap times and racing line efficiency
 * - Drift scores, overtakes, and collision frequency
 * - Nitro usage patterns and shortcut discovery
 *
 * @subsection adjusting 2. Adjusts Game Difficulty
 * - AI opponent speed and aggression
 * - Rubber-banding intensity (AI catching up when behind)
 * - Traffic and obstacle density
 * - Catch-up assist strength for the player
 *
 * @subsection assists 3. Manages Driving Assists
 * - Steering, braking, and drift assists
 * - Racing line display
 * - Collision avoidance hints
 * - Auto-recovery and rewind features
 *
 * @subsection frustration 4. Detects Player Frustration
 * - Monitors patterns that suggest the player is struggling
 * - Automatically responds with helpful adjustments
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection rubberbanding 1. Rubber-Banding
 * A controversial but common racing game technique where AI opponents slow
 * down when ahead and speed up when behind to keep races close and exciting.
 * Too much rubber-banding feels unfair; too little makes races boring.
 * This system lets you fine-tune the intensity.
 *
 * @subsection catchup 2. Catch-Up Assist
 * The opposite of rubber-banding - helps the PLAYER catch up when behind.
 * Can include speed boosts, better slipstream effects, or AI making mistakes.
 * Important for new players who might fall too far behind to recover.
 *
 * @subsection skill 3. Skill Level Estimation
 * The system estimates player skill from Beginner to Legend based on their
 * performance metrics. This helps match them against appropriate AI opponents
 * and suggest suitable difficulty settings.
 *
 * @subsection adaptation 4. Adaptation Speed
 * How quickly the system responds to player performance. "Instant" might feel
 * jarring (sudden difficulty spikes), while "Gradual" provides smoother but
 * slower adjustments. Different players prefer different speeds.
 *
 * @subsection aiprofiles 5. AI Behavior Profiles
 * AI opponents can have different "personalities":
 * - Defensive: Blocks overtakes, races conservatively
 * - Aggressive: Takes risks, rams other racers
 * - Tactical: Makes strategic decisions based on race position
 * - TrainingWheel: Intentionally makes mistakes to help new players win
 *
 * @subsection frustrationdetect 6. Frustration Detection
 * The system watches for signs of frustration:
 * - Multiple consecutive losses
 * - Frequent collisions
 * - Restarting races repeatedly
 * - Long times in last place
 * When detected, it can automatically make the game easier.
 *
 * @section datastructs Data Structures Overview
 * - FPlayerPerformanceData: Complete history of player performance metrics
 * - FDifficultyModifiers: All the knobs that control game difficulty
 * - FAIOpponentSettings: Configuration for a single AI racer
 * - FAssistSettings: Player assist options (steering help, etc.)
 * - FRaceAnalysis: Detailed breakdown of a single race's performance
 * - FDifficultyAdjustment: Record of a single difficulty change
 * - FDifficultyProfile: Complete difficulty preset configuration
 *
 * @section usage Usage Examples
 *
 * @code
 * // Get the subsystem
 * UMGDynamicDifficultySubsystem* DDA = GetGameInstance()->GetSubsystem<UMGDynamicDifficultySubsystem>();
 *
 * // === DIFFICULTY PRESETS ===
 * // Set a difficulty preset
 * DDA->SetDifficultyPreset(EDifficultyPreset::Normal);
 *
 * // Enable adaptive difficulty (auto-adjusts based on performance)
 * DDA->EnableAdaptiveDifficulty(true);
 * DDA->SetAdaptationSpeed(EAdaptationSpeed::Medium);
 *
 * // === RECORDING RACE RESULTS ===
 * // After a race, record the results for tracking
 * FRaceAnalysis Analysis;
 * Analysis.FinalPosition = 3;
 * Analysis.BestLapTime = 75.5f;
 * Analysis.TotalCollisions = 5;
 * Analysis.OvertakesMade = 8;
 * Analysis.TotalDriftScore = 150000.0f;
 * // ... fill in other fields
 * DDA->RecordRaceResult(Analysis);
 *
 * // === AI OPPONENT GENERATION ===
 * // Generate AI opponents calibrated to player skill
 * TArray<FAIOpponentSettings> Opponents = DDA->GenerateOpponentGrid(7);
 *
 * // Generate a specific opponent
 * FAIOpponentSettings BossRival = DDA->GenerateOpponentSettings(ESkillLevel::Expert, EAIBehaviorProfile::Aggressive);
 *
 * // === FRUSTRATION DETECTION ===
 * // Check if player needs help
 * EFrustrationLevel Frustration = DDA->GetCurrentFrustrationLevel();
 * if (Frustration >= EFrustrationLevel::Frustrated)
 * {
 *     // Maybe show a "Would you like to lower the difficulty?" prompt
 *     ShowDifficultyAssistUI();
 * }
 *
 * // Enable automatic frustration response
 * DDA->SetFrustrationResponseEnabled(true);
 *
 * // === DRIVING ASSISTS ===
 * // Enable/disable individual assists
 * DDA->ToggleAssist(EAssistType::Steering, true);
 * DDA->ToggleAssist(EAssistType::RacingLine, true);
 * DDA->SetAssistStrength(EAssistType::Braking, 0.5f);
 *
 * // Get suggested assists for skill level
 * DDA->SuggestAssistsForSkillLevel(ESkillLevel::Beginner);
 *
 * // === REAL-TIME ADJUSTMENTS (during race) ===
 * // Update race progress for real-time catch-up calculations
 * DDA->UpdateRaceProgress(PlayerPosition, GapToLeader, RaceProgressPercent);
 *
 * // Record collisions for frustration tracking
 * DDA->OnPlayerCollision(ImpactSeverity);
 *
 * // Get dynamic catch-up boost (for player behind)
 * float CatchUpBoost = DDA->GetDynamicCatchUpBoost();
 *
 * // Get AI slowdown factor (for rubber-banding)
 * float AISlowdown = DDA->GetDynamicAISlowdown();
 *
 * // === PLAYER STATS ===
 * // Get player performance data
 * FPlayerPerformanceData Performance = DDA->GetPlayerPerformance();
 * ESkillLevel Skill = DDA->GetEstimatedSkillLevel();
 * float Trend = DDA->GetPerformanceTrend();  // Positive = improving
 *
 * // === EVENT LISTENERS ===
 * DDA->OnDifficultyChanged.AddDynamic(this, &UMyClass::HandleDifficultyChange);
 * DDA->OnSkillLevelChanged.AddDynamic(this, &UMyClass::HandleSkillChange);
 * DDA->OnFrustrationDetected.AddDynamic(this, &UMyClass::HandleFrustration);
 * @endcode
 *
 * @section bestpractices Best Practices
 * 1. Always call RecordRaceResult() after every race for accurate tracking
 * 2. Let players see and control their assists - transparency builds trust
 * 3. Don't make rubber-banding too obvious - players hate feeling cheated
 * 4. Gradual adjustments feel fairer than sudden difficulty spikes
 * 5. Give players the option to disable adaptive difficulty entirely
 *
 * @section delegates Available Delegates
 * - OnDifficultyChanged: Difficulty preset changed
 * - OnDifficultyAdjusted: A specific difficulty aspect was adjusted
 * - OnSkillLevelChanged: Player's estimated skill level changed
 * - OnFrustrationDetected: Frustration level changed
 * - OnAssistToggled: An assist was enabled/disabled
 * - OnRaceAnalyzed: Race result was processed
 * - OnAISettingsChanged: AI opponent settings were modified
 *
 * @see UMGBalancingSubsystem For economy and global balance settings
 * @see UMGStatsTracker For persistent stat tracking
 * ============================================================================
 */

// MGDynamicDifficultySubsystem.h
// Dynamic Difficulty Adjustment System - Adaptive AI and challenge scaling
// Midnight Grind - Y2K Arcade Street Racing

/**
 * OVERVIEW:
 * ---------
 * This file implements Dynamic Difficulty Adjustment (DDA) for Midnight Grind.
 * DDA is a technique used in modern games to automatically adjust the game's
 * challenge level based on the player's performance. The goal is to keep players
 * in the "flow state" - challenged enough to stay engaged, but not so frustrated
 * that they want to quit.
 *
 * This is one of the most important systems for player retention and satisfaction
 * in a racing game!
 *
 * WHAT DOES THIS SYSTEM DO?
 * -------------------------
 * 1. TRACKS PLAYER PERFORMANCE
 *    - Win rate, podium finishes, DNFs (Did Not Finish)
 *    - Lap times and racing line efficiency
 *    - Drift scores, overtakes, and collision frequency
 *    - Nitro usage patterns and shortcut discovery
 *
 * 2. ADJUSTS GAME DIFFICULTY
 *    - AI opponent speed and aggression
 *    - Rubber-banding intensity (AI catching up when behind)
 *    - Traffic and obstacle density
 *    - Catch-up assist strength for the player
 *
 * 3. MANAGES DRIVING ASSISTS
 *    - Steering, braking, and drift assists
 *    - Racing line display
 *    - Collision avoidance hints
 *    - Auto-recovery and rewind features
 *
 * 4. DETECTS PLAYER FRUSTRATION
 *    - Monitors patterns that suggest the player is struggling
 *    - Automatically responds with helpful adjustments
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. RUBBER-BANDING:
 *    A controversial but common racing game technique where AI opponents slow
 *    down when ahead and speed up when behind to keep races close and exciting.
 *    Too much rubber-banding feels unfair; too little makes races boring.
 *    This system lets you fine-tune the intensity.
 *
 * 2. CATCH-UP ASSIST:
 *    The opposite of rubber-banding - helps the PLAYER catch up when behind.
 *    Can include speed boosts, better slipstream effects, or AI making mistakes.
 *    Important for new players who might fall too far behind to recover.
 *
 * 3. SKILL LEVEL ESTIMATION:
 *    The system estimates player skill from Beginner to Legend based on their
 *    performance metrics. This helps match them against appropriate AI opponents
 *    and suggest suitable difficulty settings.
 *
 * 4. ADAPTATION SPEED:
 *    How quickly the system responds to player performance. "Instant" might feel
 *    jarring (sudden difficulty spikes), while "Gradual" provides smoother but
 *    slower adjustments. Different players prefer different speeds.
 *
 * 5. AI BEHAVIOR PROFILES:
 *    AI opponents can have different "personalities":
 *    - Defensive: Blocks overtakes, races conservatively
 *    - Aggressive: Takes risks, rams other racers
 *    - Tactical: Makes strategic decisions based on race position
 *    - TrainingWheel: Intentionally makes mistakes to help new players win
 *
 * 6. FRUSTRATION DETECTION:
 *    The system watches for signs of frustration:
 *    - Multiple consecutive losses
 *    - Frequent collisions
 *    - Restarting races repeatedly
 *    - Long times in last place
 *    When detected, it can automatically make the game easier.
 *
 * DATA STRUCTURES OVERVIEW:
 * -------------------------
 * - FPlayerPerformanceData: Complete history of player performance metrics
 * - FDifficultyModifiers: All the knobs that control game difficulty
 * - FAIOpponentSettings: Configuration for a single AI racer
 * - FAssistSettings: Player assist options (steering help, etc.)
 * - FRaceAnalysis: Detailed breakdown of a single race's performance
 * - FDifficultyAdjustment: Record of a single difficulty change
 * - FDifficultyProfile: Complete difficulty preset configuration
 *
 * USAGE EXAMPLE:
 * --------------
 * @code
 * UMGDynamicDifficultySubsystem* DDA = GetGameInstance()->GetSubsystem<UMGDynamicDifficultySubsystem>();
 *
 * // Enable adaptive difficulty
 * DDA->EnableAdaptiveDifficulty(true);
 *
 * // After a race, record the results
 * FRaceAnalysis Analysis;
 * Analysis.FinalPosition = 3;
 * Analysis.BestLapTime = 75.5f;
 * // ... fill in other fields
 * DDA->RecordRaceResult(Analysis);
 *
 * // Generate AI opponents for the next race
 * TArray<FAIOpponentSettings> Opponents = DDA->GenerateOpponentGrid(7);
 *
 * // Check if player needs help
 * if (DDA->GetCurrentFrustrationLevel() >= EFrustrationLevel::Frustrated)
 * {
 *     // Maybe show a "Would you like to lower the difficulty?" prompt
 * }
 * @endcode
 *
 * BEST PRACTICES:
 * ---------------
 * 1. Always call RecordRaceResult() after every race for accurate tracking
 * 2. Let players see and control their assists - transparency builds trust
 * 3. Don't make rubber-banding too obvious - players hate feeling cheated
 * 4. Gradual adjustments feel fairer than sudden difficulty spikes
 * 5. Give players the option to disable adaptive difficulty entirely
 *
 * @see UMGBalancingSubsystem for economy and global balance settings
 * @see UMGStatsTracker for persistent stat tracking
 */

// MGDynamicDifficultySubsystem.h
// Dynamic Difficulty Adjustment System - Adaptive AI and challenge scaling
// Midnight Grind - Y2K Arcade Street Racing

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDynamicDifficultySubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// Enums
// ============================================================================

/**
 * Difficulty presets that players can choose from.
 *
 * "Adaptive" is special - it means the system will automatically adjust
 * based on player performance. "Custom" means the player has manually
 * tweaked individual settings.
 */
UENUM(BlueprintType)
enum class EDifficultyPreset : uint8
{
    /** Minimal challenge - lots of assists, very slow AI */
    VeryEasy,
    /** Reduced challenge - helpful assists, slower AI */
    Easy,
    /** Balanced experience - the intended default */
    Normal,
    /** Increased challenge - fewer assists, faster AI */
    Hard,
    /** Significant challenge - minimal assists, aggressive AI */
    VeryHard,
    /** Maximum challenge - no assists, fastest AI */
    Extreme,
    /** Automatically adjusts based on player performance */
    Adaptive,
    /** Player has manually configured individual settings */
    Custom
};

/**
 * Metrics used to measure player performance.
 *
 * The DDA system tracks these metrics to understand how well the player
 * is doing and whether difficulty should be adjusted.
 */
UENUM(BlueprintType)
enum class EPerformanceMetric : uint8
{
    /** Where the player finishes in races (1st, 2nd, etc.) */
    RacePosition,
    /** How fast the player completes laps */
    LapTime,
    /** Points earned from drifting */
    DriftScore,
    /** How often the player crashes */
    CollisionCount,
    /** How effectively player uses nitro boosts */
    NitroUsage,
    /** Whether player discovers and uses shortcuts */
    ShortcutUsage,
    /** How many opponents the player passes */
    OvertakeCount,
    /** Percentage of races won */
    WinRate,
    /** Percentage of races completed (not DNF) */
    FinishRate,
    /** Typical speed maintained during races */
    AverageSpeed
};

/**
 * Personality profiles that define how AI opponents race.
 *
 * Each profile creates a distinct racing style. Mixing different
 * profiles in a race creates variety and more interesting competition.
 */
UENUM(BlueprintType)
enum class EAIBehaviorProfile : uint8
{
    /** Blocks overtakes, takes safe lines, avoids contact */
    Defensive,
    /** Standard racing behavior, mix of offense and defense */
    Balanced,
    /** Rams opponents, takes risky lines, fights for position */
    Aggressive,
    /** Makes smart decisions based on race situation and position */
    Tactical,
    /** Random behavior - hard to predict, keeps races interesting */
    Unpredictable,
    /** Speeds up when behind, slows when ahead (controversial but common) */
    Rubberband,
    /** Intentionally makes mistakes to help new players win */
    TrainingWheel
};

/**
 * Individual aspects of difficulty that can be adjusted independently.
 *
 * The DDA system can tune each of these separately for fine-grained
 * control over the game's challenge level.
 */
UENUM(BlueprintType)
enum class EDifficultyAspect : uint8
{
    /** How fast AI opponents drive */
    AISpeed,
    /** How aggressively AI races (ramming, blocking) */
    AIAggression,
    /** How often AI makes mistakes (missed corners, poor lines) */
    AIErrorRate,
    /** How many civilian cars are on the road */
    TrafficDensity,
    /** How many hazards/obstacles appear on track */
    ObstacleDensity,
    /** How much help the player gets when behind */
    CatchUpAssist,
    /** Boost received when drafting behind opponents */
    SlipstreamBoost,
    /** How fast nitro refills */
    NitroRecharge,
    /** Severity of speed loss from collisions */
    CollisionPenalty,
    /** Strictness of time limits in timed events */
    TimePressure
};

/**
 * Estimated player skill levels used for matchmaking and AI calibration.
 *
 * The system estimates skill from performance metrics and uses it to
 * generate appropriately challenging AI opponents.
 */
UENUM(BlueprintType)
enum class ESkillLevel : uint8
{
    /** New to racing games - needs significant help */
    Beginner,
    /** Learning the basics - still developing core skills */
    Novice,
    /** Competent racer - understands game mechanics */
    Intermediate,
    /** Skilled player - consistent performance */
    Advanced,
    /** Highly skilled - can compete at high level */
    Expert,
    /** Top tier player - near perfect execution */
    Master,
    /** Elite player - among the best */
    Legend
};

/**
 * How quickly the adaptive difficulty system responds to player performance.
 *
 * Faster adaptation responds quickly but can feel jarring (sudden difficulty
 * spikes). Slower adaptation feels smoother but might not help struggling
 * players quickly enough.
 */
UENUM(BlueprintType)
enum class EAdaptationSpeed : uint8
{
    /** Immediate changes - can feel jarring but responsive */
    Instant,
    /** Quick response - noticeable but not abrupt */
    Fast,
    /** Balanced response - recommended for most players */
    Medium,
    /** Gradual changes - very smooth but slow to respond */
    Slow,
    /** Very gradual - barely noticeable, long-term trends only */
    Gradual
};

/**
 * Types of driving assists available to help players.
 *
 * Assists help new players enjoy the game while they learn. Advanced
 * players typically disable them for a purer experience.
 */
UENUM(BlueprintType)
enum class EAssistType : uint8
{
    /** Helps keep the car going straight and through corners */
    Steering,
    /** Automatically applies brakes before corners */
    Braking,
    /** Helps maintain and control drifts */
    Drifting,
    /** Suggests optimal moments to use nitro */
    NitroTiming,
    /** Shows the optimal path through corners */
    RacingLine,
    /** Warns about imminent collisions */
    CollisionAvoidance,
    /** Reveals hidden shortcuts on the track */
    ShortcutHints,
    /** Shows where opponents are (minimap/arrows) */
    OpponentTracking
};

/**
 * Detected frustration level of the player.
 *
 * The system monitors for signs of frustration and can automatically
 * intervene to help (if frustration response is enabled).
 */
UENUM(BlueprintType)
enum class EFrustrationLevel : uint8
{
    /** Player is cruising - might need more challenge */
    Relaxed,
    /** Player is comfortable - ideal state */
    Comfortable,
    /** Player is challenged but engaged - good difficulty */
    Challenged,
    /** Player showing signs of frustration - consider easing up */
    Frustrated,
    /** Player very frustrated - needs immediate help or might quit */
    Overwhelmed
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FPlayerPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalRaces = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalWins = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalPodiums = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalFinishes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WinRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PodiumRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FinishRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AveragePosition = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLapTime = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BestLapTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageDriftScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDriftScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalOvertakes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCollisions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CollisionRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageTopSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NitroEfficiency = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShortcutsUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecentPerformanceTrend = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> RecentPositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> RecentLapTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> TrackBestTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESkillLevel EstimatedSkillLevel = ESkillLevel::Intermediate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastUpdated;
};

USTRUCT(BlueprintType)
struct FDifficultyModifiers
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float AISpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AIAggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float AIErrorRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float AIReactionTimeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float TrafficDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float ObstacleDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CatchUpAssistStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float SlipstreamBoostMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float NitroRechargeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float CollisionPenaltyMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float TimePressureMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float RewardMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float XPMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> CustomModifiers;
};

USTRUCT(BlueprintType)
struct FAIOpponentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OpponentId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OpponentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAIBehaviorProfile BehaviorProfile = EAIBehaviorProfile::Balanced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ErrorRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReactionTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DriftSkill = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OvertakeSkill = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefenseSkill = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NitroUsageEfficiency = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ShortcutKnowledge = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RubberbandingFactor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUsesRubberBanding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAdaptive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> TrackFamiliarity;
};

USTRUCT(BlueprintType)
struct FAssistSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSteeringAssist = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SteeringAssistStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bBrakingAssist = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BrakingAssistStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bDriftAssist = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DriftAssistStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNitroTimingAssist = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRacingLineDisplay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCollisionPrediction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bShortcutHints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bOpponentTrackingUI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoRecovery = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoRecoveryDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRewindFeature = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewindCharges = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, bool> CustomAssists;
};

USTRUCT(BlueprintType)
struct FRaceAnalysis
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid RaceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FinalPosition = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalRaceTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BestLapTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageLapTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositionChanges = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OvertakesMade = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 OvertakesReceived = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalCollisions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalDriftScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageSpeed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NitroActivations = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NitroTimeUsed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ShortcutsUsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInFirst = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeInLast = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GapToLeader = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float GapToSecond = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bFinished = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWon = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPodium = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFrustrationLevel DetectedFrustration = EFrustrationLevel::Comfortable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> PositionOverTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> LapTimes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> SectorTimes;
};

USTRUCT(BlueprintType)
struct FDifficultyAdjustment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDifficultyAspect Aspect = EDifficultyAspect::AISpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreviousValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NewValue = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChangeAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Reason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime AdjustedAt;
};

USTRUCT(BlueprintType)
struct FDifficultyProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDifficultyPreset BasePreset = EDifficultyPreset::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDifficultyModifiers Modifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FAssistSettings Assists;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FAIOpponentSettings> OpponentTemplates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAdaptationSpeed AdaptationSpeed = EAdaptationSpeed::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AdaptationSensitivity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAllowDynamicAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDifficulty = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDifficulty = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> CustomSettings;
};

USTRUCT(BlueprintType)
struct FAdaptiveHistory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDifficultyAdjustment> RecentAdjustments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FRaceAnalysis> RecentRaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OverallTrend = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StabilityScore = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ConsecutiveWins = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ConsecutiveLosses = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AdjustmentCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastAdjustment;
};

USTRUCT(BlueprintType)
struct FDynamicDifficultyStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalAdjustments = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DifficultyIncreases = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DifficultyDecreases = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageAdjustmentSize = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentDifficultyLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HistoricalWinRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecentWinRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerSatisfactionEstimate = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> AspectAdjustmentCounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> AspectAverageValues;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDifficultyChanged, EDifficultyPreset, OldPreset, EDifficultyPreset, NewPreset);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDifficultyAdjusted, const FDifficultyAdjustment&, Adjustment);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSkillLevelChanged, ESkillLevel, NewSkillLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFrustrationDetected, EFrustrationLevel, FrustrationLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssistToggled, EAssistType, AssistType, bool, bEnabled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceAnalyzed, const FRaceAnalysis&, Analysis);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAISettingsChanged, const FString&, OpponentId, const FAIOpponentSettings&, NewSettings);

// ============================================================================
// Main Subsystem
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGDynamicDifficultySubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // Difficulty Preset Management
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Presets")
    void SetDifficultyPreset(EDifficultyPreset Preset);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Presets")
    EDifficultyPreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Presets")
    FDifficultyProfile GetProfileForPreset(EDifficultyPreset Preset) const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Presets")
    void CreateCustomProfile(const FString& ProfileName, const FDifficultyProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Presets")
    void LoadCustomProfile(const FString& ProfileName);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Presets")
    TArray<FString> GetAvailableCustomProfiles() const;

    // ========================================================================
    // Difficulty Modifiers
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Modifiers")
    FDifficultyModifiers GetCurrentModifiers() const { return CurrentModifiers; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Modifiers")
    void SetModifiers(const FDifficultyModifiers& NewModifiers);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Modifiers")
    void AdjustModifier(EDifficultyAspect Aspect, float NewValue);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Modifiers")
    float GetModifierValue(EDifficultyAspect Aspect) const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Modifiers")
    void ResetModifiersToPreset();

    // ========================================================================
    // Player Performance Tracking
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Performance")
    void RecordRaceResult(const FRaceAnalysis& Analysis);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Performance")
    FPlayerPerformanceData GetPlayerPerformance() const { return PlayerPerformance; }

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Performance")
    ESkillLevel GetEstimatedSkillLevel() const { return PlayerPerformance.EstimatedSkillLevel; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Performance")
    void UpdatePerformanceMetric(EPerformanceMetric Metric, float Value);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Performance")
    float GetPerformanceTrend() const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Performance")
    void ResetPerformanceData();

    // ========================================================================
    // Adaptive Difficulty
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Adaptive")
    void EnableAdaptiveDifficulty(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Adaptive")
    bool IsAdaptiveDifficultyEnabled() const { return bAdaptiveDifficultyEnabled; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Adaptive")
    void SetAdaptationSpeed(EAdaptationSpeed Speed);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Adaptive")
    void SetAdaptationSensitivity(float Sensitivity);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Adaptive")
    void TriggerAdaptiveAdjustment();

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Adaptive")
    FAdaptiveHistory GetAdaptiveHistory() const { return AdaptiveHistory; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Adaptive")
    void SetAdaptiveBounds(float MinDifficulty, float MaxDifficulty);

    // ========================================================================
    // AI Opponent Configuration
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|AI")
    FAIOpponentSettings GenerateOpponentSettings(ESkillLevel TargetSkill, EAIBehaviorProfile Profile);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|AI")
    TArray<FAIOpponentSettings> GenerateOpponentGrid(int32 OpponentCount);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|AI")
    void AdjustOpponentDifficulty(const FString& OpponentId, float DifficultyDelta);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|AI")
    void SetOpponentBehaviorProfile(const FString& OpponentId, EAIBehaviorProfile Profile);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|AI")
    FAIOpponentSettings GetOpponentSettings(const FString& OpponentId) const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|AI")
    void ApplyRubberbanding(const FString& OpponentId, float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|AI")
    void SetGlobalRubberbanding(bool bEnable, float Strength);

    // ========================================================================
    // Assist Configuration
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Assists")
    FAssistSettings GetCurrentAssists() const { return CurrentAssists; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Assists")
    void SetAssists(const FAssistSettings& NewAssists);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Assists")
    void ToggleAssist(EAssistType AssistType, bool bEnable);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Assists")
    bool IsAssistEnabled(EAssistType AssistType) const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Assists")
    void SetAssistStrength(EAssistType AssistType, float Strength);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Assists")
    void SuggestAssistsForSkillLevel(ESkillLevel SkillLevel);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Assists")
    void ResetAssistsToDefault();

    // ========================================================================
    // Frustration Detection
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Frustration")
    EFrustrationLevel DetectFrustrationLevel(const FRaceAnalysis& Analysis);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Frustration")
    void RespondToFrustration(EFrustrationLevel Level);

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Frustration")
    EFrustrationLevel GetCurrentFrustrationLevel() const { return CurrentFrustrationLevel; }

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Frustration")
    void SetFrustrationResponseEnabled(bool bEnable);

    // ========================================================================
    // Race-Time Adjustments
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|RealTime")
    void UpdateRaceProgress(int32 PlayerPosition, float GapToLeader, float RaceProgress);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|RealTime")
    void OnPlayerCollision(float ImpactSeverity);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|RealTime")
    void OnPlayerOvertake(bool bMadeOvertake);

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|RealTime")
    float GetDynamicCatchUpBoost() const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|RealTime")
    float GetDynamicAISlowdown() const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|RealTime")
    void ResetRaceTimeAdjustments();

    /** Get the number of collisions recorded during the current race */
    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|RealTime")
    int32 GetRaceCollisionCount() const { return RaceCollisionCount; }

    // ========================================================================
    // Statistics
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Stats")
    FDynamicDifficultyStats GetDifficultyStats() const { return DifficultyStats; }

    UFUNCTION(BlueprintPure, Category = "DynamicDifficulty|Stats")
    float GetOverallDifficultyLevel() const;

    UFUNCTION(BlueprintCallable, Category = "DynamicDifficulty|Stats")
    void ExportPerformanceReport(FString& OutReport);

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnDifficultyChanged OnDifficultyChanged;

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnDifficultyAdjusted OnDifficultyAdjusted;

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnSkillLevelChanged OnSkillLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnFrustrationDetected OnFrustrationDetected;

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnAssistToggled OnAssistToggled;

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnRaceAnalyzed OnRaceAnalyzed;

    UPROPERTY(BlueprintAssignable, Category = "DynamicDifficulty|Events")
    FOnAISettingsChanged OnAISettingsChanged;

protected:
    void InitializePresetProfiles();
    void LoadPlayerData();
    void SavePlayerData();

    void CalculateSkillLevel();
    float CalculateAdaptiveAdjustment();
    void ApplyAdaptiveAdjustment(float Adjustment);

    FDifficultyAdjustment CreateAdjustment(EDifficultyAspect Aspect, float OldValue, float NewValue, const FString& Reason);

private:
    UPROPERTY()
    EDifficultyPreset CurrentPreset;

    UPROPERTY()
    FDifficultyModifiers CurrentModifiers;

    UPROPERTY()
    FAssistSettings CurrentAssists;

    UPROPERTY()
    FPlayerPerformanceData PlayerPerformance;

    UPROPERTY()
    FAdaptiveHistory AdaptiveHistory;

    UPROPERTY()
    FDynamicDifficultyStats DifficultyStats;

    UPROPERTY()
    TMap<FString, FDifficultyProfile> CustomProfiles;

    UPROPERTY()
    TMap<FString, FAIOpponentSettings> ActiveOpponents;

    UPROPERTY()
    TMap<EDifficultyPreset, FDifficultyProfile> PresetProfiles;

    UPROPERTY()
    bool bAdaptiveDifficultyEnabled;

    UPROPERTY()
    EAdaptationSpeed CurrentAdaptationSpeed;

    UPROPERTY()
    float AdaptationSensitivity;

    UPROPERTY()
    float MinDifficultyBound;

    UPROPERTY()
    float MaxDifficultyBound;

    UPROPERTY()
    EFrustrationLevel CurrentFrustrationLevel;

    UPROPERTY()
    bool bFrustrationResponseEnabled;

    // Race-time tracking
    UPROPERTY()
    int32 CurrentRacePosition;

    UPROPERTY()
    float CurrentGapToLeader;

    UPROPERTY()
    float CurrentRaceProgress;

    UPROPERTY()
    int32 RaceCollisionCount;

    UPROPERTY()
    bool bGlobalRubberbandingEnabled;

    UPROPERTY()
    float GlobalRubberbandingStrength;

    FTimerHandle AdaptiveUpdateTimer;
};

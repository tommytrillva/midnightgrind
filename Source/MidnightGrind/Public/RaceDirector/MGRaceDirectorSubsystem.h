#pragma once


/**
 * @file MGRaceDirectorSubsystem.h
 * @brief Race Director Subsystem - AI-driven race pacing and dramatic moment orchestration
 *
 * The Race Director is Midnight Grind's "invisible hand" that ensures every race feels
 * exciting and competitive. Inspired by racing game systems like Burnout and Need for Speed,
 * it dynamically adjusts AI behavior, applies rubber-banding, and orchestrates dramatic
 * moments like close finishes and comebacks.
 *
 * ## Key Responsibilities
 * - Monitoring race tension and pacing in real-time
 * - Applying rubber-band speed/handling modifiers to keep races competitive
 * - Detecting and enhancing dramatic moments (lead changes, photo finishes)
 * - Controlling AI behavior states (aggressive, defensive, catch-up)
 * - Managing race phases (early race chaos, mid-race settling, final lap push)
 * - Tracking comprehensive race statistics for post-race analysis
 *
 * ## Director Styles
 * - Authentic: Minimal intervention for sim-racing purists
 * - Competitive: Moderate rubber-banding for close races
 * - Dramatic: Maximizes exciting moments and close finishes
 * - Arcade: Heavy catch-up, fun-first philosophy
 * - Simulation: Zero assistance, realistic AI
 * - Balanced: Adapts to player skill over time
 *
 * ## How Rubber-Banding Works
 * When a racer falls behind, they receive subtle speed/handling boosts.
 * When a racer gets too far ahead, they may experience slight reductions.
 * This keeps the pack together without feeling unfair. The intensity is
 * controlled by the FMGRubberBandConfig and director style.
 *
 * ## Usage Example
 * @code
 * UMGRaceDirectorSubsystem* Director = GetGameInstance()->GetSubsystem<UMGRaceDirectorSubsystem>();
 * Director->SetDirectorStyle(EMGDirectorStyle::Competitive);
 * Director->InitializeRace(3, 5000.0f); // 3 laps, 5km track
 * Director->StartRace();
 * // Each frame:
 * Director->UpdateDirector(DeltaTime);
 * float SpeedMod = Director->GetSpeedModifier(RacerID);
 * @endcode
 *
 * @note This is a GameInstanceSubsystem that persists across level loads
 * @see UMGRaceFlowSubsystem for high-level race orchestration
 * @see UMGRaceModeSubsystem for core race logic
 *
 * Copyright Midnight Grind. All Rights Reserved.
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceDirectorSubsystem.generated.h"

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

class UMGRaceDirectorSubsystem;

// ============================================================================
// DIRECTOR STYLE ENUMERATION
// ============================================================================

/**
 * @enum EMGDirectorStyle
 * @brief Defines how aggressively the race director intervenes
 *
 * Each style represents a different philosophy on race pacing and
 * player assistance. Players can choose their preferred experience.
 */
UENUM(BlueprintType)
enum class EMGDirectorStyle : uint8
{
    /// Minimal intervention - pure skill determines outcomes
    Authentic       UMETA(DisplayName = "Authentic"),

    /// Moderate rubber-banding - keeps races competitive
    Competitive     UMETA(DisplayName = "Competitive"),

    /// Maximum drama - orchestrates exciting moments and close finishes
    Dramatic        UMETA(DisplayName = "Dramatic"),

    /// Heavy catch-up - prioritizes fun over realism
    Arcade          UMETA(DisplayName = "Arcade"),

    /// No assistance - realistic AI with no adjustments
    Simulation      UMETA(DisplayName = "Simulation"),

    /// Adapts dynamically based on player skill history
    Balanced        UMETA(DisplayName = "Balanced")
};

// ============================================================================
// RACE PHASE ENUMERATION
// ============================================================================

/**
 * @enum EMGRacePhase
 * @brief Current phase of the race for pacing decisions
 *
 * Different phases have different pacing goals. Early race allows
 * chaos and position changes, mid-race settles, and late race
 * increases intensity for an exciting finish.
 */
UENUM(BlueprintType)
enum class EMGRacePhase : uint8
{
    /// Before race starts - grid formation
    PreRace         UMETA(DisplayName = "Pre-Race"),

    /// First few seconds - green flag chaos
    Start           UMETA(DisplayName = "Start"),

    /// First ~25% of race - positions establishing
    EarlyRace       UMETA(DisplayName = "Early Race"),

    /// Middle ~25-75% of race - settled racing
    MidRace         UMETA(DisplayName = "Mid Race"),

    /// 75%+ of race - building to finale
    LateRace        UMETA(DisplayName = "Late Race"),

    /// Last lap - maximum intensity
    FinalLap        UMETA(DisplayName = "Final Lap"),

    /// Within striking distance at finish - extreme tension
    PhotoFinish     UMETA(DisplayName = "Photo Finish"),

    /// Race has ended
    Finished        UMETA(DisplayName = "Finished")
};

// ============================================================================
// DRAMATIC MOMENT ENUMERATION
// ============================================================================

/**
 * @enum EMGDramaticMoment
 * @brief Types of exciting race events the director tracks and enhances
 *
 * When these moments occur, the director may trigger special effects,
 * camera angles, audio cues, or UI highlights.
 */
UENUM(BlueprintType)
enum class EMGDramaticMoment : uint8
{
    /// No special moment occurring
    None            UMETA(DisplayName = "None"),

    /// Racers within close proximity - tense racing
    CloseRace       UMETA(DisplayName = "Close Race"),

    /// Racer recovering from significant position loss
    Comeback        UMETA(DisplayName = "Comeback"),

    /// Position changed for the lead
    LeadChange      UMETA(DisplayName = "Lead Change"),

    /// Race decided by less than 0.5 seconds at finish
    PhotoFinish     UMETA(DisplayName = "Photo Finish"),

    /// Lower-skilled racer challenging favorites
    Underdog        UMETA(DisplayName = "Underdog Victory"),

    /// Racer dominating with large lead
    Dominance       UMETA(DisplayName = "Dominant Performance"),

    /// Designated rival confrontation
    Rivalry         UMETA(DisplayName = "Rivalry Moment"),

    /// Near-miss collision avoided
    WreckAvoidance  UMETA(DisplayName = "Wreck Avoidance"),

    /// Clean lap with optimal racing line
    PerfectLap      UMETA(DisplayName = "Perfect Lap")
};

// ============================================================================
// RUBBER-BAND LEVEL ENUMERATION
// ============================================================================

/**
 * @enum EMGRubberBandLevel
 * @brief Intensity of the catch-up/slowdown assistance
 *
 * Higher levels mean more aggressive speed adjustments to keep
 * the pack together. Lower levels feel more authentic but may
 * result in runaway victories or losses.
 */
UENUM(BlueprintType)
enum class EMGRubberBandLevel : uint8
{
    /// No rubber-banding - pure racing
    None            UMETA(DisplayName = "None"),

    /// Barely perceptible assistance
    VeryLight       UMETA(DisplayName = "Very Light"),

    /// Subtle assistance - hard to notice
    Light           UMETA(DisplayName = "Light"),

    /// Noticeable but fair assistance
    Moderate        UMETA(DisplayName = "Moderate"),

    /// Significant catch-up mechanics
    Strong          UMETA(DisplayName = "Strong"),

    /// Maximum assistance - arcade feel
    VeryStrong      UMETA(DisplayName = "Very Strong")
};

// ============================================================================
// AI BEHAVIOR STATE ENUMERATION
// ============================================================================

/**
 * @enum EMGAIBehaviorState
 * @brief Current behavioral mode for AI racers
 *
 * The director assigns behavior states to AI racers based on
 * their position, proximity to player, and race phase.
 */
UENUM(BlueprintType)
enum class EMGAIBehaviorState : uint8
{
    /// Standard racing behavior
    Normal          UMETA(DisplayName = "Normal"),

    /// Actively trying to overtake - risky moves
    Aggressive      UMETA(DisplayName = "Aggressive"),

    /// Protecting position - blocking lines
    Defensive       UMETA(DisplayName = "Defensive"),

    /// Specifically targeting player for overtake
    Hunting         UMETA(DisplayName = "Hunting"),

    /// Deliberately blocking player's racing line
    Blocking        UMETA(DisplayName = "Blocking"),

    /// Receiving speed boost to close gap
    CatchUp         UMETA(DisplayName = "Catch Up"),

    /// Being slowed to let others catch up
    SlowDown        UMETA(DisplayName = "Slow Down"),

    /// Intentionally making an error (spins, missed apex)
    Mistake         UMETA(DisplayName = "Making Mistake"),

    /// Recovering from crash or spin
    Recovery        UMETA(DisplayName = "Recovery")
};

// ============================================================================
// POSITION ADJUSTMENT ENUMERATION
// ============================================================================

/**
 * @enum EMGPositionAdjustment
 * @brief Types of performance modifiers applied by rubber-banding
 *
 * The director applies these adjustments invisibly to keep races
 * competitive without obviously cheating for or against racers.
 */
UENUM(BlueprintType)
enum class EMGPositionAdjustment : uint8
{
    /// No adjustment active
    None            UMETA(DisplayName = "None"),

    /// Increased top speed
    SpeedBoost      UMETA(DisplayName = "Speed Boost"),

    /// Decreased top speed
    SpeedReduction  UMETA(DisplayName = "Speed Reduction"),

    /// Improved grip and turn-in
    BetterHandling  UMETA(DisplayName = "Better Handling"),

    /// Reduced grip and stability
    WorseHandling   UMETA(DisplayName = "Worse Handling"),

    /// Faster nitro recharge
    NitroBonus      UMETA(DisplayName = "Nitro Bonus"),

    /// More likely to make driving errors
    MistakeProne    UMETA(DisplayName = "Mistake Prone")
};

// ============================================================================
// RACE TENSION ENUMERATION
// ============================================================================

/**
 * @enum EMGRaceTension
 * @brief Overall tension level of the current race
 *
 * Used to drive audio, visual effects, and commentator systems.
 * Higher tension means more dramatic music and effects.
 */
UENUM(BlueprintType)
enum class EMGRaceTension : uint8
{
    /// Comfortable lead or no competition
    Calm            UMETA(DisplayName = "Calm"),

    /// Some competition present
    Mild            UMETA(DisplayName = "Mild"),

    /// Active battling for position
    Moderate        UMETA(DisplayName = "Moderate"),

    /// Close racing, position changes
    Intense         UMETA(DisplayName = "Intense"),

    /// Photo finish territory
    Extreme         UMETA(DisplayName = "Extreme")
};

// ============================================================================
// RACER STATE STRUCTURE
// ============================================================================

/**
 * @struct FMGRacerState
 * @brief Comprehensive state tracking for a single racer
 *
 * The director maintains this state for each racer to make pacing
 * decisions. Updated continuously during the race.
 */
USTRUCT(BlueprintType)
struct FMGRacerState
{
    GENERATED_BODY()

    // ---- Identity ----

    /// Unique identifier for this racer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    FGuid RacerId;

    /// Display name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    FString RacerName;

    /// True if this is the human player
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bIsPlayer;

    /// True if racer is still in the race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bIsActive;

    // ---- Position Tracking ----

    /// Current race position (1 = first)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 CurrentPosition;

    /// Position at race start (for comeback tracking)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 StartingPosition;

    /// Best position achieved during race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 BestPosition;

    /// Worst position during race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 WorstPosition;

    /// Current lap number
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 CurrentLap;

    /// Overall race progress (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float RaceProgress;

    // ---- Gap Tracking ----

    /// Distance in meters to race leader
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float DistanceToLeader;

    /// Distance to racer directly ahead
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float DistanceToAhead;

    /// Distance to racer directly behind
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float DistanceToBehind;

    // ---- Speed Data ----

    /// Current speed in km/h
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float CurrentSpeed;

    /// Maximum theoretical speed for this vehicle
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float MaxSpeed;

    // ---- Director Modifiers ----

    /// Current speed modifier from rubber-banding (1.0 = normal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float SpeedModifier;

    /// Current handling modifier (1.0 = normal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float HandlingModifier;

    /// Current AI behavior mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    EMGAIBehaviorState BehaviorState;

    /// Active position adjustment type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    EMGPositionAdjustment CurrentAdjustment;

    // ---- Race Statistics ----

    /// Total position changes during race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 PositionChanges;

    /// Successful takedowns of other racers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 Takedowns;

    /// Times this racer was wrecked
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 TimesWrecked;

    // ---- AI Personality ----

    /// Overall skill rating (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float SkillRating;

    /// Aggression tendency (0.0 = passive, 1.0 = very aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float AggresionLevel;

    /// Current lap performance vs. expected
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float PerformanceRating;

    // ---- Special Status ----

    /// Is this a story/career rival
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bIsRival;

    /// Has racer finished the race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bHasFinished;

    /// Time when racer finished
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float FinishTime;

    /// Default constructor with sensible initial values
    FMGRacerState()
        : bIsPlayer(false)
        , bIsActive(true)
        , CurrentPosition(0)
        , StartingPosition(0)
        , BestPosition(99)
        , WorstPosition(0)
        , CurrentLap(1)
        , RaceProgress(0.0f)
        , DistanceToLeader(0.0f)
        , DistanceToAhead(0.0f)
        , DistanceToBehind(0.0f)
        , CurrentSpeed(0.0f)
        , MaxSpeed(200.0f)
        , SpeedModifier(1.0f)
        , HandlingModifier(1.0f)
        , BehaviorState(EMGAIBehaviorState::Normal)
        , CurrentAdjustment(EMGPositionAdjustment::None)
        , PositionChanges(0)
        , Takedowns(0)
        , TimesWrecked(0)
        , SkillRating(0.5f)
        , AggresionLevel(0.5f)
        , PerformanceRating(1.0f)
        , bIsRival(false)
        , bHasFinished(false)
        , FinishTime(0.0f)
    {}
};

// ============================================================================
// RUBBER-BAND CONFIGURATION STRUCTURE
// ============================================================================

/**
 * @struct FMGRubberBandConfig
 * @brief Configuration for the catch-up/slowdown system
 *
 * Fine-tunes how aggressively rubber-banding affects racers.
 * Can be modified at runtime for dynamic difficulty adjustment.
 */
USTRUCT(BlueprintType)
struct FMGRubberBandConfig
{
    GENERATED_BODY()

    /// Overall intensity level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    EMGRubberBandLevel Level;

    /// Maximum speed boost multiplier (e.g., 1.1 = 10% faster)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float MaxSpeedBoost;

    /// Maximum speed reduction multiplier (e.g., 0.95 = 5% slower)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float MaxSpeedReduction;

    /// Distance in meters before rubber-banding activates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float ActivationDistance;

    /// Seconds after activation before effect begins
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float CooldownTime;

    /// Seconds to reach full effect intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float RampUpTime;

    /// Handling boost multiplier for struggling racers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float HandlingBoost;

    /// Nitro recharge rate multiplier for struggling racers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float NitroRechargeBonus;

    /// Apply rubber-banding to player (not just AI)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    bool bAffectsPlayer;

    /// Apply rubber-banding to AI racers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    bool bAffectsAI;

    /// Increase effect based on position (last place gets more help)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    bool bScaleWithPosition;

    /// Default constructor with balanced settings
    FMGRubberBandConfig()
        : Level(EMGRubberBandLevel::Moderate)
        , MaxSpeedBoost(1.1f)
        , MaxSpeedReduction(0.95f)
        , ActivationDistance(100.0f)
        , CooldownTime(3.0f)
        , RampUpTime(2.0f)
        , HandlingBoost(1.05f)
        , NitroRechargeBonus(1.2f)
        , bAffectsPlayer(true)
        , bAffectsAI(true)
        , bScaleWithPosition(true)
    {}
};

// ============================================================================
// DRAMA CONFIGURATION STRUCTURE
// ============================================================================

/**
 * @struct FMGDramaConfig
 * @brief Configuration for dramatic moment detection and enhancement
 *
 * Controls how the director identifies and responds to exciting
 * race events. Higher values mean more aggressive drama seeking.
 */
USTRUCT(BlueprintType)
struct FMGDramaConfig
{
    GENERATED_BODY()

    /// Gap in seconds to consider a "close race" (triggers tension)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float CloseRaceThreshold;

    /// Time window at finish for "photo finish" detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float PhotoFinishWindow;

    /// Positions gained to trigger "comeback" detection
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float ComebackThreshold;

    /// Multiplier for lead change tension contribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float LeadChangeWeight;

    /// Rate at which tension builds (per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float TensionBuildupRate;

    /// Minimum seconds between dramatic moment triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float MinDramaCooldown;

    /// Enable dramatic moment system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    bool bEnableDramaticMoments;

    /// Enable rivalry tracking and enhancement
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    bool bEnableRivalrySystem;

    /// Give extra help to underdogs challenging leaders
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    bool bEnableUnderdogBonus;

    /// Default constructor with balanced settings
    FMGDramaConfig()
        : CloseRaceThreshold(3.0f)
        , PhotoFinishWindow(0.5f)
        , ComebackThreshold(5)
        , LeadChangeWeight(1.5f)
        , TensionBuildupRate(0.1f)
        , MinDramaCooldown(10.0f)
        , bEnableDramaticMoments(true)
        , bEnableRivalrySystem(true)
        , bEnableUnderdogBonus(true)
    {}
};

// ============================================================================
// AI DIFFICULTY CONFIGURATION STRUCTURE
// ============================================================================

/**
 * @struct FMGAIDifficultyConfig
 * @brief Comprehensive AI behavior configuration for a difficulty level
 *
 * Defines how AI racers perform at different difficulty settings.
 * The director uses this to configure AI behavior.
 */
USTRUCT(BlueprintType)
struct FMGAIDifficultyConfig
{
    GENERATED_BODY()

    /// Display name for this difficulty
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    FString DifficultyName;

    /// Speed multiplier vs. base performance (1.0 = 100%)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float SpeedMultiplier;

    /// Reaction time in seconds (lower = faster reactions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float ReactionTime;

    /// Frequency of AI mistakes (0.0 = never, 1.0 = frequent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float MistakeFrequency;

    /// Base aggression level for overtaking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float AggressionBase;

    /// How closely AI follows optimal racing line (1.0 = perfect)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float RacingLineOptimality;

    /// How efficiently AI uses nitro (1.0 = optimal)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float NitroUsageEfficiency;

    /// Drifting skill level (1.0 = expert)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float DriftProficiency;

    /// Traffic avoidance skill (1.0 = never hits traffic)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float TrafficAvoidance;

    /// Speed of recovery from crashes/spins
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float RecoverySpeed;

    /// Associated rubber-band level for this difficulty
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    EMGRubberBandLevel RubberBandLevel;

    /// Default constructor with medium difficulty values
    FMGAIDifficultyConfig()
        : DifficultyName(TEXT("Normal"))
        , SpeedMultiplier(1.0f)
        , ReactionTime(0.3f)
        , MistakeFrequency(0.1f)
        , AggressionBase(0.5f)
        , RacingLineOptimality(0.8f)
        , NitroUsageEfficiency(0.7f)
        , DriftProficiency(0.7f)
        , TrafficAvoidance(0.8f)
        , RecoverySpeed(0.8f)
        , RubberBandLevel(EMGRubberBandLevel::Moderate)
    {}
};

// ============================================================================
// RACE EVENT STRUCTURE
// ============================================================================

/**
 * @struct FMGRaceEvent
 * @brief Record of a dramatic moment that occurred during the race
 *
 * Used for post-race highlights, replays, and statistics.
 */
USTRUCT(BlueprintType)
struct FMGRaceEvent
{
    GENERATED_BODY()

    /// Unique identifier for this event
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGuid EventId;

    /// Type of dramatic moment
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    EMGDramaticMoment MomentType;

    /// Race time when event occurred
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float Timestamp;

    /// Primary racer involved (e.g., overtaker)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGuid PrimaryRacerId;

    /// Secondary racer involved (e.g., overtaken)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGuid SecondaryRacerId;

    /// Lap when event occurred
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    int32 Lap;

    /// Intensity/significance of the moment (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float Intensity;

    /// Human-readable description for display
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString Description;

    /// Default constructor
    FMGRaceEvent()
        : MomentType(EMGDramaticMoment::None)
        , Timestamp(0.0f)
        , Lap(1)
        , Intensity(0.0f)
    {}
};

// ============================================================================
// RACE PACING CONFIGURATION STRUCTURE
// ============================================================================

/**
 * @struct FMGRacePacingConfig
 * @brief Configuration for race phase timing and intensity
 *
 * Defines when race phases transition and how intense each phase
 * should be. Used for dynamic difficulty and drama adjustment.
 */
USTRUCT(BlueprintType)
struct FMGRacePacingConfig
{
    GENERATED_BODY()

    /// Percentage of race considered "early" (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float EarlyRacePercent;

    /// Percentage of race considered "mid" (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float MidRacePercent;

    /// Percentage of race considered "late" (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float LateRacePercent;

    /// Intensity multiplier for final lap
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float FinalLapIntensity;

    /// Seconds at start with increased chaos tolerance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float StartChaosWindow;

    /// Seconds of calm mid-race settling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float MidRaceSettleTime;

    /// Seconds before finish to begin final push
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float EndGamePushTime;

    /// Default constructor with standard race pacing
    FMGRacePacingConfig()
        : EarlyRacePercent(0.25f)
        , MidRacePercent(0.50f)
        , LateRacePercent(0.75f)
        , FinalLapIntensity(1.3f)
        , StartChaosWindow(10.0f)
        , MidRaceSettleTime(5.0f)
        , EndGamePushTime(30.0f)
    {}
};

// ============================================================================
// DIRECTOR STATE STRUCTURE
// ============================================================================

/**
 * @struct FMGDirectorState
 * @brief Current state summary of the race director
 *
 * Provides a snapshot of all director decisions and race status.
 * Useful for debugging and UI display.
 */
USTRUCT(BlueprintType)
struct FMGDirectorState
{
    GENERATED_BODY()

    /// Current race phase
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    EMGRacePhase CurrentPhase;

    /// Current tension level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    EMGRaceTension TensionLevel;

    /// Active dramatic moment (if any)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    EMGDramaticMoment CurrentMoment;

    /// Overall race progress (0.0 to 1.0 based on leader)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float RaceProgress;

    /// Tension score (raw value before level mapping)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float TensionScore;

    /// Total lead changes in race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    int32 LeadChanges;

    /// Average gap between racers (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float AverageGap;

    /// Player performance rating (1.0 = as expected)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float PlayerPerformance;

    /// True if race is considered close
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    bool bIsCloseRace;

    /// True if photo finish is still possible
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    bool bPhotoFinishPossible;

    /// Default constructor
    FMGDirectorState()
        : CurrentPhase(EMGRacePhase::PreRace)
        , TensionLevel(EMGRaceTension::Calm)
        , CurrentMoment(EMGDramaticMoment::None)
        , RaceProgress(0.0f)
        , TensionScore(0.0f)
        , LeadChanges(0)
        , AverageGap(0.0f)
        , PlayerPerformance(1.0f)
        , bIsCloseRace(false)
        , bPhotoFinishPossible(false)
    {}
};

// ============================================================================
// RACE STATISTICS STRUCTURE
// ============================================================================

/**
 * @struct FMGRaceStatistics
 * @brief Comprehensive statistics gathered during a race
 *
 * Used for post-race analysis, achievements, and telemetry.
 */
USTRUCT(BlueprintType)
struct FMGRaceStatistics
{
    GENERATED_BODY()

    // ---- Racer Counts ----

    /// Total racers at race start
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalRacers;

    /// Racers still racing (not finished/wrecked)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 ActiveRacers;

    /// Racers who have finished
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 FinishedRacers;

    /// Racers who crashed out
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 WreckedRacers;

    // ---- Event Counts ----

    /// Total times the lead changed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalLeadChanges;

    /// Total position changes (all racers)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalPositionChanges;

    /// Total takedowns (vehicle eliminations)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalTakedowns;

    /// Dramatic moments triggered
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalDramaticMoments;

    // ---- Performance Metrics ----

    /// Average speed across all racers (km/h)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AverageSpeed;

    /// Fastest lap time in the race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FastestLap;

    /// Slowest lap time in the race
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SlowestLap;

    /// Gap between 1st and 2nd at finish
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WinningMargin;

    /// Closest gap during the race (meters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ClosestGap;

    /// Total race duration (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RaceTime;

    /// Default constructor
    FMGRaceStatistics()
        : TotalRacers(0)
        , ActiveRacers(0)
        , FinishedRacers(0)
        , WreckedRacers(0)
        , TotalLeadChanges(0)
        , TotalPositionChanges(0)
        , TotalTakedowns(0)
        , TotalDramaticMoments(0)
        , AverageSpeed(0.0f)
        , FastestLap(999999.0f)
        , SlowestLap(0.0f)
        , WinningMargin(0.0f)
        , ClosestGap(999999.0f)
        , RaceTime(0.0f)
    {}
};

// ============================================================================
// EVENT DELEGATES
// ============================================================================

/// Broadcast when race phase changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRacePhaseChanged, EMGRacePhase, NewPhase);

/// Broadcast when a dramatic moment is detected
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDramaticMoment, const FMGRaceEvent&, Event);

/// Broadcast when the race lead changes hands
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLeadChange, const FGuid&, NewLeaderId, int32, TotalChanges);

/// Broadcast when any racer's position changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPositionChange, const FGuid&, RacerId, int32, NewPosition);

/// Broadcast when tension level changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTensionChanged, EMGRaceTension, NewTension);

/// Broadcast when rubber-banding is applied to a racer
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRubberBandApplied, const FGuid&, RacerId, float, Modifier);

/// Broadcast when race finishes with final statistics
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRaceFinished, const FMGRaceStatistics&, Statistics);

/// Broadcast when individual racer finishes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRacerFinished, const FGuid&, RacerId, int32, Position);

// ============================================================================
// RACE DIRECTOR SUBSYSTEM CLASS
// ============================================================================

/**
 * @class UMGRaceDirectorSubsystem
 * @brief AI-driven race pacing and dramatic moment orchestration
 *
 * ## Overview
 * The Race Director ensures every race in Midnight Grind feels exciting.
 * It works behind the scenes to keep races competitive, create dramatic
 * moments, and adapt difficulty to player skill.
 *
 * ## Key Systems
 * - **Rubber-Banding**: Invisible speed/handling adjustments to keep pack together
 * - **AI Behavior Control**: Dynamic switching between aggressive/defensive/catch-up
 * - **Drama Detection**: Identifies exciting moments for camera/audio enhancement
 * - **Race Pacing**: Phase-based intensity management
 * - **Statistics Tracking**: Comprehensive race data for post-race analysis
 *
 * ## Integration
 * - Call UpdateDirector() every frame during a race
 * - Query GetSpeedModifier() for rubber-band adjustments
 * - Query GetRecommendedBehavior() for AI decision making
 * - Subscribe to events for dramatic moment notifications
 *
 * ## For New Developers
 * 1. Call SetDirectorStyle() to choose racing philosophy
 * 2. Call InitializeRace() with lap count and track length
 * 3. Call RegisterRacer() for each participant
 * 4. Call StartRace() when countdown ends
 * 5. Call UpdateDirector() and UpdateRacerState() each frame
 * 6. Query modifiers and apply to vehicle physics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGRaceDirectorSubsystem();

    //~ Begin USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    // ==========================================
    // RACE LIFECYCLE
    // Setup and control the race
    // ==========================================

    /// Initialize director for a new race
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void InitializeRace(int32 TotalLaps, float TrackLength);

    /// Signal race has started (after countdown)
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void StartRace();

    /// Signal race has ended
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void EndRace();

    /// Reset director state for restart
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void ResetRace();

    /// Check if race is active
    UFUNCTION(BlueprintPure, Category = "RaceDirector")
    bool IsRaceActive() const;

    // ==========================================
    // RACER MANAGEMENT
    // Register and update racer information
    // ==========================================

    /// Register a new racer with the director. Returns racer's GUID.
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    FGuid RegisterRacer(const FString& RacerName, bool bIsPlayer, int32 StartPosition);

    /// Remove racer from director tracking
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void UnregisterRacer(const FGuid& RacerId);

    /// Update racer's position, speed, and progress (call each frame)
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void UpdateRacerState(const FGuid& RacerId, int32 Position, float Speed, float Progress);

    /// Update racer's current lap
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void SetRacerLap(const FGuid& RacerId, int32 Lap);

    /// Mark racer as finished
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void SetRacerFinished(const FGuid& RacerId, float FinishTime);

    /// Mark racer as wrecked/eliminated
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void SetRacerWrecked(const FGuid& RacerId);

    /// Get current state for a racer
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    FMGRacerState GetRacerState(const FGuid& RacerId) const;

    /// Get all racer states
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    TArray<FMGRacerState> GetAllRacerStates() const;

    /// Get player's racer state
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    FMGRacerState GetPlayerState() const;

    /// Get current leader's state
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    FMGRacerState GetLeaderState() const;

    // ==========================================
    // CONFIGURATION
    // Adjust director behavior
    // ==========================================

    /// Set the director's intervention style
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetDirectorStyle(EMGDirectorStyle Style);

    /// Configure rubber-banding parameters
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetRubberBandConfig(const FMGRubberBandConfig& Config);

    /// Configure drama detection parameters
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetDramaConfig(const FMGDramaConfig& Config);

    /// Configure race pacing parameters
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetPacingConfig(const FMGRacePacingConfig& Config);

    /// Set AI difficulty configuration
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetAIDifficulty(const FMGAIDifficultyConfig& Config);

    /// Get current director style
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Config")
    EMGDirectorStyle GetDirectorStyle() const;

    /// Get current rubber-band configuration
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Config")
    FMGRubberBandConfig GetRubberBandConfig() const;

    /// Get current AI difficulty configuration
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Config")
    FMGAIDifficultyConfig GetAIDifficulty() const;

    // ==========================================
    // DIRECTOR UPDATE
    // Call every frame during race
    // ==========================================

    /// Main update function - processes all director logic
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void UpdateDirector(float MGDeltaTime);

    // ==========================================
    // STATE QUERIES
    // Get current director status
    // ==========================================

    /// Get complete director state snapshot
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    FMGDirectorState GetDirectorState() const;

    /// Get current race phase
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    EMGRacePhase GetCurrentPhase() const;

    /// Get current tension level
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    EMGRaceTension GetTensionLevel() const;

    /// Get overall race progress (0.0 to 1.0)
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    float GetRaceProgress() const;

    /// Get raw tension score
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    float GetTensionScore() const;

    /// Get total lead changes
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    int32 GetLeadChanges() const;

    // ==========================================
    // MODIFIERS
    // Query rubber-banding adjustments
    // ==========================================

    /// Get speed modifier for racer (1.0 = normal, >1 = boost, <1 = reduction)
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    float GetSpeedModifier(const FGuid& RacerId) const;

    /// Get handling modifier for racer
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    float GetHandlingModifier(const FGuid& RacerId) const;

    /// Get nitro recharge modifier for racer
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    float GetNitroRechargeModifier(const FGuid& RacerId) const;

    /// Get recommended AI behavior for racer
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    EMGAIBehaviorState GetRecommendedBehavior(const FGuid& RacerId) const;

    // ==========================================
    // AI ASSISTANCE
    // Direct AI behavior control
    // ==========================================

    /// Request AI make a mistake (spin, missed apex, etc.)
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|AI")
    void RequestMistake(const FGuid& RacerId, float Severity);

    /// Set aggression level for AI racer
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|AI")
    void SetRacerAggression(const FGuid& RacerId, float Aggression);

    /// Mark racer as story/career rival
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|AI")
    void DesignateRival(const FGuid& RacerId, bool bIsRival);

    // ==========================================
    // EVENT RECORDING
    // Track race events for statistics
    // ==========================================

    /// Record a takedown (racer A wrecks racer B)
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Events")
    void RecordTakedown(const FGuid& AttackerId, const FGuid& VictimId);

    /// Record a near-miss with traffic or obstacle
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Events")
    void RecordNearMiss(const FGuid& RacerId);

    /// Record a perfect lap with optimal racing line
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Events")
    void RecordPerfectLap(const FGuid& RacerId, float LapTime);

    /// Get all dramatic events from this race
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Events")
    TArray<FMGRaceEvent> GetDramaticEvents() const;

    /// Get current active dramatic moment
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Events")
    EMGDramaticMoment GetCurrentMoment() const;

    // ==========================================
    // STATISTICS
    // Query race statistics
    // ==========================================

    /// Get comprehensive race statistics
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Stats")
    FMGRaceStatistics GetRaceStatistics() const;

    /// Get finish order (array of racer GUIDs)
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Stats")
    TArray<FGuid> GetFinishOrder() const;

    // ==========================================
    // DIFFICULTY PRESETS
    // Preset difficulty configurations
    // ==========================================

    /// Set difficulty from preset (0=Easy, 4=Legendary)
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Difficulty")
    void SetDifficultyPreset(int32 Level);

    /// Get difficulty preset configuration
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Difficulty")
    FMGAIDifficultyConfig GetDifficultyPreset(int32 Level) const;

    // ==========================================
    // EVENT DELEGATES
    // Subscribe to race events
    // ==========================================

    /// Fires when race phase changes
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRacePhaseChanged OnRacePhaseChanged;

    /// Fires when dramatic moment is detected
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnDramaticMoment OnDramaticMoment;

    /// Fires when race lead changes
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnLeadChange OnLeadChange;

    /// Fires when any position changes
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnPositionChange OnPositionChange;

    /// Fires when tension level changes
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnTensionChanged OnTensionChanged;

    /// Fires when rubber-banding applied
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRubberBandApplied OnRubberBandApplied;

    /// Fires when race finishes
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRaceFinished OnRaceFinished;

    /// Fires when individual racer finishes
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRacerFinished OnRacerFinished;

protected:
    // ==========================================
    // RACE STATE
    // ==========================================

    /// Is race currently active
    UPROPERTY()
    bool bRaceActive;

    /// Total laps for this race
    UPROPERTY()
    int32 TotalLaps;

    /// Track length in meters
    UPROPERTY()
    float TrackLength;

    /// Elapsed race time
    UPROPERTY()
    float RaceTime;

    /// Current race phase
    UPROPERTY()
    EMGRacePhase CurrentPhase;

    // ==========================================
    // CONFIGURATION
    // ==========================================

    /// Current director intervention style
    UPROPERTY()
    EMGDirectorStyle DirectorStyle;

    /// Rubber-band configuration
    UPROPERTY()
    FMGRubberBandConfig RubberBandConfig;

    /// Drama detection configuration
    UPROPERTY()
    FMGDramaConfig DramaConfig;

    /// Race pacing configuration
    UPROPERTY()
    FMGRacePacingConfig PacingConfig;

    /// AI difficulty configuration
    UPROPERTY()
    FMGAIDifficultyConfig DifficultyConfig;

    // ==========================================
    // RACER TRACKING
    // ==========================================

    /// All racer states keyed by GUID
    UPROPERTY()
    TMap<FGuid, FMGRacerState> RacerStates;

    /// The player's racer GUID
    UPROPERTY()
    FGuid PlayerRacerId;

    /// Current race leader's GUID
    UPROPERTY()
    FGuid CurrentLeaderId;

    /// Racers in finish order
    UPROPERTY()
    TArray<FGuid> FinishOrder;

    // ==========================================
    // DIRECTOR STATE
    // ==========================================

    /// Current tension score (raw)
    UPROPERTY()
    float TensionScore;

    /// Current tension level (enum)
    UPROPERTY()
    EMGRaceTension TensionLevel;

    /// Current dramatic moment
    UPROPERTY()
    EMGDramaticMoment CurrentMoment;

    /// Total lead changes
    UPROPERTY()
    int32 LeadChanges;

    /// Time of last dramatic moment
    UPROPERTY()
    float LastDramaTime;

    // ==========================================
    // EVENT TRACKING
    // ==========================================

    /// All dramatic events this race
    UPROPERTY()
    TArray<FMGRaceEvent> DramaticEvents;

    // ==========================================
    // STATISTICS
    // ==========================================

    /// Race statistics
    UPROPERTY()
    FMGRaceStatistics RaceStats;

    // ==========================================
    // DIFFICULTY PRESETS
    // ==========================================

    /// Array of difficulty preset configurations
    UPROPERTY()
    TArray<FMGAIDifficultyConfig> DifficultyPresets;

    // ==========================================
    // INTERNAL METHODS
    // ==========================================

    /// Update race phase based on progress
    void UpdateRacePhase();

    /// Update tension score and level
    void UpdateTension();

    /// Update rubber-band modifiers for all racers
    void UpdateRubberBanding();

    /// Update AI behavior recommendations
    void UpdateAIBehaviors();

    /// Check for and trigger dramatic moments
    void CheckDramaticMoments();

    /// Calculate gaps between all racers
    void CalculateGaps();

    /// Update race statistics
    void UpdateStatistics();

    /// Transition to new race phase
    void SetRacePhase(EMGRacePhase NewPhase);

    /// Transition to new tension level
    void SetTensionLevel(EMGRaceTension NewLevel);

    /// Trigger a dramatic moment event
    void TriggerDramaticMoment(EMGDramaticMoment Moment, const FGuid& PrimaryRacer, const FGuid& SecondaryRacer = FGuid());

    /// Calculate rubber-band modifier for a racer
    float CalculateRubberBandModifier(const FMGRacerState& Racer) const;

    /// Determine appropriate AI behavior for racer
    EMGAIBehaviorState DetermineAIBehavior(const FMGRacerState& Racer) const;

    /// Initialize difficulty preset array
    void InitializeDifficultyPresets();

    /// Apply director style to configuration
    void ApplyDirectorStyle();
};

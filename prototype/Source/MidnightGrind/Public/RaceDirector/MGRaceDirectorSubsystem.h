// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceDirectorSubsystem.generated.h"

// Forward declarations
class UMGRaceDirectorSubsystem;

/**
 * Race director AI style for pacing control
 */
UENUM(BlueprintType)
enum class EMGDirectorStyle : uint8
{
    Authentic       UMETA(DisplayName = "Authentic"),      // Minimal intervention, pure skill
    Competitive     UMETA(DisplayName = "Competitive"),    // Moderate rubber-banding
    Dramatic        UMETA(DisplayName = "Dramatic"),       // Maximize close finishes
    Arcade          UMETA(DisplayName = "Arcade"),         // Heavy catch-up, fun first
    Simulation      UMETA(DisplayName = "Simulation"),     // No assistance, realistic AI
    Balanced        UMETA(DisplayName = "Balanced")        // Adaptive to player skill
};

/**
 * Current race phase for pacing
 */
UENUM(BlueprintType)
enum class EMGRacePhase : uint8
{
    PreRace         UMETA(DisplayName = "Pre-Race"),
    Start           UMETA(DisplayName = "Start"),
    EarlyRace       UMETA(DisplayName = "Early Race"),
    MidRace         UMETA(DisplayName = "Mid Race"),
    LateRace        UMETA(DisplayName = "Late Race"),
    FinalLap        UMETA(DisplayName = "Final Lap"),
    PhotoFinish     UMETA(DisplayName = "Photo Finish"),
    Finished        UMETA(DisplayName = "Finished")
};

/**
 * Dramatic moment type for events
 */
UENUM(BlueprintType)
enum class EMGDramaticMoment : uint8
{
    None            UMETA(DisplayName = "None"),
    CloseRace       UMETA(DisplayName = "Close Race"),
    Comeback        UMETA(DisplayName = "Comeback"),
    LeadChange      UMETA(DisplayName = "Lead Change"),
    PhotoFinish     UMETA(DisplayName = "Photo Finish"),
    Underdog        UMETA(DisplayName = "Underdog Victory"),
    Dominance       UMETA(DisplayName = "Dominant Performance"),
    Rivalry         UMETA(DisplayName = "Rivalry Moment"),
    WreckAvoidance  UMETA(DisplayName = "Wreck Avoidance"),
    PerfectLap      UMETA(DisplayName = "Perfect Lap")
};

/**
 * Rubber-band intensity level
 */
UENUM(BlueprintType)
enum class EMGRubberBandLevel : uint8
{
    None            UMETA(DisplayName = "None"),
    VeryLight       UMETA(DisplayName = "Very Light"),
    Light           UMETA(DisplayName = "Light"),
    Moderate        UMETA(DisplayName = "Moderate"),
    Strong          UMETA(DisplayName = "Strong"),
    VeryStrong      UMETA(DisplayName = "Very Strong")
};

/**
 * AI behavior state
 */
UENUM(BlueprintType)
enum class EMGAIBehaviorState : uint8
{
    Normal          UMETA(DisplayName = "Normal"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Defensive       UMETA(DisplayName = "Defensive"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Blocking        UMETA(DisplayName = "Blocking"),
    CatchUp         UMETA(DisplayName = "Catch Up"),
    SlowDown        UMETA(DisplayName = "Slow Down"),
    Mistake         UMETA(DisplayName = "Making Mistake"),
    Recovery        UMETA(DisplayName = "Recovery")
};

/**
 * Position adjustment type
 */
UENUM(BlueprintType)
enum class EMGPositionAdjustment : uint8
{
    None            UMETA(DisplayName = "None"),
    SpeedBoost      UMETA(DisplayName = "Speed Boost"),
    SpeedReduction  UMETA(DisplayName = "Speed Reduction"),
    BetterHandling  UMETA(DisplayName = "Better Handling"),
    WorseHandling   UMETA(DisplayName = "Worse Handling"),
    NitroBonus      UMETA(DisplayName = "Nitro Bonus"),
    MistakeProne    UMETA(DisplayName = "Mistake Prone")
};

/**
 * Race tension level
 */
UENUM(BlueprintType)
enum class EMGRaceTension : uint8
{
    Calm            UMETA(DisplayName = "Calm"),
    Mild            UMETA(DisplayName = "Mild"),
    Moderate        UMETA(DisplayName = "Moderate"),
    Intense         UMETA(DisplayName = "Intense"),
    Extreme         UMETA(DisplayName = "Extreme")
};

/**
 * Racer state for director tracking
 */
USTRUCT(BlueprintType)
struct FMGRacerState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    FGuid RacerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    FString RacerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bIsPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 CurrentPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 StartingPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 BestPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 WorstPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 CurrentLap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float RaceProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float DistanceToLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float DistanceToAhead;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float DistanceToBehind;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float SpeedModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float HandlingModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    EMGAIBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    EMGPositionAdjustment CurrentAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 PositionChanges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 Takedowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    int32 TimesWrecked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float SkillRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float AggresionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float PerformanceRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bIsRival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    bool bHasFinished;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racer")
    float FinishTime;

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

/**
 * Rubber-band configuration
 */
USTRUCT(BlueprintType)
struct FMGRubberBandConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    EMGRubberBandLevel Level;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float MaxSpeedBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float MaxSpeedReduction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float ActivationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float RampUpTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float HandlingBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    float NitroRechargeBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    bool bAffectsPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    bool bAffectsAI;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RubberBand")
    bool bScaleWithPosition;

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

/**
 * Drama configuration for exciting moments
 */
USTRUCT(BlueprintType)
struct FMGDramaConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float CloseRaceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float PhotoFinishWindow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float ComebackThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float LeadChangeWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float TensionBuildupRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    float MinDramaCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    bool bEnableDramaticMoments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    bool bEnableRivalrySystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drama")
    bool bEnableUnderdogBonus;

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

/**
 * AI difficulty configuration
 */
USTRUCT(BlueprintType)
struct FMGAIDifficultyConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    FString DifficultyName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float SpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float ReactionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float MistakeFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float AggressionBase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float RacingLineOptimality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float NitroUsageEfficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float DriftProficiency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float TrafficAvoidance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    float RecoverySpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Difficulty")
    EMGRubberBandLevel RubberBandLevel;

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

/**
 * Race event for dramatic moments
 */
USTRUCT(BlueprintType)
struct FMGRaceEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGuid EventId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    EMGDramaticMoment MomentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGuid PrimaryRacerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FGuid SecondaryRacerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    int32 Lap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
    FString Description;

    FMGRaceEvent()
        : MomentType(EMGDramaticMoment::None)
        , Timestamp(0.0f)
        , Lap(1)
        , Intensity(0.0f)
    {}
};

/**
 * Race pacing configuration
 */
USTRUCT(BlueprintType)
struct FMGRacePacingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float EarlyRacePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float MidRacePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float LateRacePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float FinalLapIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float StartChaosWindow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float MidRaceSettleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pacing")
    float EndGamePushTime;

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

/**
 * Race director state summary
 */
USTRUCT(BlueprintType)
struct FMGDirectorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    EMGRacePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    EMGRaceTension TensionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    EMGDramaticMoment CurrentMoment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float RaceProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float TensionScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    int32 LeadChanges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float AverageGap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    float PlayerPerformance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    bool bIsCloseRace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Director")
    bool bPhotoFinishPossible;

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

/**
 * Race statistics for analysis
 */
USTRUCT(BlueprintType)
struct FMGRaceStatistics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalRacers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 ActiveRacers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 FinishedRacers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 WreckedRacers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalLeadChanges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalPositionChanges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalTakedowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 TotalDramaticMoments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AverageSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float FastestLap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SlowestLap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float WinningMargin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ClosestGap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float RaceTime;

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

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRacePhaseChanged, EMGRacePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDramaticMoment, const FMGRaceEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnLeadChange, const FGuid&, NewLeaderId, int32, TotalChanges);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnPositionChange, const FGuid&, RacerId, int32, NewPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTensionChanged, EMGRaceTension, NewTension);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRubberBandApplied, const FGuid&, RacerId, float, Modifier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnRaceFinished, const FMGRaceStatistics&, Statistics);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnRacerFinished, const FGuid&, RacerId, int32, Position);

/**
 * Race Director Subsystem
 *
 * Controls AI race pacing, rubber-banding, dramatic moments, and overall
 * race experience. Manages tension, creates exciting finishes, and ensures
 * engaging gameplay through intelligent race direction.
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

    // Race lifecycle
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void InitializeRace(int32 TotalLaps, float TrackLength);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void StartRace();

    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void EndRace();

    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void ResetRace();

    UFUNCTION(BlueprintPure, Category = "RaceDirector")
    bool IsRaceActive() const;

    // Racer management
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    FGuid RegisterRacer(const FString& RacerName, bool bIsPlayer, int32 StartPosition);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void UnregisterRacer(const FGuid& RacerId);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void UpdateRacerState(const FGuid& RacerId, int32 Position, float Speed, float Progress);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void SetRacerLap(const FGuid& RacerId, int32 Lap);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void SetRacerFinished(const FGuid& RacerId, float FinishTime);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Racers")
    void SetRacerWrecked(const FGuid& RacerId);

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    FMGRacerState GetRacerState(const FGuid& RacerId) const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    TArray<FMGRacerState> GetAllRacerStates() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    FMGRacerState GetPlayerState() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Racers")
    FMGRacerState GetLeaderState() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetDirectorStyle(EMGDirectorStyle Style);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetRubberBandConfig(const FMGRubberBandConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetDramaConfig(const FMGDramaConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetPacingConfig(const FMGRacePacingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Config")
    void SetAIDifficulty(const FMGAIDifficultyConfig& Config);

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Config")
    EMGDirectorStyle GetDirectorStyle() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Config")
    FMGRubberBandConfig GetRubberBandConfig() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Config")
    FMGAIDifficultyConfig GetAIDifficulty() const;

    // Director update (call per frame during race)
    UFUNCTION(BlueprintCallable, Category = "RaceDirector")
    void UpdateDirector(float DeltaTime);

    // State queries
    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    FMGDirectorState GetDirectorState() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    EMGRacePhase GetCurrentPhase() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    EMGRaceTension GetTensionLevel() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    float GetRaceProgress() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    float GetTensionScore() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|State")
    int32 GetLeadChanges() const;

    // Modifiers
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    float GetSpeedModifier(const FGuid& RacerId) const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    float GetHandlingModifier(const FGuid& RacerId) const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    float GetNitroRechargeModifier(const FGuid& RacerId) const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Modifiers")
    EMGAIBehaviorState GetRecommendedBehavior(const FGuid& RacerId) const;

    // AI assistance
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|AI")
    void RequestMistake(const FGuid& RacerId, float Severity);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|AI")
    void SetRacerAggression(const FGuid& RacerId, float Aggression);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|AI")
    void DesignateRival(const FGuid& RacerId, bool bIsRival);

    // Events
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Events")
    void RecordTakedown(const FGuid& AttackerId, const FGuid& VictimId);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Events")
    void RecordNearMiss(const FGuid& RacerId);

    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Events")
    void RecordPerfectLap(const FGuid& RacerId, float LapTime);

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Events")
    TArray<FMGRaceEvent> GetDramaticEvents() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Events")
    EMGDramaticMoment GetCurrentMoment() const;

    // Statistics
    UFUNCTION(BlueprintPure, Category = "RaceDirector|Stats")
    FMGRaceStatistics GetRaceStatistics() const;

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Stats")
    TArray<FGuid> GetFinishOrder() const;

    // Difficulty presets
    UFUNCTION(BlueprintCallable, Category = "RaceDirector|Difficulty")
    void SetDifficultyPreset(int32 Level);

    UFUNCTION(BlueprintPure, Category = "RaceDirector|Difficulty")
    FMGAIDifficultyConfig GetDifficultyPreset(int32 Level) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRacePhaseChanged OnRacePhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnDramaticMoment OnDramaticMoment;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnLeadChange OnLeadChange;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnPositionChange OnPositionChange;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnTensionChanged OnTensionChanged;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRubberBandApplied OnRubberBandApplied;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRaceFinished OnRaceFinished;

    UPROPERTY(BlueprintAssignable, Category = "RaceDirector|Events")
    FMGOnRacerFinished OnRacerFinished;

protected:
    // Race state
    UPROPERTY()
    bool bRaceActive;

    UPROPERTY()
    int32 TotalLaps;

    UPROPERTY()
    float TrackLength;

    UPROPERTY()
    float RaceTime;

    UPROPERTY()
    EMGRacePhase CurrentPhase;

    // Configuration
    UPROPERTY()
    EMGDirectorStyle DirectorStyle;

    UPROPERTY()
    FMGRubberBandConfig RubberBandConfig;

    UPROPERTY()
    FMGDramaConfig DramaConfig;

    UPROPERTY()
    FMGRacePacingConfig PacingConfig;

    UPROPERTY()
    FMGAIDifficultyConfig DifficultyConfig;

    // Racer tracking
    UPROPERTY()
    TMap<FGuid, FMGRacerState> RacerStates;

    UPROPERTY()
    FGuid PlayerRacerId;

    UPROPERTY()
    FGuid CurrentLeaderId;

    UPROPERTY()
    TArray<FGuid> FinishOrder;

    // Director state
    UPROPERTY()
    float TensionScore;

    UPROPERTY()
    EMGRaceTension TensionLevel;

    UPROPERTY()
    EMGDramaticMoment CurrentMoment;

    UPROPERTY()
    int32 LeadChanges;

    UPROPERTY()
    float LastDramaTime;

    // Event tracking
    UPROPERTY()
    TArray<FMGRaceEvent> DramaticEvents;

    // Statistics
    UPROPERTY()
    FMGRaceStatistics RaceStats;

    // Difficulty presets
    UPROPERTY()
    TArray<FMGAIDifficultyConfig> DifficultyPresets;

    // Helper functions
    void UpdateRacePhase();
    void UpdateTension();
    void UpdateRubberBanding();
    void UpdateAIBehaviors();
    void CheckDramaticMoments();
    void CalculateGaps();
    void UpdateStatistics();

    void SetRacePhase(EMGRacePhase NewPhase);
    void SetTensionLevel(EMGRaceTension NewLevel);
    void TriggerDramaticMoment(EMGDramaticMoment Moment, const FGuid& PrimaryRacer, const FGuid& SecondaryRacer = FGuid());

    float CalculateRubberBandModifier(const FMGRacerState& Racer) const;
    EMGAIBehaviorState DetermineAIBehavior(const FMGRacerState& Racer) const;

    void InitializeDifficultyPresets();
    void ApplyDirectorStyle();
};

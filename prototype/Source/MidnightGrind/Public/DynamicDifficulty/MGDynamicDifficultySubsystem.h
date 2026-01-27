// Copyright Midnight Grind. All Rights Reserved.

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

UENUM(BlueprintType)
enum class EDifficultyPreset : uint8
{
    VeryEasy,
    Easy,
    Normal,
    Hard,
    VeryHard,
    Extreme,
    Adaptive,
    Custom
};

UENUM(BlueprintType)
enum class EPerformanceMetric : uint8
{
    RacePosition,
    LapTime,
    DriftScore,
    CollisionCount,
    NitroUsage,
    ShortcutUsage,
    OvertakeCount,
    WinRate,
    FinishRate,
    AverageSpeed
};

UENUM(BlueprintType)
enum class EAIBehaviorProfile : uint8
{
    Defensive,
    Balanced,
    Aggressive,
    Tactical,
    Unpredictable,
    Rubberband,
    TrainingWheel
};

UENUM(BlueprintType)
enum class EDifficultyAspect : uint8
{
    AISpeed,
    AIAggression,
    AIErrorRate,
    TrafficDensity,
    ObstacleDensity,
    CatchUpAssist,
    SlipstreamBoost,
    NitroRecharge,
    CollisionPenalty,
    TimePressure
};

UENUM(BlueprintType)
enum class ESkillLevel : uint8
{
    Beginner,
    Novice,
    Intermediate,
    Advanced,
    Expert,
    Master,
    Legend
};

UENUM(BlueprintType)
enum class EAdaptationSpeed : uint8
{
    Instant,
    Fast,
    Medium,
    Slow,
    Gradual
};

UENUM(BlueprintType)
enum class EAssistType : uint8
{
    Steering,
    Braking,
    Drifting,
    NitroTiming,
    RacingLine,
    CollisionAvoidance,
    ShortcutHints,
    OpponentTracking
};

UENUM(BlueprintType)
enum class EFrustrationLevel : uint8
{
    Relaxed,
    Comfortable,
    Challenged,
    Frustrated,
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

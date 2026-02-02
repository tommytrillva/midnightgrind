// Copyright Midnight Grind. All Rights Reserved.

// MGDynamicDifficultySubsystem.cpp
// Dynamic Difficulty Adjustment System - Implementation
// Midnight Grind - Y2K Arcade Street Racing

#include "DynamicDifficulty/MGDynamicDifficultySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

void UMGDynamicDifficultySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentPreset = EDifficultyPreset::Normal;
    bAdaptiveDifficultyEnabled = true;
    CurrentAdaptationSpeed = EAdaptationSpeed::Medium;
    AdaptationSensitivity = 0.5f;
    MinDifficultyBound = 0.3f;
    MaxDifficultyBound = 1.8f;
    CurrentFrustrationLevel = EFrustrationLevel::Comfortable;
    bFrustrationResponseEnabled = true;
    bGlobalRubberbandingEnabled = false;
    GlobalRubberbandingStrength = 0.3f;

    CurrentRacePosition = 1;
    CurrentGapToLeader = 0.0f;
    CurrentRaceProgress = 0.0f;
    RaceCollisionCount = 0;

    InitializePresetProfiles();
    LoadPlayerData();

    // Set initial modifiers from normal preset
    if (PresetProfiles.Contains(EDifficultyPreset::Normal))
    {
        CurrentModifiers = PresetProfiles[EDifficultyPreset::Normal].Modifiers;
        CurrentAssists = PresetProfiles[EDifficultyPreset::Normal].Assists;
    }

    UE_LOG(LogTemp, Log, TEXT("MGDynamicDifficultySubsystem initialized"));
}

void UMGDynamicDifficultySubsystem::Deinitialize()
{
    SavePlayerData();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AdaptiveUpdateTimer);
    }

    Super::Deinitialize();
}

void UMGDynamicDifficultySubsystem::InitializePresetProfiles()
{
    // Very Easy Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Very Easy");
        Profile.BasePreset = EDifficultyPreset::VeryEasy;
        Profile.Modifiers.AISpeedMultiplier = 0.7f;
        Profile.Modifiers.AIAggressionLevel = 0.1f;
        Profile.Modifiers.AIErrorRate = 0.3f;
        Profile.Modifiers.AIReactionTimeMultiplier = 1.5f;
        Profile.Modifiers.TrafficDensityMultiplier = 0.3f;
        Profile.Modifiers.ObstacleDensityMultiplier = 0.5f;
        Profile.Modifiers.CatchUpAssistStrength = 0.8f;
        Profile.Modifiers.SlipstreamBoostMultiplier = 1.5f;
        Profile.Modifiers.NitroRechargeMultiplier = 1.5f;
        Profile.Modifiers.CollisionPenaltyMultiplier = 0.5f;
        Profile.Modifiers.TimePressureMultiplier = 0.7f;
        Profile.Modifiers.RewardMultiplier = 0.7f;
        Profile.Modifiers.XPMultiplier = 0.8f;

        Profile.Assists.bSteeringAssist = true;
        Profile.Assists.SteeringAssistStrength = 0.8f;
        Profile.Assists.bBrakingAssist = true;
        Profile.Assists.BrakingAssistStrength = 0.8f;
        Profile.Assists.bDriftAssist = true;
        Profile.Assists.DriftAssistStrength = 0.8f;
        Profile.Assists.bNitroTimingAssist = true;
        Profile.Assists.bRacingLineDisplay = true;
        Profile.Assists.bCollisionPrediction = true;
        Profile.Assists.bShortcutHints = true;
        Profile.Assists.bAutoRecovery = true;
        Profile.Assists.AutoRecoveryDelay = 2.0f;
        Profile.Assists.bRewindFeature = true;
        Profile.Assists.RewindCharges = 5;

        Profile.MinDifficulty = 0.1f;
        Profile.MaxDifficulty = 0.8f;

        PresetProfiles.Add(EDifficultyPreset::VeryEasy, Profile);
    }

    // Easy Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Easy");
        Profile.BasePreset = EDifficultyPreset::Easy;
        Profile.Modifiers.AISpeedMultiplier = 0.85f;
        Profile.Modifiers.AIAggressionLevel = 0.25f;
        Profile.Modifiers.AIErrorRate = 0.2f;
        Profile.Modifiers.AIReactionTimeMultiplier = 1.3f;
        Profile.Modifiers.TrafficDensityMultiplier = 0.5f;
        Profile.Modifiers.ObstacleDensityMultiplier = 0.7f;
        Profile.Modifiers.CatchUpAssistStrength = 0.5f;
        Profile.Modifiers.SlipstreamBoostMultiplier = 1.3f;
        Profile.Modifiers.NitroRechargeMultiplier = 1.3f;
        Profile.Modifiers.CollisionPenaltyMultiplier = 0.7f;
        Profile.Modifiers.TimePressureMultiplier = 0.85f;
        Profile.Modifiers.RewardMultiplier = 0.85f;
        Profile.Modifiers.XPMultiplier = 0.9f;

        Profile.Assists.bSteeringAssist = true;
        Profile.Assists.SteeringAssistStrength = 0.5f;
        Profile.Assists.bBrakingAssist = true;
        Profile.Assists.BrakingAssistStrength = 0.5f;
        Profile.Assists.bDriftAssist = true;
        Profile.Assists.DriftAssistStrength = 0.5f;
        Profile.Assists.bRacingLineDisplay = true;
        Profile.Assists.bCollisionPrediction = true;
        Profile.Assists.bAutoRecovery = true;
        Profile.Assists.AutoRecoveryDelay = 3.0f;
        Profile.Assists.bRewindFeature = true;
        Profile.Assists.RewindCharges = 3;

        Profile.MinDifficulty = 0.3f;
        Profile.MaxDifficulty = 1.0f;

        PresetProfiles.Add(EDifficultyPreset::Easy, Profile);
    }

    // Normal Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Normal");
        Profile.BasePreset = EDifficultyPreset::Normal;
        Profile.Modifiers.AISpeedMultiplier = 1.0f;
        Profile.Modifiers.AIAggressionLevel = 0.5f;
        Profile.Modifiers.AIErrorRate = 0.1f;
        Profile.Modifiers.AIReactionTimeMultiplier = 1.0f;
        Profile.Modifiers.TrafficDensityMultiplier = 1.0f;
        Profile.Modifiers.ObstacleDensityMultiplier = 1.0f;
        Profile.Modifiers.CatchUpAssistStrength = 0.2f;
        Profile.Modifiers.SlipstreamBoostMultiplier = 1.0f;
        Profile.Modifiers.NitroRechargeMultiplier = 1.0f;
        Profile.Modifiers.CollisionPenaltyMultiplier = 1.0f;
        Profile.Modifiers.TimePressureMultiplier = 1.0f;
        Profile.Modifiers.RewardMultiplier = 1.0f;
        Profile.Modifiers.XPMultiplier = 1.0f;

        Profile.Assists.bSteeringAssist = false;
        Profile.Assists.bBrakingAssist = false;
        Profile.Assists.bDriftAssist = false;
        Profile.Assists.bRacingLineDisplay = false;
        Profile.Assists.bAutoRecovery = true;
        Profile.Assists.AutoRecoveryDelay = 4.0f;
        Profile.Assists.bRewindFeature = false;

        Profile.MinDifficulty = 0.5f;
        Profile.MaxDifficulty = 1.5f;

        PresetProfiles.Add(EDifficultyPreset::Normal, Profile);
    }

    // Hard Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Hard");
        Profile.BasePreset = EDifficultyPreset::Hard;
        Profile.Modifiers.AISpeedMultiplier = 1.15f;
        Profile.Modifiers.AIAggressionLevel = 0.7f;
        Profile.Modifiers.AIErrorRate = 0.05f;
        Profile.Modifiers.AIReactionTimeMultiplier = 0.8f;
        Profile.Modifiers.TrafficDensityMultiplier = 1.3f;
        Profile.Modifiers.ObstacleDensityMultiplier = 1.3f;
        Profile.Modifiers.CatchUpAssistStrength = 0.0f;
        Profile.Modifiers.SlipstreamBoostMultiplier = 0.8f;
        Profile.Modifiers.NitroRechargeMultiplier = 0.8f;
        Profile.Modifiers.CollisionPenaltyMultiplier = 1.3f;
        Profile.Modifiers.TimePressureMultiplier = 1.2f;
        Profile.Modifiers.RewardMultiplier = 1.3f;
        Profile.Modifiers.XPMultiplier = 1.2f;

        Profile.Assists.bAutoRecovery = true;
        Profile.Assists.AutoRecoveryDelay = 5.0f;

        Profile.MinDifficulty = 0.8f;
        Profile.MaxDifficulty = 1.8f;

        PresetProfiles.Add(EDifficultyPreset::Hard, Profile);
    }

    // Very Hard Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Very Hard");
        Profile.BasePreset = EDifficultyPreset::VeryHard;
        Profile.Modifiers.AISpeedMultiplier = 1.25f;
        Profile.Modifiers.AIAggressionLevel = 0.85f;
        Profile.Modifiers.AIErrorRate = 0.02f;
        Profile.Modifiers.AIReactionTimeMultiplier = 0.6f;
        Profile.Modifiers.TrafficDensityMultiplier = 1.5f;
        Profile.Modifiers.ObstacleDensityMultiplier = 1.5f;
        Profile.Modifiers.CatchUpAssistStrength = 0.0f;
        Profile.Modifiers.SlipstreamBoostMultiplier = 0.6f;
        Profile.Modifiers.NitroRechargeMultiplier = 0.7f;
        Profile.Modifiers.CollisionPenaltyMultiplier = 1.5f;
        Profile.Modifiers.TimePressureMultiplier = 1.4f;
        Profile.Modifiers.RewardMultiplier = 1.5f;
        Profile.Modifiers.XPMultiplier = 1.4f;

        Profile.Assists.bAutoRecovery = false;

        Profile.MinDifficulty = 1.0f;
        Profile.MaxDifficulty = 2.0f;

        PresetProfiles.Add(EDifficultyPreset::VeryHard, Profile);
    }

    // Extreme Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Extreme");
        Profile.BasePreset = EDifficultyPreset::Extreme;
        Profile.Modifiers.AISpeedMultiplier = 1.4f;
        Profile.Modifiers.AIAggressionLevel = 1.0f;
        Profile.Modifiers.AIErrorRate = 0.0f;
        Profile.Modifiers.AIReactionTimeMultiplier = 0.5f;
        Profile.Modifiers.TrafficDensityMultiplier = 2.0f;
        Profile.Modifiers.ObstacleDensityMultiplier = 2.0f;
        Profile.Modifiers.CatchUpAssistStrength = 0.0f;
        Profile.Modifiers.SlipstreamBoostMultiplier = 0.5f;
        Profile.Modifiers.NitroRechargeMultiplier = 0.5f;
        Profile.Modifiers.CollisionPenaltyMultiplier = 2.0f;
        Profile.Modifiers.TimePressureMultiplier = 1.6f;
        Profile.Modifiers.RewardMultiplier = 2.0f;
        Profile.Modifiers.XPMultiplier = 1.8f;

        Profile.MinDifficulty = 1.5f;
        Profile.MaxDifficulty = 2.5f;
        Profile.bAllowDynamicAdjustment = false;

        PresetProfiles.Add(EDifficultyPreset::Extreme, Profile);
    }

    // Adaptive Profile
    {
        FDifficultyProfile Profile;
        Profile.ProfileName = TEXT("Adaptive");
        Profile.BasePreset = EDifficultyPreset::Adaptive;
        Profile.Modifiers = PresetProfiles[EDifficultyPreset::Normal].Modifiers;
        Profile.Assists = PresetProfiles[EDifficultyPreset::Normal].Assists;
        Profile.bAllowDynamicAdjustment = true;
        Profile.AdaptationSpeed = EAdaptationSpeed::Medium;
        Profile.AdaptationSensitivity = 0.6f;
        Profile.MinDifficulty = 0.3f;
        Profile.MaxDifficulty = 1.8f;

        PresetProfiles.Add(EDifficultyPreset::Adaptive, Profile);
    }
}

void UMGDynamicDifficultySubsystem::LoadPlayerData()
{
    // Initialize default player performance data
    PlayerPerformance.PlayerId = TEXT("LocalPlayer");
    PlayerPerformance.TotalRaces = 0;
    PlayerPerformance.TotalWins = 0;
    PlayerPerformance.TotalPodiums = 0;
    PlayerPerformance.TotalFinishes = 0;
    PlayerPerformance.WinRate = 0.0f;
    PlayerPerformance.PodiumRate = 0.0f;
    PlayerPerformance.FinishRate = 0.0f;
    PlayerPerformance.AveragePosition = 4.0f;
    PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Intermediate;
    PlayerPerformance.LastUpdated = FDateTime::Now();

    // Initialize stats
    DifficultyStats.CurrentDifficultyLevel = 1.0f;
    DifficultyStats.HistoricalWinRate = 0.0f;
    DifficultyStats.RecentWinRate = 0.0f;
    DifficultyStats.PlayerSatisfactionEstimate = 0.75f;
}

void UMGDynamicDifficultySubsystem::SavePlayerData()
{
    FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Difficulty");
    IFileManager::Get().MakeDirectory(*SaveDir, true);
    FString FilePath = SaveDir / TEXT("player_difficulty.dat");

    FBufferArchive SaveArchive;

    // Version for future compatibility
    int32 Version = 1;
    SaveArchive << Version;

    // Save current preset
    int32 PresetInt = static_cast<int32>(CurrentPreset);
    SaveArchive << PresetInt;

    // Save adaptive settings
    SaveArchive << bAdaptiveDifficultyEnabled;
    SaveArchive << AdaptationSensitivity;
    SaveArchive << MinDifficultyBound;
    SaveArchive << MaxDifficultyBound;

    // Save player performance metrics
    int32 SkillLevel = static_cast<int32>(PlayerPerformance.EstimatedSkillLevel);
    SaveArchive << SkillLevel;
    SaveArchive << PlayerPerformance.TotalRacesCompleted;
    SaveArchive << PlayerPerformance.TotalWins;
    SaveArchive << PlayerPerformance.TotalLosses;
    SaveArchive << PlayerPerformance.AverageFinishPosition;
    SaveArchive << PlayerPerformance.BestLapTimeDeviation;
    SaveArchive << PlayerPerformance.AverageRaceCompletion;
    SaveArchive << PlayerPerformance.CollisionRate;
    SaveArchive << PlayerPerformance.OffroadRate;
    SaveArchive << PlayerPerformance.ConsistencyScore;

    // Save difficulty stats
    SaveArchive << DifficultyStats.CurrentDifficultyLevel;
    SaveArchive << DifficultyStats.HistoricalWinRate;
    SaveArchive << DifficultyStats.RecentWinRate;
    SaveArchive << DifficultyStats.PlayerSatisfactionEstimate;

    // Save current modifiers
    SaveArchive << CurrentModifiers.AISpeedMultiplier;
    SaveArchive << CurrentModifiers.AIAggressionMultiplier;
    SaveArchive << CurrentModifiers.RubberBandingStrength;
    SaveArchive << CurrentModifiers.CatchUpAssistStrength;
    SaveArchive << CurrentModifiers.PlayerDamageMultiplier;
    SaveArchive << CurrentModifiers.AIErrorRate;

    // Write to file
    if (SaveArchive.Num() > 0)
    {
        FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
    }

    UE_LOG(LogTemp, Log, TEXT("Saved player difficulty data - Skill Level: %d, Races: %d"),
        SkillLevel, PlayerPerformance.TotalRacesCompleted);
}

// ============================================================================
// Difficulty Preset Management
// ============================================================================

void UMGDynamicDifficultySubsystem::SetDifficultyPreset(EDifficultyPreset Preset)
{
    EDifficultyPreset OldPreset = CurrentPreset;
    CurrentPreset = Preset;

    if (PresetProfiles.Contains(Preset))
    {
        const FDifficultyProfile& Profile = PresetProfiles[Preset];
        CurrentModifiers = Profile.Modifiers;
        CurrentAssists = Profile.Assists;
        MinDifficultyBound = Profile.MinDifficulty;
        MaxDifficultyBound = Profile.MaxDifficulty;
        bAdaptiveDifficultyEnabled = Profile.bAllowDynamicAdjustment && (Preset == EDifficultyPreset::Adaptive);
    }

    OnDifficultyChanged.Broadcast(OldPreset, Preset);
    UE_LOG(LogTemp, Log, TEXT("Difficulty preset changed to: %d"), static_cast<int32>(Preset));
}

FDifficultyProfile UMGDynamicDifficultySubsystem::GetProfileForPreset(EDifficultyPreset Preset) const
{
    if (PresetProfiles.Contains(Preset))
    {
        return PresetProfiles[Preset];
    }
    return FDifficultyProfile();
}

void UMGDynamicDifficultySubsystem::CreateCustomProfile(const FString& ProfileName, const FDifficultyProfile& Profile)
{
    FDifficultyProfile NewProfile = Profile;
    NewProfile.ProfileName = ProfileName;
    NewProfile.BasePreset = EDifficultyPreset::Custom;
    CustomProfiles.Add(ProfileName, NewProfile);
}

void UMGDynamicDifficultySubsystem::LoadCustomProfile(const FString& ProfileName)
{
    if (CustomProfiles.Contains(ProfileName))
    {
        const FDifficultyProfile& Profile = CustomProfiles[ProfileName];
        CurrentModifiers = Profile.Modifiers;
        CurrentAssists = Profile.Assists;
        CurrentPreset = EDifficultyPreset::Custom;
    }
}

TArray<FString> UMGDynamicDifficultySubsystem::GetAvailableCustomProfiles() const
{
    TArray<FString> ProfileNames;
    CustomProfiles.GetKeys(ProfileNames);
    return ProfileNames;
}

// ============================================================================
// Difficulty Modifiers
// ============================================================================

void UMGDynamicDifficultySubsystem::SetModifiers(const FDifficultyModifiers& NewModifiers)
{
    CurrentModifiers = NewModifiers;
    CurrentPreset = EDifficultyPreset::Custom;
}

void UMGDynamicDifficultySubsystem::AdjustModifier(EDifficultyAspect Aspect, float NewValue)
{
    float OldValue = GetModifierValue(Aspect);

    switch (Aspect)
    {
        case EDifficultyAspect::AISpeed:
            CurrentModifiers.AISpeedMultiplier = FMath::Clamp(NewValue, 0.5f, 2.0f);
            break;
        case EDifficultyAspect::AIAggression:
            CurrentModifiers.AIAggressionLevel = FMath::Clamp(NewValue, 0.0f, 1.0f);
            break;
        case EDifficultyAspect::AIErrorRate:
            CurrentModifiers.AIErrorRate = FMath::Clamp(NewValue, 0.0f, 0.5f);
            break;
        case EDifficultyAspect::TrafficDensity:
            CurrentModifiers.TrafficDensityMultiplier = FMath::Clamp(NewValue, 0.0f, 2.0f);
            break;
        case EDifficultyAspect::ObstacleDensity:
            CurrentModifiers.ObstacleDensityMultiplier = FMath::Clamp(NewValue, 0.0f, 2.0f);
            break;
        case EDifficultyAspect::CatchUpAssist:
            CurrentModifiers.CatchUpAssistStrength = FMath::Clamp(NewValue, 0.0f, 1.0f);
            break;
        case EDifficultyAspect::SlipstreamBoost:
            CurrentModifiers.SlipstreamBoostMultiplier = FMath::Clamp(NewValue, 0.5f, 2.0f);
            break;
        case EDifficultyAspect::NitroRecharge:
            CurrentModifiers.NitroRechargeMultiplier = FMath::Clamp(NewValue, 0.5f, 2.0f);
            break;
        case EDifficultyAspect::CollisionPenalty:
            CurrentModifiers.CollisionPenaltyMultiplier = FMath::Clamp(NewValue, 0.5f, 2.0f);
            break;
        case EDifficultyAspect::TimePressure:
            CurrentModifiers.TimePressureMultiplier = FMath::Clamp(NewValue, 0.5f, 2.0f);
            break;
    }

    FDifficultyAdjustment Adjustment = CreateAdjustment(Aspect, OldValue, NewValue, TEXT("Manual adjustment"));
    OnDifficultyAdjusted.Broadcast(Adjustment);
}

float UMGDynamicDifficultySubsystem::GetModifierValue(EDifficultyAspect Aspect) const
{
    switch (Aspect)
    {
        case EDifficultyAspect::AISpeed:
            return CurrentModifiers.AISpeedMultiplier;
        case EDifficultyAspect::AIAggression:
            return CurrentModifiers.AIAggressionLevel;
        case EDifficultyAspect::AIErrorRate:
            return CurrentModifiers.AIErrorRate;
        case EDifficultyAspect::TrafficDensity:
            return CurrentModifiers.TrafficDensityMultiplier;
        case EDifficultyAspect::ObstacleDensity:
            return CurrentModifiers.ObstacleDensityMultiplier;
        case EDifficultyAspect::CatchUpAssist:
            return CurrentModifiers.CatchUpAssistStrength;
        case EDifficultyAspect::SlipstreamBoost:
            return CurrentModifiers.SlipstreamBoostMultiplier;
        case EDifficultyAspect::NitroRecharge:
            return CurrentModifiers.NitroRechargeMultiplier;
        case EDifficultyAspect::CollisionPenalty:
            return CurrentModifiers.CollisionPenaltyMultiplier;
        case EDifficultyAspect::TimePressure:
            return CurrentModifiers.TimePressureMultiplier;
        default:
            return 1.0f;
    }
}

void UMGDynamicDifficultySubsystem::ResetModifiersToPreset()
{
    if (PresetProfiles.Contains(CurrentPreset))
    {
        CurrentModifiers = PresetProfiles[CurrentPreset].Modifiers;
    }
}

// ============================================================================
// Player Performance Tracking
// ============================================================================

void UMGDynamicDifficultySubsystem::RecordRaceResult(const FRaceAnalysis& Analysis)
{
    PlayerPerformance.TotalRaces++;

    if (Analysis.bFinished)
    {
        PlayerPerformance.TotalFinishes++;
    }

    if (Analysis.bWon)
    {
        PlayerPerformance.TotalWins++;
        AdaptiveHistory.ConsecutiveWins++;
        AdaptiveHistory.ConsecutiveLosses = 0;
    }
    else
    {
        AdaptiveHistory.ConsecutiveLosses++;
        AdaptiveHistory.ConsecutiveWins = 0;
    }

    if (Analysis.bPodium)
    {
        PlayerPerformance.TotalPodiums++;
    }

    // Update rates
    PlayerPerformance.WinRate = PlayerPerformance.TotalRaces > 0 ?
        static_cast<float>(PlayerPerformance.TotalWins) / static_cast<float>(PlayerPerformance.TotalRaces) : 0.0f;
    PlayerPerformance.PodiumRate = PlayerPerformance.TotalRaces > 0 ?
        static_cast<float>(PlayerPerformance.TotalPodiums) / static_cast<float>(PlayerPerformance.TotalRaces) : 0.0f;
    PlayerPerformance.FinishRate = PlayerPerformance.TotalRaces > 0 ?
        static_cast<float>(PlayerPerformance.TotalFinishes) / static_cast<float>(PlayerPerformance.TotalRaces) : 0.0f;

    // Update recent positions
    PlayerPerformance.RecentPositions.Add(static_cast<float>(Analysis.FinalPosition));
    if (PlayerPerformance.RecentPositions.Num() > 20)
    {
        PlayerPerformance.RecentPositions.RemoveAt(0);
    }

    // Calculate average position from recent races
    float TotalPosition = 0.0f;
    for (float Pos : PlayerPerformance.RecentPositions)
    {
        TotalPosition += Pos;
    }
    PlayerPerformance.AveragePosition = PlayerPerformance.RecentPositions.Num() > 0 ?
        TotalPosition / PlayerPerformance.RecentPositions.Num() : 4.0f;

    // Update lap times
    if (Analysis.BestLapTime > 0.0f)
    {
        PlayerPerformance.RecentLapTimes.Add(Analysis.BestLapTime);
        if (PlayerPerformance.RecentLapTimes.Num() > 20)
        {
            PlayerPerformance.RecentLapTimes.RemoveAt(0);
        }

        if (PlayerPerformance.BestLapTime == 0.0f || Analysis.BestLapTime < PlayerPerformance.BestLapTime)
        {
            PlayerPerformance.BestLapTime = Analysis.BestLapTime;
        }
    }

    // Update other stats
    PlayerPerformance.TotalOvertakes += Analysis.OvertakesMade;
    PlayerPerformance.TotalCollisions += Analysis.TotalCollisions;
    PlayerPerformance.TotalDriftScore += Analysis.TotalDriftScore;

    if (PlayerPerformance.TotalRaces > 0)
    {
        PlayerPerformance.CollisionRate = static_cast<float>(PlayerPerformance.TotalCollisions) / static_cast<float>(PlayerPerformance.TotalRaces);
        PlayerPerformance.AverageDriftScore = PlayerPerformance.TotalDriftScore / static_cast<float>(PlayerPerformance.TotalRaces);
    }

    // Store race analysis
    AdaptiveHistory.RecentRaces.Add(Analysis);
    if (AdaptiveHistory.RecentRaces.Num() > 50)
    {
        AdaptiveHistory.RecentRaces.RemoveAt(0);
    }

    // Calculate skill level
    CalculateSkillLevel();

    // Update stats
    DifficultyStats.HistoricalWinRate = PlayerPerformance.WinRate;

    // Calculate recent win rate (last 10 races)
    int32 RecentWins = 0;
    int32 RecentCount = FMath::Min(10, AdaptiveHistory.RecentRaces.Num());
    for (int32 i = AdaptiveHistory.RecentRaces.Num() - RecentCount; i < AdaptiveHistory.RecentRaces.Num(); i++)
    {
        if (AdaptiveHistory.RecentRaces[i].bWon)
        {
            RecentWins++;
        }
    }
    DifficultyStats.RecentWinRate = RecentCount > 0 ? static_cast<float>(RecentWins) / static_cast<float>(RecentCount) : 0.0f;

    PlayerPerformance.LastUpdated = FDateTime::Now();

    // Detect frustration
    EFrustrationLevel Frustration = DetectFrustrationLevel(Analysis);
    if (bFrustrationResponseEnabled && Frustration >= EFrustrationLevel::Frustrated)
    {
        RespondToFrustration(Frustration);
    }

    // Trigger adaptive adjustment if enabled
    if (bAdaptiveDifficultyEnabled)
    {
        TriggerAdaptiveAdjustment();
    }

    OnRaceAnalyzed.Broadcast(Analysis);
}

void UMGDynamicDifficultySubsystem::CalculateSkillLevel()
{
    ESkillLevel OldLevel = PlayerPerformance.EstimatedSkillLevel;

    // Calculate skill score based on various metrics
    float SkillScore = 0.0f;

    // Win rate contribution (0-30 points)
    SkillScore += PlayerPerformance.WinRate * 30.0f;

    // Podium rate contribution (0-20 points)
    SkillScore += PlayerPerformance.PodiumRate * 20.0f;

    // Average position contribution (0-20 points, inverted - lower is better)
    float PositionScore = FMath::Clamp(1.0f - ((PlayerPerformance.AveragePosition - 1.0f) / 7.0f), 0.0f, 1.0f);
    SkillScore += PositionScore * 20.0f;

    // Collision rate (0-15 points, inverted - lower is better)
    float CollisionScore = FMath::Clamp(1.0f - (PlayerPerformance.CollisionRate / 10.0f), 0.0f, 1.0f);
    SkillScore += CollisionScore * 15.0f;

    // Experience bonus (0-15 points)
    float ExpScore = FMath::Clamp(static_cast<float>(PlayerPerformance.TotalRaces) / 100.0f, 0.0f, 1.0f);
    SkillScore += ExpScore * 15.0f;

    // Determine skill level based on score
    if (SkillScore < 15.0f)
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Beginner;
    }
    else if (SkillScore < 30.0f)
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Novice;
    }
    else if (SkillScore < 45.0f)
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Intermediate;
    }
    else if (SkillScore < 60.0f)
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Advanced;
    }
    else if (SkillScore < 75.0f)
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Expert;
    }
    else if (SkillScore < 90.0f)
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Master;
    }
    else
    {
        PlayerPerformance.EstimatedSkillLevel = ESkillLevel::Legend;
    }

    if (PlayerPerformance.EstimatedSkillLevel != OldLevel)
    {
        OnSkillLevelChanged.Broadcast(PlayerPerformance.EstimatedSkillLevel);
    }
}

void UMGDynamicDifficultySubsystem::UpdatePerformanceMetric(EPerformanceMetric Metric, float Value)
{
    switch (Metric)
    {
        case EPerformanceMetric::DriftScore:
            PlayerPerformance.TotalDriftScore += Value;
            break;
        case EPerformanceMetric::OvertakeCount:
            PlayerPerformance.TotalOvertakes += static_cast<int32>(Value);
            break;
        case EPerformanceMetric::CollisionCount:
            PlayerPerformance.TotalCollisions += static_cast<int32>(Value);
            break;
        case EPerformanceMetric::AverageSpeed:
            PlayerPerformance.AverageTopSpeed = (PlayerPerformance.AverageTopSpeed + Value) * 0.5f;
            break;
        case EPerformanceMetric::ShortcutUsage:
            PlayerPerformance.ShortcutsUsed += static_cast<int32>(Value);
            break;
        default:
            break;
    }
}

float UMGDynamicDifficultySubsystem::GetPerformanceTrend() const
{
    if (AdaptiveHistory.RecentRaces.Num() < 5)
    {
        return 0.0f;
    }

    // Compare recent performance to earlier performance
    int32 HalfPoint = AdaptiveHistory.RecentRaces.Num() / 2;

    float EarlyAvgPosition = 0.0f;
    for (int32 i = 0; i < HalfPoint; i++)
    {
        EarlyAvgPosition += AdaptiveHistory.RecentRaces[i].FinalPosition;
    }
    EarlyAvgPosition /= HalfPoint;

    float LateAvgPosition = 0.0f;
    for (int32 i = HalfPoint; i < AdaptiveHistory.RecentRaces.Num(); i++)
    {
        LateAvgPosition += AdaptiveHistory.RecentRaces[i].FinalPosition;
    }
    LateAvgPosition /= (AdaptiveHistory.RecentRaces.Num() - HalfPoint);

    // Positive trend = improving (lower positions)
    return EarlyAvgPosition - LateAvgPosition;
}

void UMGDynamicDifficultySubsystem::ResetPerformanceData()
{
    PlayerPerformance = FPlayerPerformanceData();
    PlayerPerformance.PlayerId = TEXT("LocalPlayer");
    PlayerPerformance.LastUpdated = FDateTime::Now();

    AdaptiveHistory = FAdaptiveHistory();
    DifficultyStats = FDynamicDifficultyStats();
    DifficultyStats.CurrentDifficultyLevel = 1.0f;
}

// ============================================================================
// Adaptive Difficulty
// ============================================================================

void UMGDynamicDifficultySubsystem::EnableAdaptiveDifficulty(bool bEnable)
{
    bAdaptiveDifficultyEnabled = bEnable;

    if (bEnable)
    {
        CurrentPreset = EDifficultyPreset::Adaptive;
    }
}

void UMGDynamicDifficultySubsystem::SetAdaptationSpeed(EAdaptationSpeed Speed)
{
    CurrentAdaptationSpeed = Speed;
}

void UMGDynamicDifficultySubsystem::SetAdaptationSensitivity(float Sensitivity)
{
    AdaptationSensitivity = FMath::Clamp(Sensitivity, 0.0f, 1.0f);
}

void UMGDynamicDifficultySubsystem::TriggerAdaptiveAdjustment()
{
    if (!bAdaptiveDifficultyEnabled)
    {
        return;
    }

    float Adjustment = CalculateAdaptiveAdjustment();
    ApplyAdaptiveAdjustment(Adjustment);
}

float UMGDynamicDifficultySubsystem::CalculateAdaptiveAdjustment()
{
    float Adjustment = 0.0f;

    // Base adjustment on recent win rate
    float TargetWinRate = 0.35f; // Target ~35% win rate for challenge
    float WinRateDiff = DifficultyStats.RecentWinRate - TargetWinRate;

    // If winning too much, increase difficulty; if losing too much, decrease
    Adjustment = WinRateDiff * AdaptationSensitivity;

    // Factor in consecutive wins/losses
    if (AdaptiveHistory.ConsecutiveWins >= 3)
    {
        Adjustment += 0.05f * AdaptiveHistory.ConsecutiveWins;
    }
    else if (AdaptiveHistory.ConsecutiveLosses >= 3)
    {
        Adjustment -= 0.05f * AdaptiveHistory.ConsecutiveLosses;
    }

    // Factor in frustration
    if (CurrentFrustrationLevel >= EFrustrationLevel::Frustrated)
    {
        Adjustment -= 0.1f;
    }

    // Apply adaptation speed modifier
    float SpeedModifier = 1.0f;
    switch (CurrentAdaptationSpeed)
    {
        case EAdaptationSpeed::Instant: SpeedModifier = 2.0f; break;
        case EAdaptationSpeed::Fast: SpeedModifier = 1.5f; break;
        case EAdaptationSpeed::Medium: SpeedModifier = 1.0f; break;
        case EAdaptationSpeed::Slow: SpeedModifier = 0.5f; break;
        case EAdaptationSpeed::Gradual: SpeedModifier = 0.25f; break;
    }

    Adjustment *= SpeedModifier;

    // Clamp adjustment size
    Adjustment = FMath::Clamp(Adjustment, -0.15f, 0.15f);

    return Adjustment;
}

void UMGDynamicDifficultySubsystem::ApplyAdaptiveAdjustment(float Adjustment)
{
    if (FMath::IsNearlyZero(Adjustment, 0.01f))
    {
        return;
    }

    float OldLevel = DifficultyStats.CurrentDifficultyLevel;
    DifficultyStats.CurrentDifficultyLevel = FMath::Clamp(
        DifficultyStats.CurrentDifficultyLevel + Adjustment,
        MinDifficultyBound,
        MaxDifficultyBound
    );

    // Apply adjustment to modifiers
    float AdjustmentFactor = DifficultyStats.CurrentDifficultyLevel / OldLevel;

    float OldAISpeed = CurrentModifiers.AISpeedMultiplier;
    CurrentModifiers.AISpeedMultiplier = FMath::Clamp(
        CurrentModifiers.AISpeedMultiplier * FMath::Pow(AdjustmentFactor, 0.5f),
        0.6f, 1.5f
    );

    // Record adjustment
    FDifficultyAdjustment AdjustmentRecord = CreateAdjustment(
        EDifficultyAspect::AISpeed,
        OldAISpeed,
        CurrentModifiers.AISpeedMultiplier,
        FString::Printf(TEXT("Adaptive adjustment (%.2f)"), Adjustment)
    );

    AdaptiveHistory.RecentAdjustments.Add(AdjustmentRecord);
    if (AdaptiveHistory.RecentAdjustments.Num() > 100)
    {
        AdaptiveHistory.RecentAdjustments.RemoveAt(0);
    }

    AdaptiveHistory.AdjustmentCount++;
    AdaptiveHistory.LastAdjustment = FDateTime::Now();

    // Update stats
    DifficultyStats.TotalAdjustments++;
    if (Adjustment > 0)
    {
        DifficultyStats.DifficultyIncreases++;
    }
    else
    {
        DifficultyStats.DifficultyDecreases++;
    }

    OnDifficultyAdjusted.Broadcast(AdjustmentRecord);
}

void UMGDynamicDifficultySubsystem::SetAdaptiveBounds(float MinDifficulty, float MaxDifficulty)
{
    MinDifficultyBound = FMath::Max(0.1f, MinDifficulty);
    MaxDifficultyBound = FMath::Min(3.0f, MaxDifficulty);
}

// ============================================================================
// AI Opponent Configuration
// ============================================================================

FAIOpponentSettings UMGDynamicDifficultySubsystem::GenerateOpponentSettings(ESkillLevel TargetSkill, EAIBehaviorProfile Profile)
{
    FAIOpponentSettings Settings;
    Settings.OpponentId = FGuid::NewGuid().ToString();
    Settings.BehaviorProfile = Profile;

    // Base values by skill level
    float SkillMultiplier = 0.5f + (static_cast<float>(TargetSkill) * 0.1f);

    Settings.BaseSpeed = SkillMultiplier * CurrentModifiers.AISpeedMultiplier;
    Settings.DriftSkill = FMath::Clamp(SkillMultiplier, 0.3f, 1.0f);
    Settings.OvertakeSkill = FMath::Clamp(SkillMultiplier * 0.9f, 0.2f, 1.0f);
    Settings.DefenseSkill = FMath::Clamp(SkillMultiplier * 0.8f, 0.2f, 1.0f);
    Settings.NitroUsageEfficiency = FMath::Clamp(SkillMultiplier, 0.3f, 1.0f);
    Settings.ShortcutKnowledge = FMath::Clamp(SkillMultiplier * 0.7f, 0.0f, 1.0f);
    Settings.ErrorRate = FMath::Clamp(0.3f - (SkillMultiplier * 0.25f), 0.0f, 0.3f);
    Settings.ReactionTime = FMath::Clamp(0.4f - (SkillMultiplier * 0.2f), 0.1f, 0.5f);

    // Apply behavior profile modifiers
    switch (Profile)
    {
        case EAIBehaviorProfile::Defensive:
            Settings.Aggression = 0.2f;
            Settings.DefenseSkill *= 1.3f;
            Settings.OvertakeSkill *= 0.7f;
            break;

        case EAIBehaviorProfile::Aggressive:
            Settings.Aggression = 0.9f;
            Settings.OvertakeSkill *= 1.2f;
            Settings.DefenseSkill *= 0.8f;
            Settings.ErrorRate *= 1.2f;
            break;

        case EAIBehaviorProfile::Tactical:
            Settings.Aggression = 0.5f;
            Settings.NitroUsageEfficiency *= 1.2f;
            Settings.ShortcutKnowledge *= 1.3f;
            break;

        case EAIBehaviorProfile::Unpredictable:
            Settings.Aggression = FMath::RandRange(0.2f, 0.9f);
            Settings.ErrorRate *= FMath::RandRange(0.8f, 1.3f);
            break;

        case EAIBehaviorProfile::Rubberband:
            Settings.bUsesRubberBanding = true;
            Settings.RubberbandingFactor = 0.5f;
            break;

        case EAIBehaviorProfile::TrainingWheel:
            Settings.Aggression = 0.1f;
            Settings.BaseSpeed *= 0.85f;
            Settings.ErrorRate = 0.25f;
            Settings.bUsesRubberBanding = true;
            Settings.RubberbandingFactor = -0.3f; // Slows down when ahead
            break;

        default: // Balanced
            Settings.Aggression = 0.5f;
            break;
    }

    // Generate name
    TArray<FString> FirstNames = { TEXT("Shadow"), TEXT("Blaze"), TEXT("Nitro"), TEXT("Viper"), TEXT("Turbo"), TEXT("Ghost"), TEXT("Storm"), TEXT("Ace") };
    TArray<FString> LastNames = { TEXT("Rider"), TEXT("Racer"), TEXT("Driver"), TEXT("Speed"), TEXT("Flash"), TEXT("Burn"), TEXT("Drift"), TEXT("Rush") };
    Settings.OpponentName = FString::Printf(TEXT("%s %s"),
        *FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)],
        *LastNames[FMath::RandRange(0, LastNames.Num() - 1)]);

    Settings.bAdaptive = true;

    return Settings;
}

TArray<FAIOpponentSettings> UMGDynamicDifficultySubsystem::GenerateOpponentGrid(int32 OpponentCount)
{
    TArray<FAIOpponentSettings> Grid;

    // Distribute opponents across skill levels relative to player
    ESkillLevel PlayerSkill = PlayerPerformance.EstimatedSkillLevel;
    int32 PlayerSkillInt = static_cast<int32>(PlayerSkill);

    for (int32 i = 0; i < OpponentCount; i++)
    {
        // Vary skill around player level
        int32 SkillVariation = (i % 5) - 2; // -2 to +2
        int32 OpponentSkillInt = FMath::Clamp(PlayerSkillInt + SkillVariation, 0, 6);
        ESkillLevel OpponentSkill = static_cast<ESkillLevel>(OpponentSkillInt);

        // Vary behavior profiles
        EAIBehaviorProfile Profile;
        switch (i % 6)
        {
            case 0: Profile = EAIBehaviorProfile::Balanced; break;
            case 1: Profile = EAIBehaviorProfile::Aggressive; break;
            case 2: Profile = EAIBehaviorProfile::Defensive; break;
            case 3: Profile = EAIBehaviorProfile::Tactical; break;
            case 4: Profile = EAIBehaviorProfile::Unpredictable; break;
            default: Profile = EAIBehaviorProfile::Balanced; break;
        }

        FAIOpponentSettings Settings = GenerateOpponentSettings(OpponentSkill, Profile);
        Grid.Add(Settings);
        ActiveOpponents.Add(Settings.OpponentId, Settings);
    }

    return Grid;
}

void UMGDynamicDifficultySubsystem::AdjustOpponentDifficulty(const FString& OpponentId, float DifficultyDelta)
{
    if (ActiveOpponents.Contains(OpponentId))
    {
        FAIOpponentSettings& Settings = ActiveOpponents[OpponentId];
        Settings.BaseSpeed = FMath::Clamp(Settings.BaseSpeed + (DifficultyDelta * 0.1f), 0.5f, 1.5f);
        Settings.Aggression = FMath::Clamp(Settings.Aggression + (DifficultyDelta * 0.1f), 0.0f, 1.0f);

        OnAISettingsChanged.Broadcast(OpponentId, Settings);
    }
}

void UMGDynamicDifficultySubsystem::SetOpponentBehaviorProfile(const FString& OpponentId, EAIBehaviorProfile Profile)
{
    if (ActiveOpponents.Contains(OpponentId))
    {
        FAIOpponentSettings& Settings = ActiveOpponents[OpponentId];
        Settings.BehaviorProfile = Profile;

        OnAISettingsChanged.Broadcast(OpponentId, Settings);
    }
}

FAIOpponentSettings UMGDynamicDifficultySubsystem::GetOpponentSettings(const FString& OpponentId) const
{
    if (ActiveOpponents.Contains(OpponentId))
    {
        return ActiveOpponents[OpponentId];
    }
    return FAIOpponentSettings();
}

void UMGDynamicDifficultySubsystem::ApplyRubberbanding(const FString& OpponentId, float PlayerDistance)
{
    if (!ActiveOpponents.Contains(OpponentId))
    {
        return;
    }

    FAIOpponentSettings& Settings = ActiveOpponents[OpponentId];

    if (!Settings.bUsesRubberBanding)
    {
        return;
    }

    // Calculate speed adjustment based on distance
    float SpeedAdjustment = 0.0f;

    if (PlayerDistance > 0) // AI is behind player
    {
        // Speed up to catch up
        SpeedAdjustment = FMath::Clamp(PlayerDistance / 100.0f, 0.0f, 1.0f) * Settings.RubberbandingFactor;
    }
    else // AI is ahead of player
    {
        // Slow down slightly
        SpeedAdjustment = FMath::Clamp(FMath::Abs(PlayerDistance) / 100.0f, 0.0f, 0.5f) * -Settings.RubberbandingFactor;
    }

    Settings.BaseSpeed = FMath::Clamp(Settings.BaseSpeed + SpeedAdjustment, 0.6f, 1.4f);
}

void UMGDynamicDifficultySubsystem::SetGlobalRubberbanding(bool bEnable, float Strength)
{
    bGlobalRubberbandingEnabled = bEnable;
    GlobalRubberbandingStrength = FMath::Clamp(Strength, 0.0f, 1.0f);

    // Apply to all active opponents
    for (auto& Pair : ActiveOpponents)
    {
        Pair.Value.bUsesRubberBanding = bEnable;
        Pair.Value.RubberbandingFactor = Strength;
    }
}

// ============================================================================
// Assist Configuration
// ============================================================================

void UMGDynamicDifficultySubsystem::SetAssists(const FAssistSettings& NewAssists)
{
    CurrentAssists = NewAssists;
}

void UMGDynamicDifficultySubsystem::ToggleAssist(EAssistType AssistType, bool bEnable)
{
    switch (AssistType)
    {
        case EAssistType::Steering:
            CurrentAssists.bSteeringAssist = bEnable;
            break;
        case EAssistType::Braking:
            CurrentAssists.bBrakingAssist = bEnable;
            break;
        case EAssistType::Drifting:
            CurrentAssists.bDriftAssist = bEnable;
            break;
        case EAssistType::NitroTiming:
            CurrentAssists.bNitroTimingAssist = bEnable;
            break;
        case EAssistType::RacingLine:
            CurrentAssists.bRacingLineDisplay = bEnable;
            break;
        case EAssistType::CollisionAvoidance:
            CurrentAssists.bCollisionPrediction = bEnable;
            break;
        case EAssistType::ShortcutHints:
            CurrentAssists.bShortcutHints = bEnable;
            break;
        case EAssistType::OpponentTracking:
            CurrentAssists.bOpponentTrackingUI = bEnable;
            break;
    }

    OnAssistToggled.Broadcast(AssistType, bEnable);
}

bool UMGDynamicDifficultySubsystem::IsAssistEnabled(EAssistType AssistType) const
{
    switch (AssistType)
    {
        case EAssistType::Steering: return CurrentAssists.bSteeringAssist;
        case EAssistType::Braking: return CurrentAssists.bBrakingAssist;
        case EAssistType::Drifting: return CurrentAssists.bDriftAssist;
        case EAssistType::NitroTiming: return CurrentAssists.bNitroTimingAssist;
        case EAssistType::RacingLine: return CurrentAssists.bRacingLineDisplay;
        case EAssistType::CollisionAvoidance: return CurrentAssists.bCollisionPrediction;
        case EAssistType::ShortcutHints: return CurrentAssists.bShortcutHints;
        case EAssistType::OpponentTracking: return CurrentAssists.bOpponentTrackingUI;
        default: return false;
    }
}

void UMGDynamicDifficultySubsystem::SetAssistStrength(EAssistType AssistType, float Strength)
{
    Strength = FMath::Clamp(Strength, 0.0f, 1.0f);

    switch (AssistType)
    {
        case EAssistType::Steering:
            CurrentAssists.SteeringAssistStrength = Strength;
            break;
        case EAssistType::Braking:
            CurrentAssists.BrakingAssistStrength = Strength;
            break;
        case EAssistType::Drifting:
            CurrentAssists.DriftAssistStrength = Strength;
            break;
        default:
            break;
    }
}

void UMGDynamicDifficultySubsystem::SuggestAssistsForSkillLevel(ESkillLevel SkillLevel)
{
    // Reset to recommended settings based on skill
    switch (SkillLevel)
    {
        case ESkillLevel::Beginner:
        case ESkillLevel::Novice:
            CurrentAssists.bSteeringAssist = true;
            CurrentAssists.SteeringAssistStrength = 0.8f;
            CurrentAssists.bBrakingAssist = true;
            CurrentAssists.BrakingAssistStrength = 0.8f;
            CurrentAssists.bDriftAssist = true;
            CurrentAssists.bRacingLineDisplay = true;
            CurrentAssists.bAutoRecovery = true;
            CurrentAssists.bRewindFeature = true;
            break;

        case ESkillLevel::Intermediate:
            CurrentAssists.bSteeringAssist = false;
            CurrentAssists.bBrakingAssist = true;
            CurrentAssists.BrakingAssistStrength = 0.5f;
            CurrentAssists.bDriftAssist = true;
            CurrentAssists.DriftAssistStrength = 0.5f;
            CurrentAssists.bRacingLineDisplay = true;
            CurrentAssists.bAutoRecovery = true;
            CurrentAssists.bRewindFeature = false;
            break;

        case ESkillLevel::Advanced:
            CurrentAssists.bSteeringAssist = false;
            CurrentAssists.bBrakingAssist = false;
            CurrentAssists.bDriftAssist = false;
            CurrentAssists.bRacingLineDisplay = false;
            CurrentAssists.bAutoRecovery = true;
            break;

        default: // Expert and above
            CurrentAssists = FAssistSettings();
            CurrentAssists.bOpponentTrackingUI = true;
            break;
    }
}

void UMGDynamicDifficultySubsystem::ResetAssistsToDefault()
{
    if (PresetProfiles.Contains(CurrentPreset))
    {
        CurrentAssists = PresetProfiles[CurrentPreset].Assists;
    }
}

// ============================================================================
// Frustration Detection
// ============================================================================

EFrustrationLevel UMGDynamicDifficultySubsystem::DetectFrustrationLevel(const FRaceAnalysis& Analysis)
{
    float FrustrationScore = 0.0f;

    // Factor: Finished in last place
    if (Analysis.bFinished && Analysis.FinalPosition >= 7)
    {
        FrustrationScore += 20.0f;
    }

    // Factor: DNF
    if (!Analysis.bFinished)
    {
        FrustrationScore += 30.0f;
    }

    // Factor: Many collisions
    if (Analysis.TotalCollisions > 10)
    {
        FrustrationScore += 15.0f;
    }

    // Factor: Time in last place
    float TotalRaceTime = FMath::Max(Analysis.TotalRaceTime, 1.0f);
    if (Analysis.TimeInLast / TotalRaceTime > 0.5f)
    {
        FrustrationScore += 20.0f;
    }

    // Factor: Consecutive losses
    if (AdaptiveHistory.ConsecutiveLosses >= 3)
    {
        FrustrationScore += 10.0f * AdaptiveHistory.ConsecutiveLosses;
    }

    // Factor: Large gap to leader
    if (Analysis.GapToLeader > 30.0f)
    {
        FrustrationScore += 15.0f;
    }

    // Determine level
    if (FrustrationScore < 20.0f)
    {
        CurrentFrustrationLevel = EFrustrationLevel::Relaxed;
    }
    else if (FrustrationScore < 40.0f)
    {
        CurrentFrustrationLevel = EFrustrationLevel::Comfortable;
    }
    else if (FrustrationScore < 60.0f)
    {
        CurrentFrustrationLevel = EFrustrationLevel::Challenged;
    }
    else if (FrustrationScore < 80.0f)
    {
        CurrentFrustrationLevel = EFrustrationLevel::Frustrated;
    }
    else
    {
        CurrentFrustrationLevel = EFrustrationLevel::Overwhelmed;
    }

    if (CurrentFrustrationLevel >= EFrustrationLevel::Frustrated)
    {
        OnFrustrationDetected.Broadcast(CurrentFrustrationLevel);
    }

    return CurrentFrustrationLevel;
}

void UMGDynamicDifficultySubsystem::RespondToFrustration(EFrustrationLevel Level)
{
    if (!bFrustrationResponseEnabled)
    {
        return;
    }

    switch (Level)
    {
        case EFrustrationLevel::Frustrated:
            // Moderate adjustments
            CurrentModifiers.AISpeedMultiplier *= 0.95f;
            CurrentModifiers.CatchUpAssistStrength = FMath::Min(CurrentModifiers.CatchUpAssistStrength + 0.1f, 0.5f);
            break;

        case EFrustrationLevel::Overwhelmed:
            // Significant adjustments
            CurrentModifiers.AISpeedMultiplier *= 0.9f;
            CurrentModifiers.AIAggressionLevel *= 0.8f;
            CurrentModifiers.CatchUpAssistStrength = FMath::Min(CurrentModifiers.CatchUpAssistStrength + 0.2f, 0.7f);
            CurrentAssists.bAutoRecovery = true;
            break;

        default:
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Responding to frustration level %d"), static_cast<int32>(Level));
}

void UMGDynamicDifficultySubsystem::SetFrustrationResponseEnabled(bool bEnable)
{
    bFrustrationResponseEnabled = bEnable;
}

// ============================================================================
// Race-Time Adjustments
// ============================================================================

void UMGDynamicDifficultySubsystem::UpdateRaceProgress(int32 PlayerPosition, float GapToLeader, float RaceProgress)
{
    CurrentRacePosition = PlayerPosition;
    CurrentGapToLeader = GapToLeader;
    CurrentRaceProgress = RaceProgress;
}

void UMGDynamicDifficultySubsystem::OnPlayerCollision(float ImpactSeverity)
{
    RaceCollisionCount++;

    // Track for analysis
    if (RaceCollisionCount > 5 && CurrentModifiers.CatchUpAssistStrength > 0)
    {
        // Temporarily boost catch-up for players having trouble
    }
}

void UMGDynamicDifficultySubsystem::OnPlayerOvertake(bool bMadeOvertake)
{
    // Track overtakes for skill assessment
}

float UMGDynamicDifficultySubsystem::GetDynamicCatchUpBoost() const
{
    if (CurrentModifiers.CatchUpAssistStrength <= 0.0f)
    {
        return 0.0f;
    }

    // Calculate boost based on position and gap
    float PositionFactor = FMath::Clamp((CurrentRacePosition - 1.0f) / 7.0f, 0.0f, 1.0f);
    float GapFactor = FMath::Clamp(CurrentGapToLeader / 50.0f, 0.0f, 1.0f);

    return CurrentModifiers.CatchUpAssistStrength * PositionFactor * GapFactor;
}

float UMGDynamicDifficultySubsystem::GetDynamicAISlowdown() const
{
    if (!bGlobalRubberbandingEnabled)
    {
        return 0.0f;
    }

    // Leading AI slows down slightly if player is far behind
    if (CurrentRacePosition > 4 && CurrentGapToLeader > 20.0f)
    {
        return GlobalRubberbandingStrength * 0.1f;
    }

    return 0.0f;
}

void UMGDynamicDifficultySubsystem::ResetRaceTimeAdjustments()
{
    CurrentRacePosition = 1;
    CurrentGapToLeader = 0.0f;
    CurrentRaceProgress = 0.0f;
    RaceCollisionCount = 0;
}

// ============================================================================
// Statistics
// ============================================================================

float UMGDynamicDifficultySubsystem::GetOverallDifficultyLevel() const
{
    // Combine modifiers into single difficulty value
    float Level = CurrentModifiers.AISpeedMultiplier;
    Level += CurrentModifiers.AIAggressionLevel * 0.5f;
    Level -= CurrentModifiers.AIErrorRate * 2.0f;
    Level += CurrentModifiers.TrafficDensityMultiplier * 0.2f;
    Level += CurrentModifiers.ObstacleDensityMultiplier * 0.2f;
    Level -= CurrentModifiers.CatchUpAssistStrength * 0.3f;
    Level += CurrentModifiers.TimePressureMultiplier * 0.2f;

    return FMath::Clamp(Level, 0.0f, 3.0f);
}

void UMGDynamicDifficultySubsystem::ExportPerformanceReport(FString& OutReport)
{
    OutReport = FString::Printf(TEXT(
        "=== Player Performance Report ===\n"
        "Skill Level: %d\n"
        "Total Races: %d\n"
        "Win Rate: %.1f%%\n"
        "Podium Rate: %.1f%%\n"
        "Average Position: %.1f\n"
        "Total Drift Score: %.0f\n"
        "Total Overtakes: %d\n"
        "\n"
        "=== Difficulty Settings ===\n"
        "Current Preset: %d\n"
        "Overall Level: %.2f\n"
        "AI Speed: %.2f\n"
        "AI Aggression: %.2f\n"
        "Catch-Up Assist: %.2f\n"
        "\n"
        "=== Adaptive System ===\n"
        "Total Adjustments: %d\n"
        "Increases: %d\n"
        "Decreases: %d\n"
        "Consecutive Wins: %d\n"
        "Consecutive Losses: %d\n"
    ),
        static_cast<int32>(PlayerPerformance.EstimatedSkillLevel),
        PlayerPerformance.TotalRaces,
        PlayerPerformance.WinRate * 100.0f,
        PlayerPerformance.PodiumRate * 100.0f,
        PlayerPerformance.AveragePosition,
        PlayerPerformance.TotalDriftScore,
        PlayerPerformance.TotalOvertakes,
        static_cast<int32>(CurrentPreset),
        GetOverallDifficultyLevel(),
        CurrentModifiers.AISpeedMultiplier,
        CurrentModifiers.AIAggressionLevel,
        CurrentModifiers.CatchUpAssistStrength,
        DifficultyStats.TotalAdjustments,
        DifficultyStats.DifficultyIncreases,
        DifficultyStats.DifficultyDecreases,
        AdaptiveHistory.ConsecutiveWins,
        AdaptiveHistory.ConsecutiveLosses
    );
}

// ============================================================================
// Internal Helpers
// ============================================================================

FDifficultyAdjustment UMGDynamicDifficultySubsystem::CreateAdjustment(EDifficultyAspect Aspect, float OldValue, float NewValue, const FString& Reason)
{
    FDifficultyAdjustment Adjustment;
    Adjustment.Aspect = Aspect;
    Adjustment.PreviousValue = OldValue;
    Adjustment.NewValue = NewValue;
    Adjustment.ChangeAmount = NewValue - OldValue;
    Adjustment.Reason = Reason;
    Adjustment.AdjustedAt = FDateTime::Now();

    return Adjustment;
}

// MidnightGrind - Arcade Street Racing Game
// Milestone Subsystem Implementation

#include "Milestone/MGMilestoneSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UMGMilestoneSubsystem::UMGMilestoneSubsystem()
    : bDataDirty(false)
    , AutoSaveInterval(60.0f)
    , TimeSinceLastSave(0.0f)
{
    // Progress thresholds for notifications (25%, 50%, 75%, 90%)
    ProgressNotificationThresholds = { 0.25f, 0.50f, 0.75f, 0.90f };
}

void UMGMilestoneSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    SessionStartTime = FDateTime::Now();

    // Initialize default milestones
    InitializeDefaultMilestones();

    // Load saved data
    LoadMilestoneData();

    // Update first/last play dates
    if (PlayerStats.FirstPlayDate == FDateTime())
    {
        PlayerStats.FirstPlayDate = FDateTime::Now();
    }
    PlayerStats.LastPlayDate = FDateTime::Now();

    // Start tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            TickTimerHandle,
            [this]() { TickMilestoneSystem(0.033f); },
            0.033f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("MGMilestoneSubsystem initialized with %d milestones"), MilestoneDefinitions.Num());
}

void UMGMilestoneSubsystem::Deinitialize()
{
    // Calculate session playtime
    FTimespan SessionDuration = FDateTime::Now() - SessionStartTime;
    PlayerStats.TotalPlaytimeHours += SessionDuration.GetTotalHours();

    // Save data on shutdown
    if (bDataDirty)
    {
        SaveMilestoneData();
    }

    // Clear tick timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(TickTimerHandle);
    }

    Super::Deinitialize();
}

void UMGMilestoneSubsystem::TickMilestoneSystem(float DeltaTime)
{
    // Process timed milestones
    ProcessTimedMilestones(DeltaTime);

    // Check season expiration
    ProcessSeasonExpiration();

    // Auto-save
    TimeSinceLastSave += DeltaTime;
    if (bDataDirty && TimeSinceLastSave >= AutoSaveInterval)
    {
        SaveMilestoneData();
        TimeSinceLastSave = 0.0f;
    }

    // Update playtime stat
    IncrementStat(EMGStatType::PlaytimeHours, DeltaTime / 3600.0f);
}

// ===== Milestone Definition Management =====

void UMGMilestoneSubsystem::RegisterMilestone(const FMGMilestoneDefinition& Definition)
{
    if (Definition.MilestoneId == NAME_None)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot register milestone with invalid ID"));
        return;
    }

    MilestoneDefinitions.Add(Definition.MilestoneId, Definition);

    // Initialize progress entry if not exists
    if (!MilestoneProgress.Contains(Definition.MilestoneId))
    {
        FMGMilestoneProgress Progress;
        Progress.MilestoneId = Definition.MilestoneId;
        Progress.Status = Definition.bIsSecret ? EMGMilestoneStatus::Hidden : EMGMilestoneStatus::Locked;

        // Copy requirements for tracking
        Progress.RequirementProgress = Definition.Requirements;

        MilestoneProgress.Add(Definition.MilestoneId, Progress);
    }

    bDataDirty = true;
}

void UMGMilestoneSubsystem::UnregisterMilestone(FName MilestoneId)
{
    MilestoneDefinitions.Remove(MilestoneId);
    bDataDirty = true;
}

bool UMGMilestoneSubsystem::GetMilestoneDefinition(FName MilestoneId, FMGMilestoneDefinition& OutDefinition) const
{
    const FMGMilestoneDefinition* Found = MilestoneDefinitions.Find(MilestoneId);
    if (Found)
    {
        OutDefinition = *Found;
        return true;
    }
    return false;
}

TArray<FMGMilestoneDefinition> UMGMilestoneSubsystem::GetAllMilestones() const
{
    TArray<FMGMilestoneDefinition> Result;
    MilestoneDefinitions.GenerateValueArray(Result);
    return Result;
}

TArray<FMGMilestoneDefinition> UMGMilestoneSubsystem::GetMilestonesByCategory(EMGMilestoneCategory Category) const
{
    TArray<FMGMilestoneDefinition> Result;
    for (const auto& Pair : MilestoneDefinitions)
    {
        if (Pair.Value.Category == Category)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FMGMilestoneDefinition> UMGMilestoneSubsystem::GetMilestonesByRarity(EMGMilestoneRarity Rarity) const
{
    TArray<FMGMilestoneDefinition> Result;
    for (const auto& Pair : MilestoneDefinitions)
    {
        if (Pair.Value.Rarity == Rarity)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

// ===== Progress Tracking =====

void UMGMilestoneSubsystem::UpdateMilestoneProgress(FName MilestoneId, int32 RequirementIndex, float NewValue)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress)
    {
        return;
    }

    // Cannot update completed milestones (unless repeatable)
    FMGMilestoneDefinition Definition;
    if (GetMilestoneDefinition(MilestoneId, Definition))
    {
        if (Progress->Status == EMGMilestoneStatus::Claimed && !Definition.bIsRepeatable)
        {
            return;
        }
    }

    if (!Progress->RequirementProgress.IsValidIndex(RequirementIndex))
    {
        return;
    }

    FMGMilestoneRequirement& Req = Progress->RequirementProgress[RequirementIndex];
    float OldValue = Req.CurrentValue;

    // Apply value based on tracking type
    switch (Req.TrackingType)
    {
        case EMGMilestoneTrackingType::Counter:
        case EMGMilestoneTrackingType::Cumulative:
            Req.CurrentValue = NewValue;
            break;
        case EMGMilestoneTrackingType::Maximum:
            Req.CurrentValue = FMath::Max(Req.CurrentValue, NewValue);
            break;
        case EMGMilestoneTrackingType::Minimum:
            if (Req.CurrentValue == 0.0f || NewValue < Req.CurrentValue)
            {
                Req.CurrentValue = NewValue;
            }
            break;
        case EMGMilestoneTrackingType::Boolean:
            Req.CurrentValue = NewValue > 0.0f ? 1.0f : 0.0f;
            break;
        default:
            Req.CurrentValue = NewValue;
            break;
    }

    // Track first started time
    if (Progress->Status == EMGMilestoneStatus::Locked || Progress->Status == EMGMilestoneStatus::Revealed)
    {
        Progress->FirstStartedTime = FDateTime::Now();
        Progress->Status = EMGMilestoneStatus::InProgress;
    }

    // Fire progress event
    float NewProgress = Progress->GetOverallProgress();
    OnMilestoneProgressUpdated.Broadcast(MilestoneId, NewProgress);

    // Check for progress notification thresholds
    float OldProgress = (OldValue / Req.TargetValue);
    for (float Threshold : ProgressNotificationThresholds)
    {
        if (OldProgress < Threshold && NewProgress >= Threshold)
        {
            CreateProgressNotification(MilestoneId, NewProgress);
            break;
        }
    }

    // Check completion
    CheckMilestoneCompletion(MilestoneId);

    bDataDirty = true;
}

void UMGMilestoneSubsystem::IncrementMilestoneProgress(FName MilestoneId, int32 RequirementIndex, float Amount)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress || !Progress->RequirementProgress.IsValidIndex(RequirementIndex))
    {
        return;
    }

    float NewValue = Progress->RequirementProgress[RequirementIndex].CurrentValue + Amount;
    UpdateMilestoneProgress(MilestoneId, RequirementIndex, NewValue);
}

FMGMilestoneProgress UMGMilestoneSubsystem::GetMilestoneProgress(FName MilestoneId) const
{
    const FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    return Progress ? *Progress : FMGMilestoneProgress();
}

float UMGMilestoneSubsystem::GetMilestoneProgressPercent(FName MilestoneId) const
{
    const FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    return Progress ? Progress->GetOverallProgress() : 0.0f;
}

EMGMilestoneStatus UMGMilestoneSubsystem::GetMilestoneStatus(FName MilestoneId) const
{
    const FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    return Progress ? Progress->Status : EMGMilestoneStatus::Locked;
}

bool UMGMilestoneSubsystem::IsMilestoneComplete(FName MilestoneId) const
{
    const FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return false;

    return Progress->Status == EMGMilestoneStatus::Completed ||
           Progress->Status == EMGMilestoneStatus::Claimed;
}

bool UMGMilestoneSubsystem::IsMilestoneUnlocked(FName MilestoneId) const
{
    const FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return false;

    return Progress->Status != EMGMilestoneStatus::Locked &&
           Progress->Status != EMGMilestoneStatus::Hidden;
}

TArray<FMGMilestoneProgress> UMGMilestoneSubsystem::GetInProgressMilestones() const
{
    TArray<FMGMilestoneProgress> Result;
    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::InProgress)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

TArray<FMGMilestoneProgress> UMGMilestoneSubsystem::GetCompletedMilestones() const
{
    TArray<FMGMilestoneProgress> Result;
    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed ||
            Pair.Value.Status == EMGMilestoneStatus::Claimed)
        {
            Result.Add(Pair.Value);
        }
    }
    return Result;
}

// ===== Stat Tracking =====

void UMGMilestoneSubsystem::UpdateStat(EMGStatType StatType, float Value, bool bIsMaximum)
{
    if (bIsMaximum)
    {
        // Track best value
        float* BestValue = PlayerStats.BestStats.Find(StatType);
        if (!BestValue || Value > *BestValue)
        {
            PlayerStats.BestStats.Add(StatType, Value);
        }
    }
    else
    {
        // Set cumulative value
        PlayerStats.CumulativeStats.Add(StatType, Value);
    }

    // Update session stat
    PlayerStats.SessionStats.Add(StatType, Value);

    OnStatUpdated.Broadcast(StatType, Value);
    bDataDirty = true;
}

void UMGMilestoneSubsystem::IncrementStat(EMGStatType StatType, float Amount)
{
    float* CumulativeValue = PlayerStats.CumulativeStats.Find(StatType);
    float NewValue = CumulativeValue ? *CumulativeValue + Amount : Amount;
    PlayerStats.CumulativeStats.Add(StatType, NewValue);

    float* SessionValue = PlayerStats.SessionStats.Find(StatType);
    float NewSessionValue = SessionValue ? *SessionValue + Amount : Amount;
    PlayerStats.SessionStats.Add(StatType, NewSessionValue);

    OnStatUpdated.Broadcast(StatType, NewValue);
    bDataDirty = true;
}

void UMGMilestoneSubsystem::ResetSessionStats()
{
    PlayerStats.SessionStats.Empty();
}

float UMGMilestoneSubsystem::GetStat(EMGStatType StatType) const
{
    const float* Value = PlayerStats.CumulativeStats.Find(StatType);
    return Value ? *Value : 0.0f;
}

float UMGMilestoneSubsystem::GetSessionStat(EMGStatType StatType) const
{
    const float* Value = PlayerStats.SessionStats.Find(StatType);
    return Value ? *Value : 0.0f;
}

float UMGMilestoneSubsystem::GetBestStat(EMGStatType StatType) const
{
    const float* Value = PlayerStats.BestStats.Find(StatType);
    return Value ? *Value : 0.0f;
}

FMGPlayerStats UMGMilestoneSubsystem::GetAllStats() const
{
    return PlayerStats;
}

void UMGMilestoneSubsystem::RecordStatForMilestones(EMGStatType StatType, float Value, FName ContextId)
{
    // Increment the cumulative stat
    IncrementStat(StatType, Value);

    // Check all milestones that track this stat
    CheckAllMilestonesForStat(StatType);
}

// ===== Rewards =====

bool UMGMilestoneSubsystem::ClaimMilestoneRewards(FName MilestoneId, TArray<FMGMilestoneReward>& OutRewards)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress || Progress->Status != EMGMilestoneStatus::Completed)
    {
        return false;
    }

    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition))
    {
        return false;
    }

    // Collect rewards
    OutRewards = Definition.Rewards;

    // Mark as claimed
    Progress->Status = EMGMilestoneStatus::Claimed;
    Progress->ClaimedTime = FDateTime::Now();

    // Update stats
    PlayerStats.TotalMilestonesCompleted++;
    PlayerStats.TotalMilestonePoints += Definition.GetTotalPoints();

    if (Definition.bIsSecret)
    {
        PlayerStats.SecretMilestonesFound++;
    }

    OnMilestoneRewardsClaimed.Broadcast(MilestoneId, OutRewards);
    bDataDirty = true;

    return true;
}

bool UMGMilestoneSubsystem::ClaimAllPendingRewards(TArray<FMGMilestoneReward>& OutRewards)
{
    OutRewards.Empty();
    bool bClaimedAny = false;

    for (auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed)
        {
            TArray<FMGMilestoneReward> MilestoneRewards;
            if (ClaimMilestoneRewards(Pair.Key, MilestoneRewards))
            {
                OutRewards.Append(MilestoneRewards);
                bClaimedAny = true;
            }
        }
    }

    return bClaimedAny;
}

TArray<FMGMilestoneReward> UMGMilestoneSubsystem::GetPendingRewards() const
{
    TArray<FMGMilestoneReward> Result;

    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed)
        {
            FMGMilestoneDefinition Definition;
            if (GetMilestoneDefinition(Pair.Key, Definition))
            {
                Result.Append(Definition.Rewards);
            }
        }
    }

    return Result;
}

bool UMGMilestoneSubsystem::HasUnclaimedRewards() const
{
    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed)
        {
            return true;
        }
    }
    return false;
}

int32 UMGMilestoneSubsystem::GetUnclaimedRewardCount() const
{
    int32 Count = 0;
    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed)
        {
            Count++;
        }
    }
    return Count;
}

// ===== Milestone Chains =====

void UMGMilestoneSubsystem::RegisterChain(const FMGMilestoneChain& Chain)
{
    if (Chain.ChainId == NAME_None || Chain.MilestoneSequence.Num() == 0)
    {
        return;
    }

    MilestoneChains.Add(Chain.ChainId, Chain);
    bDataDirty = true;
}

bool UMGMilestoneSubsystem::GetChain(FName ChainId, FMGMilestoneChain& OutChain) const
{
    const FMGMilestoneChain* Found = MilestoneChains.Find(ChainId);
    if (Found)
    {
        OutChain = *Found;
        return true;
    }
    return false;
}

TArray<FMGMilestoneChain> UMGMilestoneSubsystem::GetAllChains() const
{
    TArray<FMGMilestoneChain> Result;
    MilestoneChains.GenerateValueArray(Result);
    return Result;
}

float UMGMilestoneSubsystem::GetChainProgress(FName ChainId) const
{
    const FMGMilestoneChain* Chain = MilestoneChains.Find(ChainId);
    return Chain ? Chain->GetChainProgress() : 0.0f;
}

bool UMGMilestoneSubsystem::IsChainComplete(FName ChainId) const
{
    const FMGMilestoneChain* Chain = MilestoneChains.Find(ChainId);
    return Chain ? Chain->bIsComplete : false;
}

// ===== Seasonal Milestones =====

void UMGMilestoneSubsystem::SetActiveSeason(const FMGSeasonMilestones& Season)
{
    // End current season if active
    if (ActiveSeason.bSeasonActive)
    {
        OnSeasonEnded.Broadcast(ActiveSeason.SeasonId, ActiveSeason.SeasonRank);
    }

    ActiveSeason = Season;
    ActiveSeason.bSeasonActive = true;

    // Register seasonal milestones
    for (const FName& MilestoneId : Season.SeasonalMilestoneIds)
    {
        // Milestones should already be registered, just unlock them
        if (FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId))
        {
            if (Progress->Status == EMGMilestoneStatus::Locked)
            {
                UnlockMilestone(MilestoneId);
            }
        }
    }

    OnSeasonStarted.Broadcast(Season.SeasonId);
    bDataDirty = true;
}

FMGSeasonMilestones UMGMilestoneSubsystem::GetActiveSeason() const
{
    return ActiveSeason;
}

bool UMGMilestoneSubsystem::IsSeasonActive() const
{
    return ActiveSeason.bSeasonActive && !ActiveSeason.IsSeasonExpired();
}

FTimespan UMGMilestoneSubsystem::GetSeasonTimeRemaining() const
{
    return ActiveSeason.GetTimeRemaining();
}

TArray<FMGMilestoneDefinition> UMGMilestoneSubsystem::GetSeasonalMilestones() const
{
    TArray<FMGMilestoneDefinition> Result;

    for (const FName& MilestoneId : ActiveSeason.SeasonalMilestoneIds)
    {
        FMGMilestoneDefinition Definition;
        if (GetMilestoneDefinition(MilestoneId, Definition))
        {
            Result.Add(Definition);
        }
    }

    return Result;
}

// ===== Secret Milestones =====

void UMGMilestoneSubsystem::RevealSecretMilestone(FName MilestoneId)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress || Progress->Status != EMGMilestoneStatus::Hidden)
    {
        return;
    }

    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition) || !Definition.bIsSecret)
    {
        return;
    }

    Progress->Status = EMGMilestoneStatus::Revealed;
    DiscoveredSecrets.AddUnique(MilestoneId);

    OnSecretMilestoneDiscovered.Broadcast(MilestoneId);
    bDataDirty = true;
}

TArray<FMGMilestoneDefinition> UMGMilestoneSubsystem::GetDiscoveredSecretMilestones() const
{
    TArray<FMGMilestoneDefinition> Result;

    for (const FName& MilestoneId : DiscoveredSecrets)
    {
        FMGMilestoneDefinition Definition;
        if (GetMilestoneDefinition(MilestoneId, Definition))
        {
            Result.Add(Definition);
        }
    }

    return Result;
}

int32 UMGMilestoneSubsystem::GetTotalSecretMilestoneCount() const
{
    int32 Count = 0;
    for (const auto& Pair : MilestoneDefinitions)
    {
        if (Pair.Value.bIsSecret)
        {
            Count++;
        }
    }
    return Count;
}

int32 UMGMilestoneSubsystem::GetDiscoveredSecretCount() const
{
    return DiscoveredSecrets.Num();
}

// ===== Notifications =====

void UMGMilestoneSubsystem::QueueNotification(const FMGMilestoneNotification& Notification)
{
    NotificationQueue.Add(Notification);
    OnMilestoneNotification.Broadcast(Notification);
}

bool UMGMilestoneSubsystem::PopNextNotification(FMGMilestoneNotification& OutNotification)
{
    if (NotificationQueue.Num() == 0)
    {
        return false;
    }

    OutNotification = NotificationQueue[0];
    NotificationQueue.RemoveAt(0);
    return true;
}

int32 UMGMilestoneSubsystem::GetPendingNotificationCount() const
{
    return NotificationQueue.Num();
}

void UMGMilestoneSubsystem::ClearAllNotifications()
{
    NotificationQueue.Empty();
}

// ===== Aggregate Data =====

int32 UMGMilestoneSubsystem::GetTotalMilestonePoints() const
{
    return PlayerStats.TotalMilestonePoints;
}

int32 UMGMilestoneSubsystem::GetTotalCompletedMilestones() const
{
    return PlayerStats.TotalMilestonesCompleted;
}

float UMGMilestoneSubsystem::GetOverallCompletionPercent() const
{
    int32 TotalMilestones = MilestoneDefinitions.Num();
    if (TotalMilestones == 0) return 0.0f;

    int32 Completed = 0;
    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed ||
            Pair.Value.Status == EMGMilestoneStatus::Claimed)
        {
            Completed++;
        }
    }

    return static_cast<float>(Completed) / TotalMilestones;
}

int32 UMGMilestoneSubsystem::GetCompletedCountByCategory(EMGMilestoneCategory Category) const
{
    int32 Count = 0;
    for (const auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed ||
            Pair.Value.Status == EMGMilestoneStatus::Claimed)
        {
            FMGMilestoneDefinition Definition;
            if (GetMilestoneDefinition(Pair.Key, Definition) && Definition.Category == Category)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UMGMilestoneSubsystem::GetTotalCountByCategory(EMGMilestoneCategory Category) const
{
    int32 Count = 0;
    for (const auto& Pair : MilestoneDefinitions)
    {
        if (Pair.Value.Category == Category)
        {
            Count++;
        }
    }
    return Count;
}

// ===== Persistence =====

void UMGMilestoneSubsystem::SaveMilestoneData()
{
    // In a real implementation, this would serialize to save game
    UE_LOG(LogTemp, Log, TEXT("MGMilestoneSubsystem: Saving milestone data..."));
    bDataDirty = false;
}

void UMGMilestoneSubsystem::LoadMilestoneData()
{
    // In a real implementation, this would deserialize from save game
    UE_LOG(LogTemp, Log, TEXT("MGMilestoneSubsystem: Loading milestone data..."));
}

void UMGMilestoneSubsystem::ResetAllProgress()
{
    // Reset all progress entries
    for (auto& Pair : MilestoneProgress)
    {
        Pair.Value.Status = EMGMilestoneStatus::Locked;
        Pair.Value.CompletionCount = 0;
        Pair.Value.TimeSpentSeconds = 0.0f;

        // Reset requirement progress
        FMGMilestoneDefinition Definition;
        if (GetMilestoneDefinition(Pair.Key, Definition))
        {
            Pair.Value.RequirementProgress = Definition.Requirements;
            if (Definition.bIsSecret)
            {
                Pair.Value.Status = EMGMilestoneStatus::Hidden;
            }
        }
    }

    // Reset stats
    PlayerStats = FMGPlayerStats();
    PlayerStats.FirstPlayDate = FDateTime::Now();
    PlayerStats.LastPlayDate = FDateTime::Now();

    // Clear discovered secrets
    DiscoveredSecrets.Empty();

    // Reset chains
    for (auto& Pair : MilestoneChains)
    {
        Pair.Value.CurrentIndex = 0;
        Pair.Value.bIsComplete = false;
    }

    // Clear notifications
    NotificationQueue.Empty();

    bDataDirty = true;
    UE_LOG(LogTemp, Log, TEXT("MGMilestoneSubsystem: All progress reset"));
}

// ===== Debug =====

void UMGMilestoneSubsystem::DebugCompleteMilestone(FName MilestoneId)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return;

    // Set all requirements to complete
    for (FMGMilestoneRequirement& Req : Progress->RequirementProgress)
    {
        Req.CurrentValue = Req.TargetValue;
    }

    CompleteMilestone(MilestoneId);
}

void UMGMilestoneSubsystem::DebugUnlockAllMilestones()
{
    for (auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Locked ||
            Pair.Value.Status == EMGMilestoneStatus::Hidden)
        {
            UnlockMilestone(Pair.Key);
        }
    }
}

void UMGMilestoneSubsystem::DebugSetStat(EMGStatType StatType, float Value)
{
    PlayerStats.CumulativeStats.Add(StatType, Value);
    PlayerStats.SessionStats.Add(StatType, Value);
    PlayerStats.BestStats.Add(StatType, Value);

    OnStatUpdated.Broadcast(StatType, Value);
    CheckAllMilestonesForStat(StatType);
    bDataDirty = true;
}

void UMGMilestoneSubsystem::DebugPrintMilestoneStatus(FName MilestoneId)
{
    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition))
    {
        UE_LOG(LogTemp, Warning, TEXT("Milestone %s not found"), *MilestoneId.ToString());
        return;
    }

    FMGMilestoneProgress Progress = GetMilestoneProgress(MilestoneId);

    UE_LOG(LogTemp, Log, TEXT("=== Milestone: %s ==="), *Definition.DisplayName.ToString());
    UE_LOG(LogTemp, Log, TEXT("Status: %d"), static_cast<int32>(Progress.Status));
    UE_LOG(LogTemp, Log, TEXT("Overall Progress: %.1f%%"), Progress.GetOverallProgress() * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("Completion Count: %d"), Progress.CompletionCount);

    for (int32 i = 0; i < Progress.RequirementProgress.Num(); i++)
    {
        const FMGMilestoneRequirement& Req = Progress.RequirementProgress[i];
        UE_LOG(LogTemp, Log, TEXT("  Req %d: %.1f / %.1f (%.1f%%)"),
            i, Req.CurrentValue, Req.TargetValue, Req.GetProgressPercent() * 100.0f);
    }
}

// ===== Internal Helpers =====

void UMGMilestoneSubsystem::CheckMilestoneCompletion(FName MilestoneId)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return;

    if (Progress->Status == EMGMilestoneStatus::Completed ||
        Progress->Status == EMGMilestoneStatus::Claimed)
    {
        return;
    }

    if (Progress->AreAllRequirementsMet())
    {
        CompleteMilestone(MilestoneId);
    }
}

void UMGMilestoneSubsystem::CheckAllMilestonesForStat(EMGStatType StatType)
{
    float CumulativeValue = GetStat(StatType);
    float SessionValue = GetSessionStat(StatType);
    float BestValue = GetBestStat(StatType);

    for (auto& Pair : MilestoneProgress)
    {
        if (Pair.Value.Status == EMGMilestoneStatus::Completed ||
            Pair.Value.Status == EMGMilestoneStatus::Claimed)
        {
            continue;
        }

        // Check prerequisites
        if (!ArePrerequisitesMet(Pair.Key))
        {
            continue;
        }

        // Check each requirement
        for (int32 i = 0; i < Pair.Value.RequirementProgress.Num(); i++)
        {
            FMGMilestoneRequirement& Req = Pair.Value.RequirementProgress[i];
            if (Req.StatType != StatType)
            {
                continue;
            }

            float ValueToUse = CumulativeValue;
            if (Req.bRequiresSingleSession)
            {
                ValueToUse = SessionValue;
            }
            else if (Req.TrackingType == EMGMilestoneTrackingType::Maximum)
            {
                ValueToUse = BestValue;
            }

            UpdateMilestoneProgress(Pair.Key, i, ValueToUse);
        }
    }
}

void UMGMilestoneSubsystem::UnlockMilestone(FName MilestoneId)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return;

    if (Progress->Status == EMGMilestoneStatus::Locked ||
        Progress->Status == EMGMilestoneStatus::Hidden)
    {
        Progress->Status = EMGMilestoneStatus::Revealed;
        OnMilestoneUnlocked.Broadcast(MilestoneId);
        bDataDirty = true;
    }
}

void UMGMilestoneSubsystem::CompleteMilestone(FName MilestoneId)
{
    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return;

    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition))
    {
        return;
    }

    Progress->Status = EMGMilestoneStatus::Completed;
    Progress->CompletedTime = FDateTime::Now();
    Progress->CompletionCount++;

    // Create notification
    CreateCompletionNotification(MilestoneId);

    // Reveal secret milestone if completing for first time
    if (Definition.bIsSecret && Progress->CompletionCount == 1)
    {
        DiscoveredSecrets.AddUnique(MilestoneId);
        OnSecretMilestoneDiscovered.Broadcast(MilestoneId);
    }

    // Update season points if seasonal
    if (Definition.bIsSeasonal && ActiveSeason.SeasonId == Definition.SeasonId)
    {
        ActiveSeason.EarnedSeasonPoints += Definition.GetTotalPoints();
    }

    OnMilestoneCompleted.Broadcast(MilestoneId, Definition);

    // Check chains
    for (auto& ChainPair : MilestoneChains)
    {
        FMGMilestoneChain& Chain = ChainPair.Value;
        FName CurrentMilestone = Chain.GetCurrentMilestone();
        if (CurrentMilestone == MilestoneId)
        {
            AdvanceChain(ChainPair.Key);
        }
    }

    // Check for milestones that have this as a prerequisite
    for (const auto& Pair : MilestoneDefinitions)
    {
        if (Pair.Value.PrerequisiteMilestones.Contains(MilestoneId))
        {
            CheckPrerequisites(Pair.Key);
        }
    }

    bDataDirty = true;
}

void UMGMilestoneSubsystem::AdvanceChain(FName ChainId)
{
    FMGMilestoneChain* Chain = MilestoneChains.Find(ChainId);
    if (!Chain) return;

    Chain->CurrentIndex++;
    OnChainProgressUpdated.Broadcast(ChainId, Chain->CurrentIndex);

    if (Chain->CurrentIndex >= Chain->MilestoneSequence.Num())
    {
        Chain->bIsComplete = true;
        OnChainCompleted.Broadcast(ChainId);

        // Create chain completion notification
        FMGMilestoneNotification Notification;
        Notification.bIsChainComplete = true;
        Notification.ChainId = ChainId;
        Notification.Title = Chain->ChainName;
        Notification.Message = FText::FromString(TEXT("Chain Complete!"));
        Notification.Timestamp = FDateTime::Now();
        Notification.RewardsEarned = Chain->ChainCompletionRewards;
        QueueNotification(Notification);
    }

    bDataDirty = true;
}

void UMGMilestoneSubsystem::CheckPrerequisites(FName MilestoneId)
{
    if (!ArePrerequisitesMet(MilestoneId))
    {
        return;
    }

    FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
    if (!Progress) return;

    if (Progress->Status == EMGMilestoneStatus::Locked)
    {
        UnlockMilestone(MilestoneId);
    }
}

bool UMGMilestoneSubsystem::ArePrerequisitesMet(FName MilestoneId) const
{
    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition))
    {
        return false;
    }

    for (const FName& PrereqId : Definition.PrerequisiteMilestones)
    {
        if (!IsMilestoneComplete(PrereqId))
        {
            return false;
        }
    }

    return true;
}

void UMGMilestoneSubsystem::ProcessTimedMilestones(float DeltaTime)
{
    TArray<FName> ExpiredMilestones;

    for (auto& Pair : TimedMilestoneTimers)
    {
        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.0f)
        {
            ExpiredMilestones.Add(Pair.Key);
        }
    }

    for (const FName& MilestoneId : ExpiredMilestones)
    {
        TimedMilestoneTimers.Remove(MilestoneId);

        // Check if milestone was completed before time expired
        FMGMilestoneProgress* Progress = MilestoneProgress.Find(MilestoneId);
        if (Progress && Progress->Status == EMGMilestoneStatus::InProgress)
        {
            // Reset progress for timed milestone
            FMGMilestoneDefinition Definition;
            if (GetMilestoneDefinition(MilestoneId, Definition))
            {
                Progress->RequirementProgress = Definition.Requirements;
            }
        }
    }
}

void UMGMilestoneSubsystem::ProcessSeasonExpiration()
{
    if (!ActiveSeason.bSeasonActive)
    {
        return;
    }

    if (ActiveSeason.IsSeasonExpired())
    {
        ActiveSeason.bSeasonActive = false;
        OnSeasonEnded.Broadcast(ActiveSeason.SeasonId, ActiveSeason.SeasonRank);
        bDataDirty = true;
    }
}

void UMGMilestoneSubsystem::CreateCompletionNotification(FName MilestoneId)
{
    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition))
    {
        return;
    }

    FMGMilestoneNotification Notification;
    Notification.MilestoneId = MilestoneId;
    Notification.Title = Definition.DisplayName;
    Notification.Message = Definition.Description;
    Notification.NewStatus = EMGMilestoneStatus::Completed;
    Notification.ProgressPercent = 1.0f;
    Notification.RewardsEarned = Definition.Rewards;
    Notification.Timestamp = FDateTime::Now();
    Notification.DisplayDuration = 7.0f;

    QueueNotification(Notification);
}

void UMGMilestoneSubsystem::CreateProgressNotification(FName MilestoneId, float Progress)
{
    FMGMilestoneDefinition Definition;
    if (!GetMilestoneDefinition(MilestoneId, Definition))
    {
        return;
    }

    FMGMilestoneNotification Notification;
    Notification.MilestoneId = MilestoneId;
    Notification.Title = Definition.DisplayName;
    Notification.Message = FText::Format(
        NSLOCTEXT("Milestone", "ProgressFormat", "{0}% Complete"),
        FText::AsNumber(FMath::RoundToInt(Progress * 100.0f))
    );
    Notification.NewStatus = EMGMilestoneStatus::InProgress;
    Notification.ProgressPercent = Progress;
    Notification.Timestamp = FDateTime::Now();
    Notification.DisplayDuration = 3.0f;

    QueueNotification(Notification);
}

void UMGMilestoneSubsystem::InitializeDefaultMilestones()
{
    // Racing milestones
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("FirstRace");
        Milestone.DisplayName = FText::FromString(TEXT("First Steps"));
        Milestone.Description = FText::FromString(TEXT("Complete your first race"));
        Milestone.Category = EMGMilestoneCategory::Racing;
        Milestone.Rarity = EMGMilestoneRarity::Common;
        Milestone.PointValue = 5;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("CompleteRace");
        Req.StatType = EMGStatType::RacesCompleted;
        Req.TrackingType = EMGMilestoneTrackingType::Counter;
        Req.TargetValue = 1.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("FirstRaceCurrency");
        Reward.RewardType = EMGRewardType::Currency;
        Reward.Quantity = 500;
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // First Win
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("FirstWin");
        Milestone.DisplayName = FText::FromString(TEXT("Taste of Victory"));
        Milestone.Description = FText::FromString(TEXT("Win your first race"));
        Milestone.Category = EMGMilestoneCategory::Racing;
        Milestone.Rarity = EMGMilestoneRarity::Common;
        Milestone.PointValue = 10;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("WinRace");
        Req.StatType = EMGStatType::RacesWon;
        Req.TrackingType = EMGMilestoneTrackingType::Counter;
        Req.TargetValue = 1.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("FirstWinCurrency");
        Reward.RewardType = EMGRewardType::Currency;
        Reward.Quantity = 1000;
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // Speed Demon
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("SpeedDemon");
        Milestone.DisplayName = FText::FromString(TEXT("Speed Demon"));
        Milestone.Description = FText::FromString(TEXT("Reach 200 mph"));
        Milestone.Category = EMGMilestoneCategory::Racing;
        Milestone.Rarity = EMGMilestoneRarity::Rare;
        Milestone.PointValue = 25;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("TopSpeed");
        Req.StatType = EMGStatType::TopSpeed;
        Req.TrackingType = EMGMilestoneTrackingType::Maximum;
        Req.TargetValue = 200.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("SpeedTitle");
        Reward.RewardType = EMGRewardType::Title;
        Reward.DisplayName = FText::FromString(TEXT("Speed Demon"));
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // Drift King
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("DriftKing");
        Milestone.DisplayName = FText::FromString(TEXT("Drift King"));
        Milestone.Description = FText::FromString(TEXT("Score 100,000 drift points in a single session"));
        Milestone.Category = EMGMilestoneCategory::Drifting;
        Milestone.Rarity = EMGMilestoneRarity::Epic;
        Milestone.PointValue = 50;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("DriftScore");
        Req.StatType = EMGStatType::DriftScore;
        Req.TrackingType = EMGMilestoneTrackingType::Cumulative;
        Req.TargetValue = 100000.0f;
        Req.bRequiresSingleSession = true;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("DriftTitle");
        Reward.RewardType = EMGRewardType::Title;
        Reward.DisplayName = FText::FromString(TEXT("Drift King"));
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // Takedown Master (Secret)
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("TakedownMaster");
        Milestone.DisplayName = FText::FromString(TEXT("Takedown Master"));
        Milestone.Description = FText::FromString(TEXT("Perform 100 takedowns"));
        Milestone.HintText = FText::FromString(TEXT("Show them who's boss"));
        Milestone.Category = EMGMilestoneCategory::Combat;
        Milestone.Rarity = EMGMilestoneRarity::Legendary;
        Milestone.PointValue = 100;
        Milestone.bIsSecret = true;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("Takedowns");
        Req.StatType = EMGStatType::TakedownsDealt;
        Req.TrackingType = EMGMilestoneTrackingType::Cumulative;
        Req.TargetValue = 100.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("TakedownBadge");
        Reward.RewardType = EMGRewardType::Badge;
        Reward.DisplayName = FText::FromString(TEXT("Takedown Master Badge"));
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // Marathon Driver
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("MarathonDriver");
        Milestone.DisplayName = FText::FromString(TEXT("Marathon Driver"));
        Milestone.Description = FText::FromString(TEXT("Drive 1,000 miles total"));
        Milestone.Category = EMGMilestoneCategory::Career;
        Milestone.Rarity = EMGMilestoneRarity::Rare;
        Milestone.PointValue = 30;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("TotalDistance");
        Req.StatType = EMGStatType::TotalDistance;
        Req.TrackingType = EMGMilestoneTrackingType::Cumulative;
        Req.TargetValue = 1000.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("MarathonCurrency");
        Reward.RewardType = EMGRewardType::Currency;
        Reward.Quantity = 5000;
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // Perfect Lap
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("PerfectLap");
        Milestone.DisplayName = FText::FromString(TEXT("Flawless"));
        Milestone.Description = FText::FromString(TEXT("Complete a perfect lap without any collisions"));
        Milestone.Category = EMGMilestoneCategory::Challenge;
        Milestone.Rarity = EMGMilestoneRarity::Uncommon;
        Milestone.PointValue = 15;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("PerfectLaps");
        Req.StatType = EMGStatType::PerfectLaps;
        Req.TrackingType = EMGMilestoneTrackingType::Counter;
        Req.TargetValue = 1.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("PerfectMultiplier");
        Reward.RewardType = EMGRewardType::Multiplier;
        Reward.BonusMultiplier = 1.1f;
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    // Collector
    {
        FMGMilestoneDefinition Milestone;
        Milestone.MilestoneId = FName("Collector");
        Milestone.DisplayName = FText::FromString(TEXT("Collector"));
        Milestone.Description = FText::FromString(TEXT("Own 10 different vehicles"));
        Milestone.Category = EMGMilestoneCategory::Collection;
        Milestone.Rarity = EMGMilestoneRarity::Epic;
        Milestone.PointValue = 40;

        FMGMilestoneRequirement Req;
        Req.RequirementId = FName("VehiclesOwned");
        Req.StatType = EMGStatType::VehiclesOwned;
        Req.TrackingType = EMGMilestoneTrackingType::Counter;
        Req.TargetValue = 10.0f;
        Milestone.Requirements.Add(Req);

        FMGMilestoneReward Reward;
        Reward.RewardId = FName("CollectorVehicle");
        Reward.RewardType = EMGRewardType::Vehicle;
        Reward.UnlockId = FName("Vehicle_Collector_Special");
        Milestone.Rewards.Add(Reward);

        RegisterMilestone(Milestone);
    }

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default milestones"), MilestoneDefinitions.Num());
}

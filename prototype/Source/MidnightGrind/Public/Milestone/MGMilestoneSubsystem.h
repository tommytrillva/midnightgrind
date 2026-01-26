// MidnightGrind - Arcade Street Racing Game
// Milestone Subsystem - Achievement tracking, career progression, and unlock systems

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMilestoneSubsystem.generated.h"

// Forward declarations
class UMGMilestoneSubsystem;

/**
 * EMGMilestoneCategory - Categories of player achievements and milestones
 */
UENUM(BlueprintType)
enum class EMGMilestoneCategory : uint8
{
    Racing          UMETA(DisplayName = "Racing"),
    Drifting        UMETA(DisplayName = "Drifting"),
    Combat          UMETA(DisplayName = "Combat"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Collection      UMETA(DisplayName = "Collection"),
    Social          UMETA(DisplayName = "Social"),
    Career          UMETA(DisplayName = "Career"),
    Challenge       UMETA(DisplayName = "Challenge"),
    Secret          UMETA(DisplayName = "Secret"),
    Seasonal        UMETA(DisplayName = "Seasonal"),
    Legacy          UMETA(DisplayName = "Legacy")
};

/**
 * EMGMilestoneRarity - Rarity tiers for milestones
 */
UENUM(BlueprintType)
enum class EMGMilestoneRarity : uint8
{
    Common          UMETA(DisplayName = "Common"),
    Uncommon        UMETA(DisplayName = "Uncommon"),
    Rare            UMETA(DisplayName = "Rare"),
    Epic            UMETA(DisplayName = "Epic"),
    Legendary       UMETA(DisplayName = "Legendary"),
    Mythic          UMETA(DisplayName = "Mythic")
};

/**
 * EMGMilestoneStatus - Current status of a milestone
 */
UENUM(BlueprintType)
enum class EMGMilestoneStatus : uint8
{
    Locked          UMETA(DisplayName = "Locked"),
    Hidden          UMETA(DisplayName = "Hidden"),
    Revealed        UMETA(DisplayName = "Revealed"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Claimed         UMETA(DisplayName = "Claimed")
};

/**
 * EMGMilestoneTrackingType - How milestone progress is tracked
 */
UENUM(BlueprintType)
enum class EMGMilestoneTrackingType : uint8
{
    Counter         UMETA(DisplayName = "Counter"),
    Cumulative      UMETA(DisplayName = "Cumulative"),
    Maximum         UMETA(DisplayName = "Maximum"),
    Minimum         UMETA(DisplayName = "Minimum"),
    Boolean         UMETA(DisplayName = "Boolean"),
    Sequence        UMETA(DisplayName = "Sequence"),
    Timed           UMETA(DisplayName = "Timed"),
    Compound        UMETA(DisplayName = "Compound")
};

/**
 * EMGRewardType - Types of rewards for completing milestones
 */
UENUM(BlueprintType)
enum class EMGRewardType : uint8
{
    Currency        UMETA(DisplayName = "Currency"),
    Experience      UMETA(DisplayName = "Experience"),
    Vehicle         UMETA(DisplayName = "Vehicle"),
    Part            UMETA(DisplayName = "Part"),
    Cosmetic        UMETA(DisplayName = "Cosmetic"),
    Title           UMETA(DisplayName = "Title"),
    Badge           UMETA(DisplayName = "Badge"),
    Emote           UMETA(DisplayName = "Emote"),
    TrackAccess     UMETA(DisplayName = "Track Access"),
    SpecialAbility  UMETA(DisplayName = "Special Ability"),
    LootBox         UMETA(DisplayName = "Loot Box"),
    Multiplier      UMETA(DisplayName = "Multiplier")
};

/**
 * EMGStatType - Types of statistics that can be tracked
 */
UENUM(BlueprintType)
enum class EMGStatType : uint8
{
    RacesWon        UMETA(DisplayName = "Races Won"),
    RacesCompleted  UMETA(DisplayName = "Races Completed"),
    TotalDistance   UMETA(DisplayName = "Total Distance"),
    DriftScore      UMETA(DisplayName = "Drift Score"),
    NitroUsed       UMETA(DisplayName = "Nitro Used"),
    AirtimeSeconds  UMETA(DisplayName = "Airtime Seconds"),
    NearMisses      UMETA(DisplayName = "Near Misses"),
    TakedownsDealt  UMETA(DisplayName = "Takedowns Dealt"),
    PerfectLaps     UMETA(DisplayName = "Perfect Laps"),
    FirstPlaces     UMETA(DisplayName = "First Places"),
    Overtakes       UMETA(DisplayName = "Overtakes"),
    CleanLaps       UMETA(DisplayName = "Clean Laps"),
    TopSpeed        UMETA(DisplayName = "Top Speed"),
    LongestDrift    UMETA(DisplayName = "Longest Drift"),
    BiggestJump     UMETA(DisplayName = "Biggest Jump"),
    ChainLength     UMETA(DisplayName = "Chain Length"),
    MultiplierMax   UMETA(DisplayName = "Multiplier Max"),
    VehiclesOwned   UMETA(DisplayName = "Vehicles Owned"),
    PartsCollected  UMETA(DisplayName = "Parts Collected"),
    TracksUnlocked  UMETA(DisplayName = "Tracks Unlocked"),
    ChallengesWon   UMETA(DisplayName = "Challenges Won"),
    TournamentsWon  UMETA(DisplayName = "Tournaments Won"),
    ReplaysSaved    UMETA(DisplayName = "Replays Saved"),
    PhotosTaken     UMETA(DisplayName = "Photos Taken"),
    PlaytimeHours   UMETA(DisplayName = "Playtime Hours"),
    MoneyEarned     UMETA(DisplayName = "Money Earned"),
    MoneySpent      UMETA(DisplayName = "Money Spent"),
    DaysPlayed      UMETA(DisplayName = "Days Played"),
    LoginStreak     UMETA(DisplayName = "Login Streak")
};

/**
 * FMGMilestoneRequirement - A single requirement for milestone completion
 */
USTRUCT(BlueprintType)
struct FMGMilestoneRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequirementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGStatType StatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMilestoneTrackingType TrackingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ContextFilter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresSingleSession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresSpecificVehicle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredVehicleId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresSpecificTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredTrackId;

    FMGMilestoneRequirement()
        : RequirementId(NAME_None)
        , StatType(EMGStatType::RacesWon)
        , TrackingType(EMGMilestoneTrackingType::Counter)
        , TargetValue(1.0f)
        , CurrentValue(0.0f)
        , ContextFilter(NAME_None)
        , bRequiresSingleSession(false)
        , bRequiresSpecificVehicle(false)
        , RequiredVehicleId(NAME_None)
        , bRequiresSpecificTrack(false)
        , RequiredTrackId(NAME_None)
    {}

    float GetProgressPercent() const
    {
        return TargetValue > 0.0f ? FMath::Clamp(CurrentValue / TargetValue, 0.0f, 1.0f) : 0.0f;
    }

    bool IsComplete() const
    {
        return CurrentValue >= TargetValue;
    }
};

/**
 * FMGMilestoneReward - A reward granted upon milestone completion
 */
USTRUCT(BlueprintType)
struct FMGMilestoneReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RewardId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGRewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName UnlockId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPremium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BonusMultiplier;

    FMGMilestoneReward()
        : RewardId(NAME_None)
        , RewardType(EMGRewardType::Currency)
        , Quantity(100)
        , UnlockId(NAME_None)
        , bIsPremium(false)
        , BonusMultiplier(1.0f)
    {}
};

/**
 * FMGMilestoneDefinition - Complete definition of a milestone
 */
USTRUCT(BlueprintType)
struct FMGMilestoneDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MilestoneId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText HintText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMilestoneCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMilestoneRarity Rarity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PointValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMilestoneRequirement> Requirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMilestoneReward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> PrerequisiteMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> IconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> LockedIconTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSecret;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxCompletions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeLimitSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsSeasonal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SeasonId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SortOrder;

    FMGMilestoneDefinition()
        : MilestoneId(NAME_None)
        , Category(EMGMilestoneCategory::Racing)
        , Rarity(EMGMilestoneRarity::Common)
        , PointValue(10)
        , bIsSecret(false)
        , bIsRepeatable(false)
        , MaxCompletions(1)
        , bHasTimeLimit(false)
        , TimeLimitSeconds(0.0f)
        , bIsSeasonal(false)
        , SeasonId(NAME_None)
        , SortOrder(0)
    {}

    int32 GetRarityPointBonus() const
    {
        switch (Rarity)
        {
            case EMGMilestoneRarity::Common: return 0;
            case EMGMilestoneRarity::Uncommon: return 5;
            case EMGMilestoneRarity::Rare: return 15;
            case EMGMilestoneRarity::Epic: return 30;
            case EMGMilestoneRarity::Legendary: return 50;
            case EMGMilestoneRarity::Mythic: return 100;
            default: return 0;
        }
    }

    int32 GetTotalPoints() const
    {
        return PointValue + GetRarityPointBonus();
    }
};

/**
 * FMGMilestoneProgress - Player's progress on a specific milestone
 */
USTRUCT(BlueprintType)
struct FMGMilestoneProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MilestoneId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMilestoneStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMilestoneRequirement> RequirementProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CompletionCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime FirstStartedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CompletedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ClaimedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeSpentSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bNotificationSent;

    FMGMilestoneProgress()
        : MilestoneId(NAME_None)
        , Status(EMGMilestoneStatus::Locked)
        , CompletionCount(0)
        , TimeSpentSeconds(0.0f)
        , bNotificationSent(false)
    {}

    float GetOverallProgress() const
    {
        if (RequirementProgress.Num() == 0) return 0.0f;
        float TotalProgress = 0.0f;
        for (const FMGMilestoneRequirement& Req : RequirementProgress)
        {
            TotalProgress += Req.GetProgressPercent();
        }
        return TotalProgress / RequirementProgress.Num();
    }

    bool AreAllRequirementsMet() const
    {
        for (const FMGMilestoneRequirement& Req : RequirementProgress)
        {
            if (!Req.IsComplete()) return false;
        }
        return RequirementProgress.Num() > 0;
    }
};

/**
 * FMGPlayerStats - Aggregated player statistics
 */
USTRUCT(BlueprintType)
struct FMGPlayerStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGStatType, float> CumulativeStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGStatType, float> SessionStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<EMGStatType, float> BestStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalMilestonesCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalMilestonePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SecretMilestonesFound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime FirstPlayDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastPlayDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPlaytimeHours;

    FMGPlayerStats()
        : TotalMilestonesCompleted(0)
        , TotalMilestonePoints(0)
        , SecretMilestonesFound(0)
        , TotalPlaytimeHours(0.0f)
    {}

    float GetStat(EMGStatType StatType, bool bSessionOnly = false) const
    {
        if (bSessionOnly)
        {
            const float* Value = SessionStats.Find(StatType);
            return Value ? *Value : 0.0f;
        }
        const float* Value = CumulativeStats.Find(StatType);
        return Value ? *Value : 0.0f;
    }

    float GetBestStat(EMGStatType StatType) const
    {
        const float* Value = BestStats.Find(StatType);
        return Value ? *Value : 0.0f;
    }
};

/**
 * FMGMilestoneChain - A series of related milestones
 */
USTRUCT(BlueprintType)
struct FMGMilestoneChain
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChainId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ChainName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText ChainDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> MilestoneSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMilestoneReward> ChainCompletionRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UTexture2D> ChainBadgeIcon;

    FMGMilestoneChain()
        : ChainId(NAME_None)
        , CurrentIndex(0)
        , bIsComplete(false)
    {}

    float GetChainProgress() const
    {
        return MilestoneSequence.Num() > 0 ?
            static_cast<float>(CurrentIndex) / MilestoneSequence.Num() : 0.0f;
    }

    FName GetCurrentMilestone() const
    {
        return MilestoneSequence.IsValidIndex(CurrentIndex) ?
            MilestoneSequence[CurrentIndex] : NAME_None;
    }
};

/**
 * FMGMilestoneNotification - Notification for milestone events
 */
USTRUCT(BlueprintType)
struct FMGMilestoneNotification
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName MilestoneId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMGMilestoneStatus NewStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProgressPercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMilestoneReward> RewardsEarned;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsChainComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ChainId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DisplayDuration;

    FMGMilestoneNotification()
        : MilestoneId(NAME_None)
        , NewStatus(EMGMilestoneStatus::InProgress)
        , ProgressPercent(0.0f)
        , bIsChainComplete(false)
        , ChainId(NAME_None)
        , DisplayDuration(5.0f)
    {}
};

/**
 * FMGSeasonMilestones - Milestones specific to a season
 */
USTRUCT(BlueprintType)
struct FMGSeasonMilestones
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName SeasonId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText SeasonName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EndDate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> SeasonalMilestoneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSeasonPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EarnedSeasonPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SeasonRank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMGMilestoneReward> SeasonRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSeasonActive;

    FMGSeasonMilestones()
        : SeasonId(NAME_None)
        , TotalSeasonPoints(0)
        , EarnedSeasonPoints(0)
        , SeasonRank(0)
        , bSeasonActive(false)
    {}

    float GetSeasonProgress() const
    {
        return TotalSeasonPoints > 0 ?
            static_cast<float>(EarnedSeasonPoints) / TotalSeasonPoints : 0.0f;
    }

    bool IsSeasonExpired() const
    {
        return FDateTime::Now() > EndDate;
    }

    FTimespan GetTimeRemaining() const
    {
        FDateTime Now = FDateTime::Now();
        return EndDate > Now ? EndDate - Now : FTimespan::Zero();
    }
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMilestoneProgressUpdated, FName, MilestoneId, float, NewProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMilestoneCompleted, FName, MilestoneId, const FMGMilestoneDefinition&, Definition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMilestoneRewardsClaimed, FName, MilestoneId, const TArray<FMGMilestoneReward>&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMilestoneUnlocked, FName, MilestoneId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSecretMilestoneDiscovered, FName, MilestoneId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnChainProgressUpdated, FName, ChainId, int32, NewIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChainCompleted, FName, ChainId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnStatUpdated, EMGStatType, StatType, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMilestoneNotification, const FMGMilestoneNotification&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSeasonStarted, FName, SeasonId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnSeasonEnded, FName, SeasonId, int32, FinalRank);

/**
 * UMGMilestoneSubsystem
 *
 * Manages the comprehensive achievement and milestone system for Midnight Grind.
 * Features include:
 * - Achievement tracking with multiple requirement types
 * - Career progression milestones
 * - Statistics aggregation and persistence
 * - Milestone chains and sequences
 * - Seasonal achievements
 * - Secret/hidden milestones
 * - Reward distribution
 * - Progress notifications
 */
UCLASS()
class MIDNIGHTGRIND_API UMGMilestoneSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UMGMilestoneSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Tick functionality
    void TickMilestoneSystem(float DeltaTime);

    // ===== Milestone Definition Management =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Definition")
    void RegisterMilestone(const FMGMilestoneDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Definition")
    void UnregisterMilestone(FName MilestoneId);

    UFUNCTION(BlueprintPure, Category = "Milestone|Definition")
    bool GetMilestoneDefinition(FName MilestoneId, FMGMilestoneDefinition& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Definition")
    TArray<FMGMilestoneDefinition> GetAllMilestones() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Definition")
    TArray<FMGMilestoneDefinition> GetMilestonesByCategory(EMGMilestoneCategory Category) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Definition")
    TArray<FMGMilestoneDefinition> GetMilestonesByRarity(EMGMilestoneRarity Rarity) const;

    // ===== Progress Tracking =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Progress")
    void UpdateMilestoneProgress(FName MilestoneId, int32 RequirementIndex, float NewValue);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Progress")
    void IncrementMilestoneProgress(FName MilestoneId, int32 RequirementIndex, float Amount);

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    FMGMilestoneProgress GetMilestoneProgress(FName MilestoneId) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    float GetMilestoneProgressPercent(FName MilestoneId) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    EMGMilestoneStatus GetMilestoneStatus(FName MilestoneId) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    bool IsMilestoneComplete(FName MilestoneId) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    bool IsMilestoneUnlocked(FName MilestoneId) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    TArray<FMGMilestoneProgress> GetInProgressMilestones() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Progress")
    TArray<FMGMilestoneProgress> GetCompletedMilestones() const;

    // ===== Stat Tracking =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Stats")
    void UpdateStat(EMGStatType StatType, float Value, bool bIsMaximum = false);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Stats")
    void IncrementStat(EMGStatType StatType, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Stats")
    void ResetSessionStats();

    UFUNCTION(BlueprintPure, Category = "Milestone|Stats")
    float GetStat(EMGStatType StatType) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Stats")
    float GetSessionStat(EMGStatType StatType) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Stats")
    float GetBestStat(EMGStatType StatType) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Stats")
    FMGPlayerStats GetAllStats() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone|Stats")
    void RecordStatForMilestones(EMGStatType StatType, float Value, FName ContextId = NAME_None);

    // ===== Rewards =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Rewards")
    bool ClaimMilestoneRewards(FName MilestoneId, TArray<FMGMilestoneReward>& OutRewards);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Rewards")
    bool ClaimAllPendingRewards(TArray<FMGMilestoneReward>& OutRewards);

    UFUNCTION(BlueprintPure, Category = "Milestone|Rewards")
    TArray<FMGMilestoneReward> GetPendingRewards() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Rewards")
    bool HasUnclaimedRewards() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Rewards")
    int32 GetUnclaimedRewardCount() const;

    // ===== Milestone Chains =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Chains")
    void RegisterChain(const FMGMilestoneChain& Chain);

    UFUNCTION(BlueprintPure, Category = "Milestone|Chains")
    bool GetChain(FName ChainId, FMGMilestoneChain& OutChain) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Chains")
    TArray<FMGMilestoneChain> GetAllChains() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Chains")
    float GetChainProgress(FName ChainId) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Chains")
    bool IsChainComplete(FName ChainId) const;

    // ===== Seasonal Milestones =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Seasonal")
    void SetActiveSeason(const FMGSeasonMilestones& Season);

    UFUNCTION(BlueprintPure, Category = "Milestone|Seasonal")
    FMGSeasonMilestones GetActiveSeason() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Seasonal")
    bool IsSeasonActive() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Seasonal")
    FTimespan GetSeasonTimeRemaining() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Seasonal")
    TArray<FMGMilestoneDefinition> GetSeasonalMilestones() const;

    // ===== Secret Milestones =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Secret")
    void RevealSecretMilestone(FName MilestoneId);

    UFUNCTION(BlueprintPure, Category = "Milestone|Secret")
    TArray<FMGMilestoneDefinition> GetDiscoveredSecretMilestones() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Secret")
    int32 GetTotalSecretMilestoneCount() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Secret")
    int32 GetDiscoveredSecretCount() const;

    // ===== Notifications =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Notifications")
    void QueueNotification(const FMGMilestoneNotification& Notification);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Notifications")
    bool PopNextNotification(FMGMilestoneNotification& OutNotification);

    UFUNCTION(BlueprintPure, Category = "Milestone|Notifications")
    int32 GetPendingNotificationCount() const;

    UFUNCTION(BlueprintCallable, Category = "Milestone|Notifications")
    void ClearAllNotifications();

    // ===== Aggregate Data =====

    UFUNCTION(BlueprintPure, Category = "Milestone|Aggregate")
    int32 GetTotalMilestonePoints() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Aggregate")
    int32 GetTotalCompletedMilestones() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Aggregate")
    float GetOverallCompletionPercent() const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Aggregate")
    int32 GetCompletedCountByCategory(EMGMilestoneCategory Category) const;

    UFUNCTION(BlueprintPure, Category = "Milestone|Aggregate")
    int32 GetTotalCountByCategory(EMGMilestoneCategory Category) const;

    // ===== Persistence =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Persistence")
    void SaveMilestoneData();

    UFUNCTION(BlueprintCallable, Category = "Milestone|Persistence")
    void LoadMilestoneData();

    UFUNCTION(BlueprintCallable, Category = "Milestone|Persistence")
    void ResetAllProgress();

    // ===== Debug =====

    UFUNCTION(BlueprintCallable, Category = "Milestone|Debug")
    void DebugCompleteMilestone(FName MilestoneId);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Debug")
    void DebugUnlockAllMilestones();

    UFUNCTION(BlueprintCallable, Category = "Milestone|Debug")
    void DebugSetStat(EMGStatType StatType, float Value);

    UFUNCTION(BlueprintCallable, Category = "Milestone|Debug")
    void DebugPrintMilestoneStatus(FName MilestoneId);

    // ===== Events =====

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnMilestoneProgressUpdated OnMilestoneProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnMilestoneCompleted OnMilestoneCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnMilestoneRewardsClaimed OnMilestoneRewardsClaimed;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnMilestoneUnlocked OnMilestoneUnlocked;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnSecretMilestoneDiscovered OnSecretMilestoneDiscovered;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnChainProgressUpdated OnChainProgressUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnChainCompleted OnChainCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnStatUpdated OnStatUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnMilestoneNotification OnMilestoneNotification;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnSeasonStarted OnSeasonStarted;

    UPROPERTY(BlueprintAssignable, Category = "Milestone|Events")
    FMGOnSeasonEnded OnSeasonEnded;

protected:
    // Internal helpers
    void CheckMilestoneCompletion(FName MilestoneId);
    void CheckAllMilestonesForStat(EMGStatType StatType);
    void UnlockMilestone(FName MilestoneId);
    void CompleteMilestone(FName MilestoneId);
    void AdvanceChain(FName ChainId);
    void CheckPrerequisites(FName MilestoneId);
    bool ArePrerequisitesMet(FName MilestoneId) const;
    void ProcessTimedMilestones(float DeltaTime);
    void ProcessSeasonExpiration();
    void CreateCompletionNotification(FName MilestoneId);
    void CreateProgressNotification(FName MilestoneId, float Progress);
    void InitializeDefaultMilestones();

private:
    // Milestone definitions
    UPROPERTY()
    TMap<FName, FMGMilestoneDefinition> MilestoneDefinitions;

    // Player progress
    UPROPERTY()
    TMap<FName, FMGMilestoneProgress> MilestoneProgress;

    // Player stats
    UPROPERTY()
    FMGPlayerStats PlayerStats;

    // Milestone chains
    UPROPERTY()
    TMap<FName, FMGMilestoneChain> MilestoneChains;

    // Active season
    UPROPERTY()
    FMGSeasonMilestones ActiveSeason;

    // Notification queue
    UPROPERTY()
    TArray<FMGMilestoneNotification> NotificationQueue;

    // Discovered secrets
    UPROPERTY()
    TArray<FName> DiscoveredSecrets;

    // Timed milestone tracking
    UPROPERTY()
    TMap<FName, float> TimedMilestoneTimers;

    // Session start time
    UPROPERTY()
    FDateTime SessionStartTime;

    // Tick timer handle
    FTimerHandle TickTimerHandle;

    // Dirty flag for save optimization
    bool bDataDirty;

    // Auto-save interval
    float AutoSaveInterval;
    float TimeSinceLastSave;

    // Progress thresholds for notifications
    TArray<float> ProgressNotificationThresholds;
};

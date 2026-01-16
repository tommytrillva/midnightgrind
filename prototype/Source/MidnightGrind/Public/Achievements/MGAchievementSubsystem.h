// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAchievementSubsystem.generated.h"

/**
 * Achievement Category
 */
UENUM(BlueprintType)
enum class EMGAchievementCategory : uint8
{
	Racing			UMETA(DisplayName = "Racing"),
	Career			UMETA(DisplayName = "Career"),
	Social			UMETA(DisplayName = "Social"),
	Collection		UMETA(DisplayName = "Collection"),
	Mastery			UMETA(DisplayName = "Mastery"),
	Exploration		UMETA(DisplayName = "Exploration"),
	Challenge		UMETA(DisplayName = "Challenge"),
	Secret			UMETA(DisplayName = "Secret")
};

/**
 * Achievement Rarity
 */
UENUM(BlueprintType)
enum class EMGAchievementRarity : uint8
{
	Common			UMETA(DisplayName = "Common"),
	Uncommon		UMETA(DisplayName = "Uncommon"),
	Rare			UMETA(DisplayName = "Rare"),
	Epic			UMETA(DisplayName = "Epic"),
	Legendary		UMETA(DisplayName = "Legendary")
};

/**
 * Achievement Stat Type (what triggers progress)
 */
UENUM(BlueprintType)
enum class EMGAchievementStatType : uint8
{
	// Racing stats
	RacesCompleted,
	RacesWon,
	FirstPlaceFinishes,
	PodiumFinishes,
	PerfectRaces,
	TotalDistance,
	TotalDriftDistance,
	TotalAirTime,
	TotalNitroUsed,
	NearMisses,
	Overtakes,

	// Time-based
	TotalPlayTime,
	TimeInFirstPlace,
	FastestLap,

	// Multiplayer
	OnlineRacesWon,
	OnlineRacesCompleted,
	TournamentWins,
	TournamentParticipations,
	RankedWins,

	// Social
	CrewsJoined,
	CrewChallengesCompleted,
	PhotosTaken,
	PhotosShared,
	FriendsAdded,

	// Collection
	VehiclesOwned,
	VehiclesMaxUpgraded,
	LiveriesCreated,
	PartsCollected,

	// Economy
	TotalCashEarned,
	TotalXPEarned,
	TotalRepEarned,
	ItemsPurchased,

	// Track-specific
	TrackCompleted,
	TrackMastered,
	AllTracksCompleted,

	// Special
	ConsecutiveWins,
	ConsecutivePodiums,
	ComebackWins,
	PhotoFinishes,

	// Custom (for specific achievements)
	Custom
};

/**
 * Achievement Reward
 */
USTRUCT(BlueprintType)
struct FMGAchievementReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 CashReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 XPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ReputationReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName TitleUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName BadgeUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName VehicleUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	FName ItemUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 PlatformGamerScore = 0; // Xbox/Steam points
};

/**
 * Achievement Tier (for progressive achievements)
 */
USTRUCT(BlueprintType)
struct FMGAchievementTier
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	int32 TierLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	FText TierName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	int32 RequiredProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tier")
	FMGAchievementReward Reward;
};

/**
 * Achievement Definition
 */
USTRUCT(BlueprintType)
struct FMGAchievementDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName AchievementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText HiddenDescription; // Shown before unlock for secret achievements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementCategory Category = EMGAchievementCategory::Racing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementStatType StatType = EMGAchievementStatType::RacesCompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	int32 TargetProgress = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsSecret = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsProgressive = false; // Has multiple tiers

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	TArray<FMGAchievementTier> Tiers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FMGAchievementReward Reward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	UTexture2D* LockedIcon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName PlatformAchievementID; // Steam/Xbox/PlayStation ID

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	TArray<FName> PrerequisiteAchievements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName RequiredTrack; // For track-specific achievements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName RequiredVehicle; // For vehicle-specific achievements
};

/**
 * Achievement Progress Data
 */
USTRUCT(BlueprintType)
struct FMGAchievementProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FName AchievementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentProgress = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime UnlockTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bRewardClaimed = false;
};

/**
 * In-Game Badge
 */
USTRUCT(BlueprintType)
struct FMGBadge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FName BadgeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	bool bIsEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Badge")
	FDateTime UnlockTime;
};

/**
 * Player Title
 */
USTRUCT(BlueprintType)
struct FMGPlayerTitle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FName TitleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FText DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FLinearColor TitleColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Title")
	FDateTime UnlockTime;
};

/**
 * Milestone Data
 */
USTRUCT(BlueprintType)
struct FMGMilestone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FName MilestoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	EMGAchievementStatType StatType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	TArray<int32> Thresholds; // e.g., 100, 500, 1000, 5000, 10000

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	int32 CurrentThresholdIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
	TArray<FMGAchievementReward> ThresholdRewards;
};

/**
 * Achievement Stats Snapshot
 */
USTRUCT(BlueprintType)
struct FMGAchievementStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalAchievements = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 UnlockedAchievements = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalGamerScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 EarnedGamerScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CompletionPercentage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 BadgesUnlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TitlesUnlocked = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FDateTime LastAchievementUnlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FName RarestAchievement;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementUnlocked, const FMGAchievementDefinition&, Achievement, int32, TierUnlocked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementProgress, FName, AchievementID, float, ProgressPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBadgeUnlocked, const FMGBadge&, Badge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTitleUnlocked, const FMGPlayerTitle&, Title);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMilestoneReached, const FMGMilestone&, Milestone, int32, ThresholdIndex);

/**
 * Achievement Subsystem
 * Manages achievements, badges, titles, and milestones
 */
UCLASS()
class MIDNIGHTGRIND_API UMGAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementProgress OnAchievementProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnBadgeUnlocked OnBadgeUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTitleUnlocked OnTitleUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMilestoneReached OnMilestoneReached;

	// ==========================================
	// STAT REPORTING
	// ==========================================

	/** Report stat increment */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportStatIncrement(EMGAchievementStatType StatType, int32 Amount = 1);

	/** Report stat value (for max-based stats like fastest lap) */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportStatValue(EMGAchievementStatType StatType, int32 Value);

	/** Report custom achievement progress */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportCustomProgress(FName AchievementID, int32 Progress);

	/** Report race completion with context */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	void ReportRaceCompletion(int32 Position, bool bIsOnline, bool bIsPerfect, FName TrackID, FName VehicleID);

	// ==========================================
	// ACHIEVEMENT QUERIES
	// ==========================================

	/** Get all achievements */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetAllAchievements() const;

	/** Get achievements by category */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetAchievementsByCategory(EMGAchievementCategory Category) const;

	/** Get achievement definition */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	bool GetAchievement(FName AchievementID, FMGAchievementDefinition& OutAchievement) const;

	/** Get achievement progress */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	FMGAchievementProgress GetAchievementProgress(FName AchievementID) const;

	/** Is achievement unlocked */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	bool IsAchievementUnlocked(FName AchievementID) const;

	/** Get unlocked achievements */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetUnlockedAchievements() const;

	/** Get locked achievements */
	UFUNCTION(BlueprintPure, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetLockedAchievements() const;

	/** Get recently unlocked achievements */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetRecentlyUnlocked(int32 Count = 5) const;

	/** Get nearest to completion achievements */
	UFUNCTION(BlueprintCallable, Category = "Achievements")
	TArray<FMGAchievementDefinition> GetNearestToCompletion(int32 Count = 5) const;

	// ==========================================
	// BADGES
	// ==========================================

	/** Get all unlocked badges */
	UFUNCTION(BlueprintPure, Category = "Badges")
	TArray<FMGBadge> GetUnlockedBadges() const;

	/** Get equipped badges */
	UFUNCTION(BlueprintPure, Category = "Badges")
	TArray<FMGBadge> GetEquippedBadges() const;

	/** Equip badge */
	UFUNCTION(BlueprintCallable, Category = "Badges")
	bool EquipBadge(FName BadgeID, int32 SlotIndex);

	/** Unequip badge */
	UFUNCTION(BlueprintCallable, Category = "Badges")
	void UnequipBadge(int32 SlotIndex);

	/** Get max badge slots */
	UFUNCTION(BlueprintPure, Category = "Badges")
	int32 GetMaxBadgeSlots() const { return MaxBadgeSlots; }

	// ==========================================
	// TITLES
	// ==========================================

	/** Get all unlocked titles */
	UFUNCTION(BlueprintPure, Category = "Titles")
	TArray<FMGPlayerTitle> GetUnlockedTitles() const;

	/** Get current equipped title */
	UFUNCTION(BlueprintPure, Category = "Titles")
	FMGPlayerTitle GetEquippedTitle() const;

	/** Equip title */
	UFUNCTION(BlueprintCallable, Category = "Titles")
	bool EquipTitle(FName TitleID);

	// ==========================================
	// MILESTONES
	// ==========================================

	/** Get all milestones */
	UFUNCTION(BlueprintPure, Category = "Milestones")
	TArray<FMGMilestone> GetAllMilestones() const { return Milestones; }

	/** Get milestone progress */
	UFUNCTION(BlueprintCallable, Category = "Milestones")
	float GetMilestoneProgress(FName MilestoneID) const;

	// ==========================================
	// STATS
	// ==========================================

	/** Get achievement stats */
	UFUNCTION(BlueprintPure, Category = "Stats")
	FMGAchievementStats GetAchievementStats() const;

	/** Get stat value */
	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetStatValue(EMGAchievementStatType StatType) const;

	// ==========================================
	// REWARDS
	// ==========================================

	/** Claim achievement reward */
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	bool ClaimAchievementReward(FName AchievementID);

	/** Has unclaimed rewards */
	UFUNCTION(BlueprintPure, Category = "Rewards")
	bool HasUnclaimedRewards() const;

	/** Get unclaimed reward count */
	UFUNCTION(BlueprintPure, Category = "Rewards")
	int32 GetUnclaimedRewardCount() const;

	// ==========================================
	// PLATFORM INTEGRATION
	// ==========================================

	/** Sync achievements with platform */
	UFUNCTION(BlueprintCallable, Category = "Platform")
	void SyncWithPlatform();

	/** Get platform achievement status */
	UFUNCTION(BlueprintPure, Category = "Platform")
	bool IsPlatformAchievementUnlocked(FName PlatformAchievementID) const;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** All achievement definitions */
	UPROPERTY()
	TArray<FMGAchievementDefinition> AchievementDefinitions;

	/** Achievement progress */
	UPROPERTY()
	TMap<FName, FMGAchievementProgress> AchievementProgressMap;

	/** Stat values */
	UPROPERTY()
	TMap<EMGAchievementStatType, int32> StatValues;

	/** Unlocked badges */
	UPROPERTY()
	TArray<FMGBadge> UnlockedBadges;

	/** Equipped badge IDs by slot */
	UPROPERTY()
	TArray<FName> EquippedBadgeSlots;

	/** Unlocked titles */
	UPROPERTY()
	TArray<FMGPlayerTitle> UnlockedTitles;

	/** Equipped title ID */
	UPROPERTY()
	FName EquippedTitleID;

	/** Milestones */
	UPROPERTY()
	TArray<FMGMilestone> Milestones;

	/** Max badge slots */
	UPROPERTY()
	int32 MaxBadgeSlots = 3;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize achievement definitions */
	void InitializeAchievements();

	/** Initialize milestones */
	void InitializeMilestones();

	/** Check achievement unlock conditions */
	void CheckAchievementUnlocks(EMGAchievementStatType StatType);

	/** Unlock achievement */
	void UnlockAchievement(FName AchievementID, int32 Tier = 0);

	/** Unlock badge */
	void UnlockBadge(FName BadgeID);

	/** Unlock title */
	void UnlockTitle(FName TitleID);

	/** Check milestone progress */
	void CheckMilestoneProgress(EMGAchievementStatType StatType);

	/** Report to platform */
	void ReportToPlatform(FName AchievementID);

	/** Apply rewards */
	void ApplyRewards(const FMGAchievementReward& Reward);

	/** Load progress */
	void LoadProgress();

	/** Save progress */
	void SaveProgress();
};

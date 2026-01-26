// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGAchievementSubsystem.generated.h"

/**
 * Achievement category
 */
UENUM(BlueprintType)
enum class EMGAchievementCategory : uint8
{
	Racing UMETA(DisplayName = "Racing"),
	Drifting UMETA(DisplayName = "Drifting"),
	Collection UMETA(DisplayName = "Collection"),
	Customization UMETA(DisplayName = "Customization"),
	Social UMETA(DisplayName = "Social"),
	Police UMETA(DisplayName = "Police"),
	Exploration UMETA(DisplayName = "Exploration"),
	Story UMETA(DisplayName = "Story"),
	Skill UMETA(DisplayName = "Skill"),
	Secret UMETA(DisplayName = "Secret")
};

/**
 * Achievement rarity
 */
UENUM(BlueprintType)
enum class EMGAchievementRarity : uint8
{
	Common UMETA(DisplayName = "Common"),
	Uncommon UMETA(DisplayName = "Uncommon"),
	Rare UMETA(DisplayName = "Rare"),
	Epic UMETA(DisplayName = "Epic"),
	Legendary UMETA(DisplayName = "Legendary")
};

/**
 * Achievement stat type for tracking
 */
UENUM(BlueprintType)
enum class EMGAchievementStatType : uint8
{
	// Racing
	RacesWon UMETA(DisplayName = "Races Won"),
	RacesCompleted UMETA(DisplayName = "Races Completed"),
	FirstPlaceFinishes UMETA(DisplayName = "First Place Finishes"),
	PerfectStarts UMETA(DisplayName = "Perfect Starts"),
	PhotoFinishes UMETA(DisplayName = "Photo Finishes"),
	ComeBacks UMETA(DisplayName = "Come From Behind Wins"),

	// Drifting
	TotalDriftScore UMETA(DisplayName = "Total Drift Score"),
	LongestDrift UMETA(DisplayName = "Longest Drift"),
	DriftCombos UMETA(DisplayName = "Drift Combos"),
	DriftRacesWon UMETA(DisplayName = "Drift Races Won"),

	// Speed
	TopSpeedReached UMETA(DisplayName = "Top Speed Reached"),
	TotalMilesDriven UMETA(DisplayName = "Total Miles Driven"),
	NearMisses UMETA(DisplayName = "Near Misses"),
	AirTime UMETA(DisplayName = "Air Time"),

	// Collection
	VehiclesOwned UMETA(DisplayName = "Vehicles Owned"),
	PartsOwned UMETA(DisplayName = "Parts Owned"),
	PinkSlipsWon UMETA(DisplayName = "Pink Slips Won"),
	LegendaryVehicles UMETA(DisplayName = "Legendary Vehicles"),

	// Economy
	TotalCashEarned UMETA(DisplayName = "Total Cash Earned"),
	TotalCashSpent UMETA(DisplayName = "Total Cash Spent"),
	BiggestPurchase UMETA(DisplayName = "Biggest Purchase"),

	// Police
	PursuitEscapes UMETA(DisplayName = "Pursuit Escapes"),
	CopsDisabled UMETA(DisplayName = "Cops Disabled"),
	RoadblocksEvaded UMETA(DisplayName = "Roadblocks Evaded"),
	MaxHeatSurvived UMETA(DisplayName = "Max Heat Level Survived"),
	LongestPursuit UMETA(DisplayName = "Longest Pursuit"),

	// Social
	RivalsDefeated UMETA(DisplayName = "Rivals Defeated"),
	NemesisDefeated UMETA(DisplayName = "Nemesis Defeated"),
	CrewRacesWon UMETA(DisplayName = "Crew Races Won"),
	CalloutsChallenged UMETA(DisplayName = "Callouts Answered"),

	// Story
	MissionsCompleted UMETA(DisplayName = "Missions Completed"),
	ChaptersCompleted UMETA(DisplayName = "Chapters Completed"),
	BossesDefeated UMETA(DisplayName = "Bosses Defeated"),

	// Skill
	PerfectShifts UMETA(DisplayName = "Perfect Shifts"),
	CleanLaps UMETA(DisplayName = "Clean Laps"),
	NitrousUsed UMETA(DisplayName = "Nitrous Used"),

	// Time
	TotalPlaytime UMETA(DisplayName = "Total Playtime"),
	NightRacing UMETA(DisplayName = "Night Racing Hours"),
	RainRacing UMETA(DisplayName = "Rain Racing Hours"),

	// Custom
	Custom UMETA(DisplayName = "Custom")
};

/**
 * Achievement definition
 */
USTRUCT(BlueprintType)
struct FMGAchievementDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FName AchievementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement", meta = (MultiLine = true))
	FText HiddenDescription; // Shown before unlock for secret achievements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementCategory Category = EMGAchievementCategory::Racing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	EMGAchievementRarity Rarity = EMGAchievementRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievement")
	bool bIsSecret = false;

	// Progress tracking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	EMGAchievementStatType StatType = EMGAchievementStatType::RacesWon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FName CustomStatID; // For custom stat types

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 TargetValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bShowProgress = true;

	// Rewards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int64 CashReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 REPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	int32 XPReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName UnlockVehicleID; // Vehicle unlocked by achievement

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName UnlockPartID; // Part unlocked by achievement

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rewards")
	FName UnlockVisualID; // Visual item unlocked

	// Visuals
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> LockedIcon;

	// Prerequisites
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	TArray<FName> RequiredAchievements; // Must unlock these first

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prerequisites")
	int32 RequiredLevel = 0;

	// Tiers (for multi-tier achievements like "Win 10/50/100 races")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	bool bHasTiers = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	TArray<int32> TierThresholds; // e.g., [10, 50, 100]

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiers")
	TArray<FText> TierNames; // e.g., ["Bronze", "Silver", "Gold"]
};

/**
 * Achievement progress state
 */
USTRUCT(BlueprintType)
struct FMGAchievementProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FName AchievementID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 CurrentTier = 0; // For tiered achievements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	FDateTime UnlockTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bRewardsClaimed = false;
};

/**
 * Achievement notification data
 */
USTRUCT(BlueprintType)
struct FMGAchievementNotification
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FMGAchievementDefinition Achievement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	int32 TierUnlocked = 0; // For tiered achievements

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	bool bIsNewUnlock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notification")
	FDateTime UnlockTime;
};

/**
 * Stat change event
 */
USTRUCT(BlueprintType)
struct FMGStatChangeEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	EMGAchievementStatType StatType = EMGAchievementStatType::RacesWon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName CustomStatID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 OldValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 NewValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	int32 Delta = 0;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAchievementUnlocked, const FMGAchievementNotification&, Notification);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAchievementProgress, FName, AchievementID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTierUnlocked, FName, AchievementID, int32, TierIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChanged, const FMGStatChangeEvent&, Event);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardsClaimed, FName, AchievementID);

/**
 * Achievement System Subsystem
 *
 * Tracks player stats and awards achievements based on
 * various gameplay accomplishments.
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// DELEGATES
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementUnlocked OnAchievementUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAchievementProgress OnAchievementProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTierUnlocked OnTierUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStatChanged OnStatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardsClaimed OnRewardsClaimed;

	// ==========================================
	// ACHIEVEMENT REGISTRATION
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Registration")
	void RegisterAchievement(const FMGAchievementDefinition& Definition);

	UFUNCTION(BlueprintCallable, Category = "Registration")
	void RegisterAchievementsFromDataTable(UDataTable* DataTable);

	UFUNCTION(BlueprintPure, Category = "Registration")
	bool IsAchievementRegistered(FName AchievementID) const;

	UFUNCTION(BlueprintPure, Category = "Registration")
	FMGAchievementDefinition GetAchievementDefinition(FName AchievementID) const;

	UFUNCTION(BlueprintPure, Category = "Registration")
	TArray<FMGAchievementDefinition> GetAllAchievements() const;

	// ==========================================
	// STAT TRACKING
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void IncrementStat(EMGAchievementStatType StatType, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStat(EMGAchievementStatType StatType, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetStatMax(EMGAchievementStatType StatType, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetStat(EMGAchievementStatType StatType) const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void IncrementCustomStat(FName StatID, int32 Amount = 1);

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void SetCustomStat(FName StatID, int32 Value);

	UFUNCTION(BlueprintPure, Category = "Stats")
	int32 GetCustomStat(FName StatID) const;

	UFUNCTION(BlueprintPure, Category = "Stats")
	TMap<EMGAchievementStatType, int32> GetAllStats() const { return Stats; }

	// ==========================================
	// ACHIEVEMENT PROGRESS
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Progress")
	FMGAchievementProgress GetAchievementProgress(FName AchievementID) const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	float GetAchievementProgressPercent(FName AchievementID) const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	bool IsAchievementUnlocked(FName AchievementID) const;

	UFUNCTION(BlueprintPure, Category = "Progress")
	int32 GetCurrentTier(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void ForceUnlockAchievement(FName AchievementID);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void ResetAchievementProgress(FName AchievementID);

	UFUNCTION(BlueprintCallable, Category = "Progress")
	void ResetAllProgress();

	// ==========================================
	// REWARDS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Rewards")
	bool ClaimRewards(FName AchievementID);

	UFUNCTION(BlueprintPure, Category = "Rewards")
	bool AreRewardsClaimed(FName AchievementID) const;

	UFUNCTION(BlueprintPure, Category = "Rewards")
	TArray<FName> GetUnclaimedRewards() const;

	UFUNCTION(BlueprintCallable, Category = "Rewards")
	void ClaimAllRewards();

	// ==========================================
	// QUERIES
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Queries")
	TArray<FMGAchievementDefinition> GetAchievementsByCategory(EMGAchievementCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	TArray<FMGAchievementDefinition> GetUnlockedAchievements() const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	TArray<FMGAchievementDefinition> GetLockedAchievements() const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	TArray<FMGAchievementDefinition> GetInProgressAchievements() const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	int32 GetTotalAchievementCount() const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	int32 GetUnlockedAchievementCount() const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	float GetOverallCompletionPercent() const;

	UFUNCTION(BlueprintPure, Category = "Queries")
	int32 GetTotalPointsEarned() const; // Gamerscore-style points

	// ==========================================
	// RECENT ACHIEVEMENTS
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Recent")
	TArray<FMGAchievementNotification> GetRecentUnlocks(int32 Count = 5) const;

	UFUNCTION(BlueprintCallable, Category = "Recent")
	void ClearRecentUnlocks();

	// ==========================================
	// SAVE/LOAD
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	TArray<FMGAchievementProgress> GetAllProgress() const;

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void LoadProgress(const TArray<FMGAchievementProgress>& ProgressData);

	UFUNCTION(BlueprintCallable, Category = "Persistence")
	void LoadStats(const TMap<EMGAchievementStatType, int32>& StatsData);

protected:
	void CheckAchievementsForStat(EMGAchievementStatType StatType);
	void CheckAchievementForCustomStat(FName StatID);
	void TryUnlockAchievement(FName AchievementID);
	bool CheckPrerequisites(const FMGAchievementDefinition& Definition) const;
	void NotifyAchievementUnlocked(FName AchievementID, int32 Tier = 0);
	int32 GetPointsForRarity(EMGAchievementRarity Rarity) const;

private:
	// Registered achievements
	UPROPERTY()
	TMap<FName, FMGAchievementDefinition> RegisteredAchievements;

	// Progress tracking
	UPROPERTY()
	TMap<FName, FMGAchievementProgress> AchievementProgress;

	// Stats tracking
	UPROPERTY()
	TMap<EMGAchievementStatType, int32> Stats;

	UPROPERTY()
	TMap<FName, int32> CustomStats;

	// Recent unlocks for UI
	UPROPERTY()
	TArray<FMGAchievementNotification> RecentUnlocks;

	static constexpr int32 MaxRecentUnlocks = 20;

	// Stat to achievement mapping for quick lookup
	TMultiMap<EMGAchievementStatType, FName> StatToAchievementMap;
	TMultiMap<FName, FName> CustomStatToAchievementMap;
};

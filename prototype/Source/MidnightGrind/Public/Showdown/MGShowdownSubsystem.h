// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGShowdownSubsystem.generated.h"

/**
 * Showdown type
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
 * Showdown status
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
 * Showdown difficulty
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
 * Boss phase type
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
 * Showdown modifier
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

/**
 * Showdown definition
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
 * Boss phase definition
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
 * Active showdown
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
 * Showdown result
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
 * Boss encounter
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
 * Showdown player stats
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
 * Showdown record
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

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShowdownStarted, const FString&, PlayerId, const FString&, ShowdownId, EMGShowdownType, Type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowdownCompleted, const FString&, PlayerId, const FMGShowdownResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnShowdownFailed, const FString&, PlayerId, const FString&, ShowdownId, int32, PhaseReached);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPhaseStarted, const FString&, PlayerId, const FString&, ShowdownId, int32, PhaseIndex, const FString&, PhaseId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPhaseCompleted, const FString&, PlayerId, const FString&, ShowdownId, int32, PhaseIndex, float, PhaseTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossHealthChanged, const FString&, ShowdownId, int32, CurrentHealth, int32, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointReached, const FString&, PlayerId, const FString&, ShowdownId, int32, CheckpointIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnShowdownUnlocked, const FString&, PlayerId, const FString&, ShowdownId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBossDefeated, const FString&, PlayerId, const FString&, BossId, float, DefeatTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewShowdownRecord, const FString&, PlayerId, const FString&, ShowdownId, float, NewBestTime);

/**
 * Showdown Subsystem
 * Manages boss battles, finale events, and epic showdowns
 */
UCLASS()
class MIDNIGHTGRIND_API UMGShowdownSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
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

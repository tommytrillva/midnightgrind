// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBonusSubsystem.generated.h"

/**
 * Bonus type
 */
UENUM(BlueprintType)
enum class EMGBonusType : uint8
{
	None					UMETA(DisplayName = "None"),
	ScoreMultiplier			UMETA(DisplayName = "Score Multiplier"),
	PointBonus				UMETA(DisplayName = "Point Bonus"),
	TimeBonus				UMETA(DisplayName = "Time Bonus"),
	NitroRefill				UMETA(DisplayName = "Nitro Refill"),
	SpeedBoost				UMETA(DisplayName = "Speed Boost"),
	Invincibility			UMETA(DisplayName = "Invincibility"),
	GhostMode				UMETA(DisplayName = "Ghost Mode"),
	DoublePoints			UMETA(DisplayName = "Double Points"),
	TriplePoints			UMETA(DisplayName = "Triple Points"),
	ComboExtender			UMETA(DisplayName = "Combo Extender"),
	InstantRepair			UMETA(DisplayName = "Instant Repair"),
	CashBonus				UMETA(DisplayName = "Cash Bonus"),
	XPBonus					UMETA(DisplayName = "XP Bonus"),
	RepBonus				UMETA(DisplayName = "Reputation Bonus"),
	LapBonus				UMETA(DisplayName = "Lap Bonus"),
	SecretBonus				UMETA(DisplayName = "Secret Bonus"),
	ChainStarter			UMETA(DisplayName = "Chain Starter")
};

/**
 * Bonus rarity
 */
UENUM(BlueprintType)
enum class EMGBonusRarity : uint8
{
	Common					UMETA(DisplayName = "Common"),
	Uncommon				UMETA(DisplayName = "Uncommon"),
	Rare					UMETA(DisplayName = "Rare"),
	Epic					UMETA(DisplayName = "Epic"),
	Legendary				UMETA(DisplayName = "Legendary"),
	Mythic					UMETA(DisplayName = "Mythic")
};

/**
 * Bonus trigger condition
 */
UENUM(BlueprintType)
enum class EMGBonusTrigger : uint8
{
	Pickup					UMETA(DisplayName = "Pickup"),
	Achievement				UMETA(DisplayName = "Achievement"),
	Milestone				UMETA(DisplayName = "Milestone"),
	Combo					UMETA(DisplayName = "Combo"),
	TimeBased				UMETA(DisplayName = "Time Based"),
	PositionBased			UMETA(DisplayName = "Position Based"),
	EventBased				UMETA(DisplayName = "Event Based"),
	RandomDrop				UMETA(DisplayName = "Random Drop"),
	SecretArea				UMETA(DisplayName = "Secret Area"),
	Challenge				UMETA(DisplayName = "Challenge Complete")
};

/**
 * Bonus round type
 */
UENUM(BlueprintType)
enum class EMGBonusRoundType : uint8
{
	None					UMETA(DisplayName = "None"),
	TimeAttack				UMETA(DisplayName = "Time Attack"),
	PointRush				UMETA(DisplayName = "Point Rush"),
	CoinCollect				UMETA(DisplayName = "Coin Collect"),
	TargetDestroy			UMETA(DisplayName = "Target Destroy"),
	Survival				UMETA(DisplayName = "Survival"),
	DriftChallenge			UMETA(DisplayName = "Drift Challenge"),
	SpeedTrap				UMETA(DisplayName = "Speed Trap"),
	NearMissRun				UMETA(DisplayName = "Near Miss Run"),
	TakedownFrenzy			UMETA(DisplayName = "Takedown Frenzy")
};

/**
 * Bonus definition
 */
USTRUCT(BlueprintType)
struct FMGBonusDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusType Type = EMGBonusType::PointBonus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusRarity Rarity = EMGBonusRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusTrigger Trigger = EMGBonusTrigger::Pickup;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnChance = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> VisualAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DisplayColor = FLinearColor::Yellow;
};

/**
 * Active bonus
 */
USTRUCT(BlueprintType)
struct FMGActiveBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActiveId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusType Type = EMGBonusType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStacks = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPaused = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ActivatedAt;
};

/**
 * Bonus pickup spawn point
 */
USTRUCT(BlueprintType)
struct FMGBonusSpawnPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpawnId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AssignedBonusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PossibleBonusIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCollected = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTimer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CollectionRadius = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFloating = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FloatHeight = 100.0f;
};

/**
 * Bonus round definition
 */
USTRUCT(BlueprintType)
struct FMGBonusRound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoundId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RoundName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusRoundType Type = EMGBonusRoundType::TimeAttack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BronzeThreshold = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SilverThreshold = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GoldThreshold = 20000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PointMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CompletionBonus = 5000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> SpawnPointIds;
};

/**
 * Active bonus round state
 */
USTRUCT(BlueprintType)
struct FMGActiveBonusRound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoundId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBonusRoundType Type = EMGBonusRoundType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetScore = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ItemsCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BestTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bFailed = false;
};

/**
 * Combo bonus event
 */
USTRUCT(BlueprintType)
struct FMGComboBonusEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BonusId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsAwarded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Multiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

/**
 * Player bonus stats
 */
USTRUCT(BlueprintType)
struct FMGBonusPlayerStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBonusType, int32> BonusesCollected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBonusRarity, int32> RaritiesCollected;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBonusesCollected = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPointsFromBonuses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRoundsCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BonusRoundsGold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SecretBonusesFound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestMultiplierDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HighestMultiplierValue = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxComboBonus = 0;
};

/**
 * Bonus config
 */
USTRUCT(BlueprintType)
struct FMGBonusConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GlobalMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RespawnTimeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboThresholdForBonus = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ComboBonusPointsPerLevel = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGBonusRarity, float> RaritySpawnWeights;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableSecretBonuses = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableBonusRounds = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusCollected, const FString&, PlayerId, const FMGBonusDefinition&, Bonus, int32, PointsAwarded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusActivated, const FString&, PlayerId, const FMGActiveBonus&, Bonus, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusExpired, const FString&, PlayerId, const FString&, BonusId, float, TotalValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusStacked, const FString&, PlayerId, const FString&, BonusId, int32, NewStackCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMultiplierChanged, const FString&, PlayerId, float, NewMultiplier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusRoundStart, const FString&, PlayerId, const FMGBonusRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBonusRoundComplete, const FString&, PlayerId, const FString&, RoundId, int32, FinalScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusRoundFailed, const FString&, PlayerId, const FString&, RoundId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnComboBonusTriggered, const FString&, PlayerId, int32, ComboLevel, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSecretBonusFound, const FString&, PlayerId, const FString&, SecretId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusSpawned, const FString&, SpawnId, const FString&, BonusId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBonusRespawned, const FString&, SpawnId, float, NextRespawnTime);

/**
 * Bonus Subsystem
 * Manages bonus pickups, multipliers, bonus rounds, and reward systems
 */
UCLASS()
class MIDNIGHTGRIND_API UMGBonusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusCollected OnBonusCollected;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusActivated OnBonusActivated;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusExpired OnBonusExpired;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusStacked OnBonusStacked;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnMultiplierChanged OnMultiplierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRoundStart OnBonusRoundStart;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRoundComplete OnBonusRoundComplete;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRoundFailed OnBonusRoundFailed;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnComboBonusTriggered OnComboBonusTriggered;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnSecretBonusFound OnSecretBonusFound;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusSpawned OnBonusSpawned;

	UPROPERTY(BlueprintAssignable, Category = "Bonus|Events")
	FOnBonusRespawned OnBonusRespawned;

	// Definition Registration
	UFUNCTION(BlueprintCallable, Category = "Bonus|Definition")
	void RegisterBonusDefinition(const FMGBonusDefinition& Definition);

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	FMGBonusDefinition GetBonusDefinition(const FString& BonusId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	TArray<FMGBonusDefinition> GetAllDefinitions() const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	TArray<FMGBonusDefinition> GetDefinitionsByType(EMGBonusType Type) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Definition")
	TArray<FMGBonusDefinition> GetDefinitionsByRarity(EMGBonusRarity Rarity) const;

	// Spawn Points
	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void RegisterSpawnPoint(const FMGBonusSpawnPoint& SpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void UnregisterSpawnPoint(const FString& SpawnId);

	UFUNCTION(BlueprintPure, Category = "Bonus|Spawn")
	FMGBonusSpawnPoint GetSpawnPoint(const FString& SpawnId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Spawn")
	TArray<FMGBonusSpawnPoint> GetAllSpawnPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void SpawnBonus(const FString& SpawnId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void SpawnAllBonuses();

	UFUNCTION(BlueprintCallable, Category = "Bonus|Spawn")
	void RespawnBonus(const FString& SpawnId);

	// Collection
	UFUNCTION(BlueprintCallable, Category = "Bonus|Collection")
	FMGActiveBonus CollectBonus(const FString& PlayerId, const FString& SpawnId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Collection")
	void GrantBonus(const FString& PlayerId, const FString& BonusId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Collection")
	bool TryCollectAtLocation(const FString& PlayerId, FVector Location);

	UFUNCTION(BlueprintPure, Category = "Bonus|Collection")
	FString GetNearestBonusSpawnId(FVector Location, float MaxDistance) const;

	// Active Bonuses
	UFUNCTION(BlueprintPure, Category = "Bonus|Active")
	TArray<FMGActiveBonus> GetActiveBonuses(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Active")
	FMGActiveBonus GetActiveBonus(const FString& PlayerId, const FString& BonusId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Active")
	bool HasActiveBonus(const FString& PlayerId, EMGBonusType Type) const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void PauseBonus(const FString& PlayerId, const FString& ActiveId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void ResumeBonus(const FString& PlayerId, const FString& ActiveId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void CancelBonus(const FString& PlayerId, const FString& ActiveId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Active")
	void ClearAllBonuses(const FString& PlayerId);

	// Multipliers
	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetTotalMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetScoreMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetXPMultiplier(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Multiplier")
	float GetCashMultiplier(const FString& PlayerId) const;

	// Bonus Rounds
	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void RegisterBonusRound(const FMGBonusRound& Round);

	UFUNCTION(BlueprintPure, Category = "Bonus|Round")
	FMGBonusRound GetBonusRound(const FString& RoundId) const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void StartBonusRound(const FString& PlayerId, const FString& RoundId);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void UpdateBonusRound(const FString& PlayerId, int32 ScoreGained);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Round")
	void EndBonusRound(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Bonus|Round")
	FMGActiveBonusRound GetActiveBonusRound(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Round")
	bool IsInBonusRound(const FString& PlayerId) const;

	// Combo Bonuses
	UFUNCTION(BlueprintCallable, Category = "Bonus|Combo")
	void ProcessComboBonus(const FString& PlayerId, int32 ComboCount, float ComboMultiplier);

	UFUNCTION(BlueprintPure, Category = "Bonus|Combo")
	int32 GetComboBonusPoints(int32 ComboLevel) const;

	// Secret Bonuses
	UFUNCTION(BlueprintCallable, Category = "Bonus|Secret")
	void RegisterSecretBonus(const FString& SecretId, const FString& BonusId, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Bonus|Secret")
	bool TryDiscoverSecret(const FString& PlayerId, FVector Location);

	UFUNCTION(BlueprintPure, Category = "Bonus|Secret")
	bool IsSecretDiscovered(const FString& SecretId) const;

	UFUNCTION(BlueprintPure, Category = "Bonus|Secret")
	TArray<FString> GetDiscoveredSecrets(const FString& PlayerId) const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Bonus|Stats")
	FMGBonusPlayerStats GetPlayerStats(const FString& PlayerId) const;

	UFUNCTION(BlueprintCallable, Category = "Bonus|Stats")
	void ResetPlayerStats(const FString& PlayerId);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Bonus|Config")
	void SetBonusConfig(const FMGBonusConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Bonus|Config")
	FMGBonusConfig GetBonusConfig() const;

	// Update
	UFUNCTION(BlueprintCallable, Category = "Bonus|Update")
	void UpdateBonusSystem(float DeltaTime);

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Bonus|Persistence")
	void SaveBonusData();

	UFUNCTION(BlueprintCallable, Category = "Bonus|Persistence")
	void LoadBonusData();

protected:
	void TickBonus(float DeltaTime);
	void UpdateActiveBonuses(float DeltaTime);
	void UpdateSpawnRespawns(float DeltaTime);
	void UpdateBonusRounds(float DeltaTime);
	void ApplyBonusEffect(const FString& PlayerId, const FMGBonusDefinition& Bonus);
	void RemoveBonusEffect(const FString& PlayerId, const FMGActiveBonus& Bonus);
	void UpdatePlayerStats(const FString& PlayerId, const FMGBonusDefinition& Bonus, int32 Points);
	FString SelectRandomBonus(const TArray<FString>& PossibleIds) const;
	FString GenerateActiveId() const;
	FString GenerateEventId() const;

private:
	UPROPERTY()
	TMap<FString, FMGBonusDefinition> Definitions;

	UPROPERTY()
	TMap<FString, FMGBonusSpawnPoint> SpawnPoints;

	UPROPERTY()
	TMap<FString, TArray<FMGActiveBonus>> PlayerActiveBonuses;

	UPROPERTY()
	TMap<FString, FMGBonusRound> BonusRounds;

	UPROPERTY()
	TMap<FString, FMGActiveBonusRound> ActiveBonusRounds;

	UPROPERTY()
	TMap<FString, FMGBonusPlayerStats> PlayerStats;

	UPROPERTY()
	TMap<FString, FString> SecretBonuses;

	UPROPERTY()
	TMap<FString, FVector> SecretLocations;

	UPROPERTY()
	TArray<FString> DiscoveredSecrets;

	UPROPERTY()
	FMGBonusConfig BonusConfig;

	UPROPERTY()
	int32 ActiveIdCounter = 0;

	UPROPERTY()
	int32 EventCounter = 0;

	FTimerHandle BonusTickTimer;
};

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGHeatLevelSubsystem.generated.h"

/**
 * Heat/wanted level tier
 */
UENUM(BlueprintType)
enum class EMGHeatLevel : uint8
{
	None			UMETA(DisplayName = "No Heat"),
	Level1			UMETA(DisplayName = "Heat Level 1"),
	Level2			UMETA(DisplayName = "Heat Level 2"),
	Level3			UMETA(DisplayName = "Heat Level 3"),
	Level4			UMETA(DisplayName = "Heat Level 4"),
	Level5			UMETA(DisplayName = "Heat Level 5"),
	MaxHeat			UMETA(DisplayName = "Maximum Heat")
};

/**
 * Heat source/infraction type
 */
UENUM(BlueprintType)
enum class EMGHeatSource : uint8
{
	Speeding		UMETA(DisplayName = "Speeding"),
	Reckless		UMETA(DisplayName = "Reckless Driving"),
	WrongWay		UMETA(DisplayName = "Wrong Way"),
	RedLight		UMETA(DisplayName = "Running Red Light"),
	PropertyDamage	UMETA(DisplayName = "Property Damage"),
	VehicleDamage	UMETA(DisplayName = "Vehicle Damage"),
	Collision		UMETA(DisplayName = "Police Collision"),
	Evading			UMETA(DisplayName = "Evading"),
	NearMiss		UMETA(DisplayName = "Police Near Miss"),
	SpeedTrap		UMETA(DisplayName = "Speed Trap"),
	RoadBlock		UMETA(DisplayName = "Roadblock Breach"),
	SpikeStrip		UMETA(DisplayName = "Spike Strip"),
	AirSupport		UMETA(DisplayName = "Helicopter Evade"),
	IllegalRace		UMETA(DisplayName = "Illegal Racing")
};

/**
 * Pursuit state
 */
UENUM(BlueprintType)
enum class EMGPursuitState : uint8
{
	None			UMETA(DisplayName = "No Pursuit"),
	Spotted			UMETA(DisplayName = "Spotted"),
	Pursuit			UMETA(DisplayName = "In Pursuit"),
	Escaping		UMETA(DisplayName = "Escaping"),
	Cooldown		UMETA(DisplayName = "Cooldown"),
	Evaded			UMETA(DisplayName = "Evaded"),
	Busted			UMETA(DisplayName = "Busted")
};

/**
 * Police unit type
 */
UENUM(BlueprintType)
enum class EMGPoliceUnitType : uint8
{
	Patrol			UMETA(DisplayName = "Patrol Car"),
	Interceptor		UMETA(DisplayName = "Interceptor"),
	SUV				UMETA(DisplayName = "SUV"),
	Muscle			UMETA(DisplayName = "Muscle Unit"),
	Supercar		UMETA(DisplayName = "Supercar Unit"),
	Undercover		UMETA(DisplayName = "Undercover"),
	Helicopter		UMETA(DisplayName = "Helicopter"),
	Roadblock		UMETA(DisplayName = "Roadblock Unit"),
	Spike			UMETA(DisplayName = "Spike Strip Unit"),
	SWAT			UMETA(DisplayName = "SWAT"),
	Rhino			UMETA(DisplayName = "Rhino/Heavy")
};

/**
 * Heat infraction event
 */
USTRUCT(BlueprintType)
struct FMGHeatInfraction
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InfractionId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatSource Source = EMGHeatSource::Speeding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatGained = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CostPenalty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWasWitnessed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString WitnessUnitId;
};

/**
 * Heat source configuration
 */
USTRUCT(BlueprintType)
struct FMGHeatSourceConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatSource Source = EMGHeatSource::Speeding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseHeatGain = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseCostPenalty = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresWitness = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bStackable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StackMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStacks = 5;
};

/**
 * Heat level configuration
 */
USTRUCT(BlueprintType)
struct FMGHeatLevelConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatLevel Level = EMGHeatLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeatThreshold = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxUnits = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPoliceUnitType> AvailableUnits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownTime = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRoadblocksEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpikeStripsEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustTimeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HeatColor = FLinearColor::White;
};

/**
 * Active police unit data
 */
USTRUCT(BlueprintType)
struct FMGActivePoliceUnit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnitId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPoliceUnitType UnitType = EMGPoliceUnitType::Patrol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToPlayer = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInPursuit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasVisual = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDisabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AggressionLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TakedownCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeInPursuit = 0.0f;
};

/**
 * Pursuit status data
 */
USTRUCT(BlueprintType)
struct FMGPursuitStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitState State = EMGPursuitState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatLevel CurrentHeatLevel = EMGHeatLevel::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentHeat = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxHeat = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PursuitDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustProgress = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ActiveUnits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitsDisabled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoadblocksEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalInfractions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AccumulatedCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterActive = false;
};

/**
 * Cooldown spot/safe house
 */
USTRUCT(BlueprintType)
struct FMGCooldownSpot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SpotId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText SpotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGHeatLevel MaxEffectiveHeat = EMGHeatLevel::Level3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UseCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUnlocked = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesUsed = 0;
};

/**
 * Heat session statistics
 */
USTRUCT(BlueprintType)
struct FMGHeatSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPursuits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PursuitsEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TimesBusted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestPursuit = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestHeatLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUnitsDisabled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRoadblocksEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalInfractions = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalCostAccumulated = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGHeatSource, int32> InfractionsByType;
};

/**
 * Bounty reward configuration
 */
USTRUCT(BlueprintType)
struct FMGBountyConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseBountyPerSecond = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeatLevelMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitDisabledBonus = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoadblockBonus = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HelicopterEvadeBonus = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EvadeMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedPenaltyPercent = 0.5f;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHeatLevelChanged, EMGHeatLevel, OldLevel, EMGHeatLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitStateChanged, EMGPursuitState, OldState, EMGPursuitState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInfractionCommitted, const FMGHeatInfraction&, Infraction, int32, TotalHeat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitSpawned, const FString&, UnitId, EMGPoliceUnitType, UnitType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPoliceUnitDisabled, const FString&, UnitId, int32, BonusPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitEvaded, float, Duration, int32, BountyEarned);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerBusted, int32, TotalCost, float, PursuitDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCooldownStarted, float, CooldownDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCooldownComplete);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHelicopterDeployed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoadblockSpawned, FVector, Location);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBustProgressUpdate, float, Progress);

/**
 * Heat Level Subsystem
 * Manages police heat/wanted level, pursuits, and evasion mechanics
 */
UCLASS()
class MIDNIGHTGRIND_API UMGHeatLevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnHeatLevelChanged OnHeatLevelChanged;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPursuitStateChanged OnPursuitStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnInfractionCommitted OnInfractionCommitted;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPoliceUnitSpawned OnPoliceUnitSpawned;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPoliceUnitDisabled OnPoliceUnitDisabled;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPursuitEvaded OnPursuitEvaded;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnPlayerBusted OnPlayerBusted;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnCooldownStarted OnCooldownStarted;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnCooldownComplete OnCooldownComplete;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnHelicopterDeployed OnHelicopterDeployed;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnRoadblockSpawned OnRoadblockSpawned;

	UPROPERTY(BlueprintAssignable, Category = "HeatLevel|Events")
	FOnBustProgressUpdate OnBustProgressUpdate;

	// Heat Management
	UFUNCTION(BlueprintCallable, Category = "HeatLevel")
	void AddHeat(EMGHeatSource Source, FVector Location, bool bWasWitnessed = true, const FString& WitnessId = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "HeatLevel")
	void RemoveHeat(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "HeatLevel")
	void ClearAllHeat();

	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	int32 GetCurrentHeat() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	EMGHeatLevel GetCurrentHeatLevel() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	float GetHeatPercent() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel")
	float GetHeatLevelProgress() const;

	// Pursuit State
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	FMGPursuitStatus GetPursuitStatus() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	EMGPursuitState GetPursuitState() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	bool IsInPursuit() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	bool IsEvading() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Pursuit")
	float GetPursuitDuration() const;

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Pursuit")
	void StartPursuit();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Pursuit")
	void StartEscaping();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Pursuit")
	void UpdateBustProgress(float DeltaProgress);

	// Police Units
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void RegisterPoliceUnit(const FMGActivePoliceUnit& Unit);

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void UpdatePoliceUnit(const FString& UnitId, FVector Location, bool bHasVisual, bool bIsInPursuit);

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void DisablePoliceUnit(const FString& UnitId);

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Units")
	void RemovePoliceUnit(const FString& UnitId);

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	TArray<FMGActivePoliceUnit> GetActiveUnits() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	int32 GetActiveUnitCount() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	FMGActivePoliceUnit GetNearestUnit() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Units")
	bool AnyUnitHasVisual() const;

	// Cooldown & Evasion
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Cooldown")
	void StartCooldown();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Cooldown")
	void EnterCooldownSpot(const FString& SpotId);

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Cooldown")
	void ExitCooldownSpot();

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Cooldown")
	float GetCooldownRemaining() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Cooldown")
	float GetCooldownProgress() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Cooldown")
	bool IsInCooldownSpot() const;

	// Cooldown Spots
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Spots")
	void RegisterCooldownSpot(const FMGCooldownSpot& Spot);

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Spots")
	FMGCooldownSpot GetCooldownSpot(const FString& SpotId) const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Spots")
	TArray<FMGCooldownSpot> GetAllCooldownSpots() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Spots")
	FMGCooldownSpot GetNearestCooldownSpot(FVector Location) const;

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Config")
	void SetHeatSourceConfig(EMGHeatSource Source, const FMGHeatSourceConfig& Config);

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Config")
	FMGHeatSourceConfig GetHeatSourceConfig(EMGHeatSource Source) const;

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Config")
	void SetHeatLevelConfig(EMGHeatLevel Level, const FMGHeatLevelConfig& Config);

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Config")
	FMGHeatLevelConfig GetHeatLevelConfig(EMGHeatLevel Level) const;

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Config")
	void SetBountyConfig(const FMGBountyConfig& Config);

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Config")
	FMGBountyConfig GetBountyConfig() const;

	// Bounty
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bounty")
	int32 GetCurrentBounty() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bounty")
	int32 CalculateEvadeBounty() const;

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Bounty")
	void NotifyRoadblockEvaded();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Bounty")
	void NotifyHelicopterEvaded();

	// Special Events
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Special")
	void DeployHelicopter();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Special")
	void SpawnRoadblock(FVector Location);

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Special")
	void SpawnSpikeStrip(FVector Location);

	// Bust
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Bust")
	void TriggerBust();

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bust")
	float GetBustProgress() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Bust")
	int32 GetBustCost() const;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Session")
	void StartSession();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Session")
	void EndSession();

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Session")
	bool IsSessionActive() const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Session")
	FMGHeatSessionStats GetSessionStats() const;

	// Utility
	UFUNCTION(BlueprintPure, Category = "HeatLevel|Utility")
	FText GetHeatLevelDisplayName(EMGHeatLevel Level) const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Utility")
	FLinearColor GetHeatLevelColor(EMGHeatLevel Level) const;

	UFUNCTION(BlueprintPure, Category = "HeatLevel|Utility")
	int32 GetHeatLevelStars(EMGHeatLevel Level) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Persistence")
	void SaveHeatData();

	UFUNCTION(BlueprintCallable, Category = "HeatLevel|Persistence")
	void LoadHeatData();

protected:
	void UpdateHeatLevel();
	void TickPursuit(float DeltaTime);
	void TickCooldown(float DeltaTime);
	void TickBounty(float DeltaTime);
	void CompleteCooldown();
	void InitializeDefaultConfigs();
	int32 CalculateInfractionHeat(EMGHeatSource Source) const;

private:
	UPROPERTY()
	FMGPursuitStatus PursuitStatus;

	UPROPERTY()
	FMGHeatSessionStats SessionStats;

	UPROPERTY()
	FMGBountyConfig BountyConfig;

	UPROPERTY()
	TMap<EMGHeatSource, FMGHeatSourceConfig> HeatSourceConfigs;

	UPROPERTY()
	TMap<EMGHeatLevel, FMGHeatLevelConfig> HeatLevelConfigs;

	UPROPERTY()
	TMap<FString, FMGActivePoliceUnit> ActiveUnits;

	UPROPERTY()
	TMap<FString, FMGCooldownSpot> CooldownSpots;

	UPROPERTY()
	TMap<EMGHeatSource, int32> InfractionStacks;

	UPROPERTY()
	TMap<EMGHeatSource, float> InfractionCooldowns;

	UPROPERTY()
	int32 CurrentBounty = 0;

	UPROPERTY()
	float CooldownTotal = 0.0f;

	UPROPERTY()
	FString CurrentCooldownSpotId;

	UPROPERTY()
	bool bSessionActive = false;

	FTimerHandle PursuitTickTimer;
};

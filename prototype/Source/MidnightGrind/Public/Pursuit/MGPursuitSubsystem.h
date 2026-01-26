// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGPursuitSubsystem.generated.h"

/**
 * Pursuit role
 */
UENUM(BlueprintType)
enum class EMGPursuitRole : uint8
{
	None				UMETA(DisplayName = "None"),
	Runner				UMETA(DisplayName = "Runner/Escapee"),
	Pursuer				UMETA(DisplayName = "Pursuer/Cop"),
	Interceptor			UMETA(DisplayName = "Interceptor"),
	Helicopter			UMETA(DisplayName = "Helicopter"),
	RoadBlock			UMETA(DisplayName = "Road Block")
};

/**
 * Pursuit state
 */
UENUM(BlueprintType)
enum class EMGPursuitState : uint8
{
	Inactive			UMETA(DisplayName = "Inactive"),
	Searching			UMETA(DisplayName = "Searching"),
	Sighted				UMETA(DisplayName = "Sighted"),
	Engaged				UMETA(DisplayName = "Engaged"),
	PursuitActive		UMETA(DisplayName = "Pursuit Active"),
	Busted				UMETA(DisplayName = "Busted"),
	Escaped				UMETA(DisplayName = "Escaped"),
	Cooldown			UMETA(DisplayName = "Cooldown")
};

/**
 * Pursuit tactic
 */
UENUM(BlueprintType)
enum class EMGPursuitTactic : uint8
{
	Follow				UMETA(DisplayName = "Follow"),
	Ram					UMETA(DisplayName = "Ram"),
	PitManeuver			UMETA(DisplayName = "PIT Maneuver"),
	BoxIn				UMETA(DisplayName = "Box In"),
	Roadblock			UMETA(DisplayName = "Roadblock"),
	SpikeStrip			UMETA(DisplayName = "Spike Strip"),
	Helicopter			UMETA(DisplayName = "Helicopter"),
	EMPDisable			UMETA(DisplayName = "EMP Disable"),
	TireShot			UMETA(DisplayName = "Tire Shot")
};

/**
 * Pursuit intensity level
 */
UENUM(BlueprintType)
enum class EMGPursuitIntensity : uint8
{
	Low					UMETA(DisplayName = "Low"),
	Medium				UMETA(DisplayName = "Medium"),
	High				UMETA(DisplayName = "High"),
	Extreme				UMETA(DisplayName = "Extreme"),
	Maximum				UMETA(DisplayName = "Maximum")
};

/**
 * Pursuit event type
 */
UENUM(BlueprintType)
enum class EMGPursuitEventType : uint8
{
	Spotted				UMETA(DisplayName = "Spotted"),
	BackupCalled		UMETA(DisplayName = "Backup Called"),
	RoadblockDeployed	UMETA(DisplayName = "Roadblock Deployed"),
	SpikeStripDeployed	UMETA(DisplayName = "Spike Strip Deployed"),
	HelicopterCalled	UMETA(DisplayName = "Helicopter Called"),
	PursuerTakenDown	UMETA(DisplayName = "Pursuer Taken Down"),
	NearEscape			UMETA(DisplayName = "Near Escape"),
	CooldownStarted		UMETA(DisplayName = "Cooldown Started"),
	IntensityIncreased	UMETA(DisplayName = "Intensity Increased")
};

/**
 * Pursuit unit
 */
USTRUCT(BlueprintType)
struct FMGPursuitUnit
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnitId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitRole Role = EMGPursuitRole::Pursuer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Velocity = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TargetPlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitTactic CurrentTactic = EMGPursuitTactic::Follow;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceToTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDisabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasVisual = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeEngaged = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DamageDealt = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UObject> UnitAsset;
};

/**
 * Pursuit status
 */
USTRUCT(BlueprintType)
struct FMGPursuitStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitState State = EMGPursuitState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitIntensity Intensity = EMGPursuitIntensity::Low;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPursuitUnit> ActiveUnits;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUnitsEngaged = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitsDisabled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PursuitDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeMeter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedMeter = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHelicopterActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoadblocksEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpikeStripsEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InfractionMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentBounty = 0;
};

/**
 * Roadblock configuration
 */
USTRUCT(BlueprintType)
struct FMGRoadblock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString RoadblockId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumUnits = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasSpikeStrip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpikeStripOffset = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUntilActive = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasBeenEvaded = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclesDamaged = 0;
};

/**
 * Pursuit event record
 */
USTRUCT(BlueprintType)
struct FMGPursuitEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitEventType Type = EMGPursuitEventType::Spotted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BountyChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString InvolvedUnitId;
};

/**
 * Pursuit scoring
 */
USTRUCT(BlueprintType)
struct FMGPursuitScoring
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseEscapeBonus = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerUnitDisabledBonus = 500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerRoadblockEvadedBonus = 750;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PerSpikeStripEvadedBonus = 300;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationMultiplierPerMinute = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float IntensityMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CleanEscapeBonus = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NearMissBonus = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HelicopterEvadeBonus = 1500;
};

/**
 * Pursuit config
 */
USTRUCT(BlueprintType)
struct FMGPursuitConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseEscapeTime = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedThreshold = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BustedFillRate = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeFillRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EscapeDrainRate = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistanceForEscape = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownDuration = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VisualRange = 15000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPursuitIntensity, int32> MaxUnitsPerIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EMGPursuitIntensity, float> IntensityUpgradeThresholds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPursuitTactic> AvailableTactics;
};

/**
 * Pursuit session stats
 */
USTRUCT(BlueprintType)
struct FMGPursuitSessionStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPursuitsStarted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalEscapes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBusted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalUnitsDisabled = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRoadblocksEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalSpikeStripsEvaded = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LongestPursuitDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HighestBounty = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyEarned = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBountyLost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPursuitIntensity HighestIntensity = EMGPursuitIntensity::Low;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MostUnitsEngagedAtOnce = 0;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitStarted, const FString&, PlayerId, EMGPursuitIntensity, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPursuitEnded, const FString&, PlayerId, bool, bEscaped, int32, FinalBounty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnPursuitIntensityChanged, const FString&, PlayerId, EMGPursuitIntensity, OldIntensity, EMGPursuitIntensity, NewIntensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitStateChanged, const FString&, PlayerId, EMGPursuitState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitEngaged, const FString&, PlayerId, const FMGPursuitUnit&, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUnitDisabled, const FString&, PlayerId, const FMGPursuitUnit&, Unit);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadblockDeployed, const FString&, PlayerId, const FMGRoadblock&, Roadblock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoadblockEvaded, const FString&, PlayerId, const FString&, RoadblockId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHelicopterCalled, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHelicopterEvaded, const FString&, PlayerId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBountyChanged, const FString&, PlayerId, int32, NewBounty);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCooldownStarted, const FString&, PlayerId, float, CooldownTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEscapeMeterChanged, const FString&, PlayerId, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBustedMeterChanged, const FString&, PlayerId, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPursuitEvent, const FString&, PlayerId, const FMGPursuitEvent&, Event);

/**
 * Pursuit Subsystem
 * Manages police pursuits, escape mechanics, and wanted levels
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPursuitSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitStarted OnPursuitStarted;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitEnded OnPursuitEnded;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitIntensityChanged OnPursuitIntensityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitStateChanged OnPursuitStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnUnitEngaged OnUnitEngaged;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnUnitDisabled OnUnitDisabled;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnRoadblockDeployed OnRoadblockDeployed;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnRoadblockEvaded OnRoadblockEvaded;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnHelicopterCalled OnHelicopterCalled;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnHelicopterEvaded OnHelicopterEvaded;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnBountyChanged OnBountyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnCooldownStarted OnCooldownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnEscapeMeterChanged OnEscapeMeterChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnBustedMeterChanged OnBustedMeterChanged;

	UPROPERTY(BlueprintAssignable, Category = "Pursuit|Events")
	FOnPursuitEvent OnPursuitEvent;

	// Pursuit Control
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void StartPursuit(const FString& PlayerId, EMGPursuitIntensity InitialIntensity = EMGPursuitIntensity::Low);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void EndPursuit(const FString& PlayerId, bool bEscaped);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void SetPursuitIntensity(const FString& PlayerId, EMGPursuitIntensity Intensity);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Control")
	void IncreaseIntensity(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Pursuit|Control")
	bool IsPursuitActive(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Control")
	bool IsInCooldown(const FString& PlayerId) const;

	// Status
	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	FMGPursuitStatus GetPursuitStatus(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	EMGPursuitState GetPursuitState(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	EMGPursuitIntensity GetPursuitIntensity(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	float GetEscapeMeter(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	float GetBustedMeter(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	int32 GetBounty(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Status")
	float GetCooldownRemaining(const FString& PlayerId) const;

	// Units
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Units")
	void SpawnPursuitUnit(const FString& PlayerId, EMGPursuitRole Role, FVector SpawnLocation);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Units")
	void RemovePursuitUnit(const FString& PlayerId, const FString& UnitId);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Units")
	void DisableUnit(const FString& PlayerId, const FString& UnitId, float Damage);

	UFUNCTION(BlueprintPure, Category = "Pursuit|Units")
	TArray<FMGPursuitUnit> GetActiveUnits(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Units")
	int32 GetActiveUnitCount(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Units")
	FMGPursuitUnit GetClosestUnit(const FString& PlayerId, FVector Location) const;

	// Tactics
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void DeployRoadblock(const FString& PlayerId, const FMGRoadblock& Roadblock);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void EvadeRoadblock(const FString& PlayerId, const FString& RoadblockId);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void HitRoadblock(const FString& PlayerId, const FString& RoadblockId, float Damage);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void CallHelicopter(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void EvadeHelicopter(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Tactics")
	void HitSpikeStrip(const FString& PlayerId);

	UFUNCTION(BlueprintPure, Category = "Pursuit|Tactics")
	TArray<FMGRoadblock> GetActiveRoadblocks(const FString& PlayerId) const;

	// Bounty
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Bounty")
	void AddBounty(const FString& PlayerId, int32 Amount, const FString& Reason = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Bounty")
	void ResetBounty(const FString& PlayerId);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Bounty")
	int32 CollectBounty(const FString& PlayerId);

	// Update
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Update")
	void UpdatePursuit(const FString& PlayerId, FVector PlayerLocation, FVector PlayerVelocity, float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Update")
	void UpdateUnitAI(const FString& PlayerId, float DeltaTime);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Config")
	void SetPursuitConfig(const FMGPursuitConfig& Config);

	UFUNCTION(BlueprintPure, Category = "Pursuit|Config")
	FMGPursuitConfig GetPursuitConfig() const;

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Config")
	void SetPursuitScoring(const FMGPursuitScoring& Scoring);

	UFUNCTION(BlueprintPure, Category = "Pursuit|Config")
	FMGPursuitScoring GetPursuitScoring() const;

	// Stats
	UFUNCTION(BlueprintPure, Category = "Pursuit|Stats")
	FMGPursuitSessionStats GetSessionStats() const;

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Stats")
	void ResetSessionStats();

	// Scoring
	UFUNCTION(BlueprintPure, Category = "Pursuit|Scoring")
	int32 CalculateEscapeScore(const FString& PlayerId) const;

	UFUNCTION(BlueprintPure, Category = "Pursuit|Scoring")
	int32 CalculateBustedPenalty(const FString& PlayerId) const;

	// Events
	UFUNCTION(BlueprintPure, Category = "Pursuit|Events")
	TArray<FMGPursuitEvent> GetPursuitEvents(const FString& PlayerId) const;

	// Save/Load
	UFUNCTION(BlueprintCallable, Category = "Pursuit|Persistence")
	void SavePursuitData();

	UFUNCTION(BlueprintCallable, Category = "Pursuit|Persistence")
	void LoadPursuitData();

protected:
	void TickPursuit(float DeltaTime);
	void UpdateEscapeProgress(const FString& PlayerId, FVector PlayerLocation, float DeltaTime);
	void UpdateBustedProgress(const FString& PlayerId, float DeltaTime);
	void CheckIntensityUpgrade(const FString& PlayerId);
	void SpawnBackup(const FString& PlayerId);
	void SetPursuitState(const FString& PlayerId, EMGPursuitState NewState);
	void RecordEvent(const FString& PlayerId, EMGPursuitEventType Type, const FString& Description, FVector Location);
	FString GenerateUnitId() const;

private:
	UPROPERTY()
	TMap<FString, FMGPursuitStatus> ActivePursuits;

	UPROPERTY()
	TMap<FString, TArray<FMGRoadblock>> ActiveRoadblocks;

	UPROPERTY()
	TMap<FString, TArray<FMGPursuitEvent>> PursuitEvents;

	UPROPERTY()
	FMGPursuitConfig PursuitConfig;

	UPROPERTY()
	FMGPursuitScoring PursuitScoring;

	UPROPERTY()
	FMGPursuitSessionStats SessionStats;

	UPROPERTY()
	int32 UnitCounter = 0;

	FTimerHandle PursuitTickTimer;
};

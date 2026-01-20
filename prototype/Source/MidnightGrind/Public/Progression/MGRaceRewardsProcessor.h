// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Progression/MGPlayerProgression.h"
#include "MGRaceRewardsProcessor.generated.h"

class AMGRaceGameMode;
struct FMGFinalRaceResult;

/**
 * XP reward breakdown
 */
USTRUCT(BlueprintType)
struct FMGXPBreakdown
{
	GENERATED_BODY()

	/** Base XP for finishing */
	UPROPERTY(BlueprintReadOnly)
	int64 BaseXP = 0;

	/** Bonus for position */
	UPROPERTY(BlueprintReadOnly)
	int64 PositionXP = 0;

	/** Bonus for best lap */
	UPROPERTY(BlueprintReadOnly)
	int64 BestLapXP = 0;

	/** Bonus for clean race (no collisions) */
	UPROPERTY(BlueprintReadOnly)
	int64 CleanRaceXP = 0;

	/** Bonus for overtakes */
	UPROPERTY(BlueprintReadOnly)
	int64 OvertakeXP = 0;

	/** Bonus for drift score */
	UPROPERTY(BlueprintReadOnly)
	int64 DriftXP = 0;

	/** Bonus for near misses */
	UPROPERTY(BlueprintReadOnly)
	int64 NearMissXP = 0;

	/** Ranked match multiplier bonus */
	UPROPERTY(BlueprintReadOnly)
	int64 RankedBonusXP = 0;

	/** Total XP earned */
	UPROPERTY(BlueprintReadOnly)
	int64 TotalXP = 0;

	void CalculateTotal()
	{
		TotalXP = BaseXP + PositionXP + BestLapXP + CleanRaceXP + OvertakeXP + DriftXP + NearMissXP + RankedBonusXP;
	}
};

/**
 * Complete race rewards data
 */
USTRUCT(BlueprintType)
struct FMGRaceRewards
{
	GENERATED_BODY()

	/** Credits earned */
	UPROPERTY(BlueprintReadOnly)
	int64 CreditsEarned = 0;

	/** XP breakdown */
	UPROPERTY(BlueprintReadOnly)
	FMGXPBreakdown XPBreakdown;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly)
	int32 ReputationEarned = 0;

	/** Crew that receives reputation */
	UPROPERTY(BlueprintReadOnly)
	EMGCrew ReputationCrew = EMGCrew::None;

	/** Did player level up */
	UPROPERTY(BlueprintReadOnly)
	bool bLeveledUp = false;

	/** New level (if leveled up) */
	UPROPERTY(BlueprintReadOnly)
	int32 NewLevel = 0;

	/** Did reputation tier change */
	UPROPERTY(BlueprintReadOnly)
	bool bReputationTierChanged = false;

	/** New reputation tier */
	UPROPERTY(BlueprintReadOnly)
	EMGReputationTier NewReputationTier = EMGReputationTier::Unknown;

	/** New unlocks granted */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGUnlock> NewUnlocks;

	/** Is this a new track record */
	UPROPERTY(BlueprintReadOnly)
	bool bNewTrackRecord = false;

	/** Is this a new personal best */
	UPROPERTY(BlueprintReadOnly)
	bool bNewPersonalBest = false;
};

/**
 * Race session performance data (collected during race)
 */
USTRUCT(BlueprintType)
struct FMGRacePerformanceData
{
	GENERATED_BODY()

	/** Number of overtakes made */
	UPROPERTY(BlueprintReadWrite)
	int32 Overtakes = 0;

	/** Number of times overtaken */
	UPROPERTY(BlueprintReadWrite)
	int32 TimesOvertaken = 0;

	/** Total drift score */
	UPROPERTY(BlueprintReadWrite)
	float TotalDriftScore = 0.0f;

	/** Number of near misses */
	UPROPERTY(BlueprintReadWrite)
	int32 NearMisses = 0;

	/** Number of collisions */
	UPROPERTY(BlueprintReadWrite)
	int32 Collisions = 0;

	/** Maximum speed achieved (MPH) */
	UPROPERTY(BlueprintReadWrite)
	float MaxSpeedMPH = 0.0f;

	/** Total distance (km) */
	UPROPERTY(BlueprintReadWrite)
	float DistanceKm = 0.0f;

	/** Starting position */
	UPROPERTY(BlueprintReadWrite)
	int32 StartingPosition = 0;

	/** Final position */
	UPROPERTY(BlueprintReadWrite)
	int32 FinalPosition = 0;

	/** Did finish race */
	UPROPERTY(BlueprintReadWrite)
	bool bFinished = false;

	/** Was clean race (no collisions) */
	bool IsCleanRace() const { return Collisions == 0 && bFinished; }

	/** Number of positions gained */
	int32 GetPositionsGained() const { return StartingPosition - FinalPosition; }
};

/**
 * Delegates for UI binding
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceRewardsCalculated, const FMGRaceRewards&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceRewardsGranted, const FMGRaceRewards&, Rewards);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewUnlockFromRace, const FMGUnlock&, Unlock, int32, Index);

/**
 * World subsystem that processes race end rewards
 * Connects the race game mode to the progression system
 *
 * Features:
 * - Calculates XP with detailed breakdown
 * - Grants credits and reputation
 * - Records race statistics
 * - Checks for new unlocks
 * - Handles level up notifications
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceRewardsProcessor : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// ==========================================
	// PERFORMANCE TRACKING
	// ==========================================

	/** Begin tracking performance for a race */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void BeginRaceTracking(int32 StartingPosition);

	/** Record an overtake */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void RecordOvertake();

	/** Record being overtaken */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void RecordOvertaken();

	/** Record drift score */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void RecordDriftScore(float Score);

	/** Record near miss */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void RecordNearMiss();

	/** Record collision */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void RecordCollision();

	/** Record max speed */
	UFUNCTION(BlueprintCallable, Category = "Rewards|Tracking")
	void RecordMaxSpeed(float SpeedMPH);

	/** Get current performance data */
	UFUNCTION(BlueprintPure, Category = "Rewards|Tracking")
	FMGRacePerformanceData GetCurrentPerformanceData() const { return CurrentPerformance; }

	// ==========================================
	// REWARD PROCESSING
	// ==========================================

	/** Process race results and calculate rewards (does not grant yet) */
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	FMGRaceRewards CalculateRewards(const FMGFinalRaceResult& Result, EMGCrew RaceCrew, bool bIsRanked);

	/** Grant calculated rewards to player progression */
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	void GrantRewards(const FMGRaceRewards& Rewards);

	/** Process complete race end (calculate and grant) */
	UFUNCTION(BlueprintCallable, Category = "Rewards")
	FMGRaceRewards ProcessRaceEnd(const FMGFinalRaceResult& Result, EMGCrew RaceCrew, bool bIsRanked);

	/** Get last calculated rewards */
	UFUNCTION(BlueprintPure, Category = "Rewards")
	FMGRaceRewards GetLastRewards() const { return LastRewards; }

	// ==========================================
	// CONFIGURATION
	// ==========================================

	/** XP multiplier for ranked races */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	float RankedXPMultiplier = 1.5f;

	/** Base XP for finishing a race */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 BaseFinishXP = 100;

	/** XP per position (1st gets this * (TotalRacers - Position + 1)) */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 XPPerPosition = 50;

	/** XP for achieving best lap */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 BestLapXP = 150;

	/** XP for clean race (no collisions) */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 CleanRaceXP = 200;

	/** XP per overtake */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 XPPerOvertake = 25;

	/** XP per 1000 drift score points */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 XPPerThousandDrift = 10;

	/** XP per near miss */
	UPROPERTY(EditDefaultsOnly, Category = "Config|XP")
	int64 XPPerNearMiss = 5;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when rewards are calculated (before granting) */
	UPROPERTY(BlueprintAssignable, Category = "Rewards|Events")
	FOnRaceRewardsCalculated OnRaceRewardsCalculated;

	/** Called when rewards are granted */
	UPROPERTY(BlueprintAssignable, Category = "Rewards|Events")
	FOnRaceRewardsGranted OnRaceRewardsGranted;

	/** Called for each new unlock (for sequential reveal animation) */
	UPROPERTY(BlueprintAssignable, Category = "Rewards|Events")
	FOnNewUnlockFromRace OnNewUnlockFromRace;

protected:
	/** Bind to race game mode events */
	void BindToRaceGameMode();

	/** Handle race results ready event */
	UFUNCTION()
	void OnRaceResultsReady(const TArray<FMGFinalRaceResult>& Results);

	/** Calculate XP breakdown */
	FMGXPBreakdown CalculateXPBreakdown(const FMGFinalRaceResult& Result, bool bIsRanked, int32 TotalRacers);

	/** Check if this is a new personal best */
	bool CheckPersonalBest(FName TrackID, float Time) const;

	/** Check if this is a new track record */
	bool CheckTrackRecord(FName TrackID, float Time) const;

private:
	/** Reference to player progression */
	UPROPERTY()
	TWeakObjectPtr<UMGPlayerProgression> PlayerProgression;

	/** Reference to race game mode */
	UPROPERTY()
	TWeakObjectPtr<AMGRaceGameMode> RaceGameMode;

	/** Current race performance tracking */
	FMGRacePerformanceData CurrentPerformance;

	/** Last calculated rewards */
	FMGRaceRewards LastRewards;

	/** Is currently tracking a race */
	bool bIsTracking = false;

	/** Track ID for current race */
	FName CurrentTrackID;

	/** Crew for current race */
	EMGCrew CurrentRaceCrew = EMGCrew::None;

	/** Is current race ranked */
	bool bCurrentRaceRanked = false;

	/** Level before rewards were granted */
	int32 PreRewardLevel = 0;

	/** Reputation tier before rewards were granted */
	EMGReputationTier PreRewardTier = EMGReputationTier::Unknown;
};

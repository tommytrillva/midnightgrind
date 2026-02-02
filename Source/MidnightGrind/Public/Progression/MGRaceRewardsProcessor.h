// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGRaceRewardsProcessor.h
 * @brief Race completion rewards calculation and distribution system
 *
 * This file defines the Race Rewards Processor, a world subsystem that handles
 * all reward calculations and distribution when a race ends. It serves as the
 * bridge between the racing game mode and the player progression system.
 *
 * @section Overview
 *
 * When a race finishes, multiple rewards need to be calculated and granted:
 * - Experience Points (XP) with detailed breakdown
 * - In-game currency (Credits)
 * - Crew reputation
 * - Content unlocks (vehicles, parts, tracks)
 * - Record tracking (personal bests, track records)
 *
 * The Race Rewards Processor handles all of this in a structured, event-driven way.
 *
 * @section KeyConcepts Key Concepts for Beginners
 *
 * **1. World Subsystem**
 *
 * This class inherits from UWorldSubsystem, meaning:
 * - One instance exists per game world
 * - It's automatically created when a level loads
 * - It's destroyed when the level is unloaded
 * - Access it via: GetWorld()->GetSubsystem<UMGRaceRewardsProcessor>()
 *
 * **2. Performance Tracking**
 *
 * Throughout a race, the processor tracks player performance metrics:
 * - Overtakes made and received
 * - Drift scores accumulated
 * - Near misses with traffic/opponents
 * - Collisions (affects "clean race" bonus)
 * - Maximum speed achieved
 *
 * These metrics feed into the XP calculation.
 *
 * **3. XP Breakdown (FMGXPBreakdown)**
 *
 * XP isn't just a single number - it's broken down into categories:
 * - BaseXP: Everyone gets this for finishing
 * - PositionXP: Bonus for placement (1st gets most)
 * - BestLapXP: Bonus for setting the fastest lap
 * - CleanRaceXP: Bonus for no collisions
 * - OvertakeXP: Bonus per overtake
 * - DriftXP: Bonus based on drift score
 * - NearMissXP: Bonus for close passes
 * - RankedBonusXP: Multiplier for ranked races
 *
 * This breakdown is shown to players post-race for satisfaction and transparency.
 *
 * **4. Two-Phase Rewards**
 *
 * Rewards are processed in two phases:
 * - **Calculate**: Compute all rewards without applying them (for UI preview)
 * - **Grant**: Actually apply the rewards to player progression
 *
 * This allows the UI to show a rewards screen before committing changes.
 *
 * **5. Event-Driven Architecture**
 *
 * The processor broadcasts delegates at key moments:
 * - OnRaceRewardsCalculated: After calculation, before granting
 * - OnRaceRewardsGranted: After rewards are applied
 * - OnNewUnlockFromRace: For each new unlock (sequential animations)
 *
 * @section Usage Usage Example
 *
 * @code
 * // Get the processor
 * UMGRaceRewardsProcessor* RewardsProcessor =
 *     GetWorld()->GetSubsystem<UMGRaceRewardsProcessor>();
 *
 * // Start tracking at race begin
 * RewardsProcessor->BeginRaceTracking(StartingGridPosition);
 *
 * // Record events during the race
 * void AMyRacerPawn::OnOvertake()
 * {
 *     RewardsProcessor->RecordOvertake();
 * }
 *
 * void AMyRacerPawn::OnDriftScoreAwarded(float Score)
 * {
 *     RewardsProcessor->RecordDriftScore(Score);
 * }
 *
 * // At race end, process rewards
 * void ARaceGameMode::OnRaceFinished(const FMGFinalRaceResult& Result)
 * {
 *     // Option 1: Calculate and grant in one step
 *     FMGRaceRewards Rewards = RewardsProcessor->ProcessRaceEnd(
 *         Result, EMGCrew::Midnight, bIsRankedRace);
 *
 *     // Option 2: Calculate first, then grant (for UI preview)
 *     FMGRaceRewards Rewards = RewardsProcessor->CalculateRewards(
 *         Result, EMGCrew::Midnight, bIsRankedRace);
 *
 *     // Show rewards screen...
 *
 *     // Then grant when player dismisses screen
 *     RewardsProcessor->GrantRewards(Rewards);
 * }
 *
 * // Bind to unlock events for sequential reveal animation
 * RewardsProcessor->OnNewUnlockFromRace.AddDynamic(
 *     this, &URewardsUI::ShowUnlockAnimation);
 * @endcode
 *
 * @section DataFlow Data Flow
 *
 * @code
 * [Race Gameplay]
 *     |
 *     v
 * [Performance Tracking] --> RecordOvertake(), RecordDriftScore(), etc.
 *     |
 *     v
 * [Race End Event] --> FMGFinalRaceResult
 *     |
 *     v
 * [CalculateRewards()] --> FMGRaceRewards with FMGXPBreakdown
 *     |
 *     +-- BaseXP calculation
 *     +-- Position bonus
 *     +-- Best lap check
 *     +-- Clean race check
 *     +-- Skill bonuses (drift, near miss, overtake)
 *     +-- Ranked multiplier
 *     |
 *     v
 * [GrantRewards()] --> UMGPlayerProgression
 *     |
 *     +-- AddXP()
 *     +-- AddCredits()
 *     +-- AddCrewReputation()
 *     +-- CheckAndGrantNewUnlocks()
 *     |
 *     v
 * [Broadcast Events]
 *     +-- OnRaceRewardsGranted
 *     +-- OnNewUnlockFromRace (for each unlock)
 * @endcode
 *
 * @section Configuration Configuration
 *
 * XP values are configurable via class properties:
 * - BaseFinishXP: XP for just finishing (default: 100)
 * - XPPerPosition: XP multiplied by position difference (default: 50)
 * - BestLapXP: Bonus for fastest lap (default: 150)
 * - CleanRaceXP: Bonus for no collisions (default: 200)
 * - XPPerOvertake: Per overtake bonus (default: 25)
 * - RankedXPMultiplier: Multiplier for ranked races (default: 1.5x)
 *
 * @see UMGPlayerProgression for the progression system that receives rewards
 * @see AMGRaceGameMode for race result generation
 * @see FMGFinalRaceResult for race result data structure
 */

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Progression/MGPlayerProgression.h"
#include "MGRaceRewardsProcessor.generated.h"

class AMGRaceGameMode;

/**
 * @brief Final race result data for a single participant
 *
 * Contains all performance data needed to calculate rewards and update leaderboards.
 * Generated by the race game mode when a player finishes a race.
 */
USTRUCT(BlueprintType)
struct FMGFinalRaceResult
{
	GENERATED_BODY()

	/** Player ID */
	UPROPERTY(BlueprintReadOnly, Category = "Identity")
	FString PlayerID;

	/** Player display name */
	UPROPERTY(BlueprintReadOnly, Category = "Identity")
	FString PlayerName;

	/** Vehicle used */
	UPROPERTY(BlueprintReadOnly, Category = "Identity")
	FGuid VehicleID;

	/** Finishing position (1 = first) */
	UPROPERTY(BlueprintReadOnly, Category = "Position")
	int32 FinishPosition = 0;

	/** Total racers in the event */
	UPROPERTY(BlueprintReadOnly, Category = "Position")
	int32 TotalRacers = 0;

	/** Whether player finished the race (vs DNF) */
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bFinished = false;

	/** Whether player was disqualified */
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	bool bDisqualified = false;

	/** Total race time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	float TotalTime = 0.0f;

	/** Best lap time in seconds */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	float BestLapTime = 0.0f;

	/** Whether this was the race's fastest lap */
	UPROPERTY(BlueprintReadOnly, Category = "Time")
	bool bHadFastestLap = false;

	/** Number of laps completed */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 LapsCompleted = 0;

	/** Total laps in race */
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 TotalLaps = 0;

	/** Number of overtakes made */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 OvertakeCount = 0;

	/** Whether race was completed cleanly (no collisions) */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	bool bCleanRace = false;

	/** Total collision count */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 CollisionCount = 0;

	/** Accumulated drift score */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int64 DriftScore = 0;

	/** Near miss count */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 NearMissCount = 0;

	/** Average speed in MPH */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float AverageSpeed = 0.0f;

	/** Top speed reached in MPH */
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float TopSpeed = 0.0f;

	/** Track ID raced on */
	UPROPERTY(BlueprintReadOnly, Category = "Track")
	FName TrackID;

	/** Race type */
	UPROPERTY(BlueprintReadOnly, Category = "Race")
	EMGRaceType RaceType = EMGRaceType::Circuit;

	/** Timestamp when finished */
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FDateTime FinishTimestamp;
};

/**
 * @brief Detailed breakdown of XP earned from a race
 *
 * Each component is calculated separately and displayed in the post-race
 * rewards screen for player satisfaction and transparency. The breakdown
 * helps players understand what actions contribute to their progression.
 */
USTRUCT(BlueprintType)
struct FMGXPBreakdown
{
	GENERATED_BODY()

	/// Base XP awarded just for finishing the race
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 BaseXP = 0;

	/// Bonus XP based on finishing position (1st gets most)
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 PositionXP = 0;

	/// Bonus XP for achieving the fastest lap in the race
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 BestLapXP = 0;

	/// Bonus XP for completing the race without collisions
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 CleanRaceXP = 0;

	/// Bonus XP for each overtake performed during the race
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 OvertakeXP = 0;

	/// Bonus XP based on accumulated drift score
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 DriftXP = 0;

	/// Bonus XP for near-miss close passes
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 NearMissXP = 0;

	/// Additional XP from ranked race multiplier
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 RankedBonusXP = 0;

	/// Sum of all XP components
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int64 TotalXP = 0;

	/** Calculate TotalXP from all component values */
	void CalculateTotal()
	{
		TotalXP = BaseXP + PositionXP + BestLapXP + CleanRaceXP + OvertakeXP + DriftXP + NearMissXP + RankedBonusXP;
	}
};

/**
 * @brief Complete rewards package from a race completion
 *
 * Contains all rewards earned from a single race, including:
 * - Currency and XP with detailed breakdown
 * - Reputation changes and tier progression
 * - New content unlocks
 * - Record achievements (personal best, track record)
 *
 * This struct is populated by CalculateRewards() and consumed by GrantRewards()
 * and the post-race UI for display.
 */
USTRUCT(BlueprintType)
struct FMGRaceRewards
{
	GENERATED_BODY()

	// ---- Currency & XP ----

	/// In-game credits earned from the race
	UPROPERTY(BlueprintReadOnly, Category = "Rewards")
	int64 CreditsEarned = 0;

	/// Detailed XP breakdown showing all bonus sources
	UPROPERTY(BlueprintReadOnly, Category = "Rewards")
	FMGXPBreakdown XPBreakdown;

	// ---- Reputation ----

	/// Reputation points earned for the associated crew
	UPROPERTY(BlueprintReadOnly, Category = "Reputation")
	int32 ReputationEarned = 0;

	/// Which crew receives the reputation (based on race type/location)
	UPROPERTY(BlueprintReadOnly, Category = "Reputation")
	EMGCrew ReputationCrew = EMGCrew::None;

	// ---- Level Progression ----

	/// True if player gained a level from this race's XP
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	bool bLeveledUp = false;

	/// The new level reached (only valid if bLeveledUp is true)
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 NewLevel = 0;

	// ---- Reputation Tier ----

	/// True if reputation tier changed with any crew
	UPROPERTY(BlueprintReadOnly, Category = "Reputation")
	bool bReputationTierChanged = false;

	/// New reputation tier (only valid if bReputationTierChanged is true)
	UPROPERTY(BlueprintReadOnly, Category = "Reputation")
	EMGReputationTier NewReputationTier = EMGReputationTier::Unknown;

	// ---- Unlocks ----

	/// New content unlocked as a result of this race's progression
	UPROPERTY(BlueprintReadOnly, Category = "Unlocks")
	TArray<FMGUnlock> NewUnlocks;

	// ---- Records ----

	/// True if player set a new global track record
	UPROPERTY(BlueprintReadOnly, Category = "Records")
	bool bNewTrackRecord = false;

	/// True if player set a new personal best time
	UPROPERTY(BlueprintReadOnly, Category = "Records")
	bool bNewPersonalBest = false;
};

/**
 * @brief Performance metrics collected during a race session
 *
 * This struct accumulates real-time performance data as the player races.
 * At race end, these metrics are used to calculate XP bonuses.
 *
 * Recording methods:
 * - RecordOvertake() / RecordOvertaken() - position changes
 * - RecordDriftScore() - drift points
 * - RecordNearMiss() / RecordCollision() - close calls
 * - RecordMaxSpeed() - top speed tracking
 */
USTRUCT(BlueprintType)
struct FMGRacePerformanceData
{
	GENERATED_BODY()

	// ---- Overtaking ----

	/// Number of opponents passed during the race
	UPROPERTY(BlueprintReadWrite, Category = "Overtakes")
	int32 Overtakes = 0;

	/// Number of times passed by opponents
	UPROPERTY(BlueprintReadWrite, Category = "Overtakes")
	int32 TimesOvertaken = 0;

	// ---- Skill Metrics ----

	/// Accumulated drift score points
	UPROPERTY(BlueprintReadWrite, Category = "Skills")
	float TotalDriftScore = 0.0f;

	/// Count of near-miss close passes with traffic/opponents
	UPROPERTY(BlueprintReadWrite, Category = "Skills")
	int32 NearMisses = 0;

	/// Count of collisions with objects/opponents (affects clean race bonus)
	UPROPERTY(BlueprintReadWrite, Category = "Skills")
	int32 Collisions = 0;

	// ---- Speed & Distance ----

	/// Peak speed achieved during the race (in MPH)
	UPROPERTY(BlueprintReadWrite, Category = "Speed")
	float MaxSpeedMPH = 0.0f;

	/// Total distance traveled (in kilometers)
	UPROPERTY(BlueprintReadWrite, Category = "Distance")
	float DistanceKm = 0.0f;

	// ---- Position Tracking ----

	/// Grid position at race start
	UPROPERTY(BlueprintReadWrite, Category = "Position")
	int32 StartingPosition = 0;

	/// Final position at race end
	UPROPERTY(BlueprintReadWrite, Category = "Position")
	int32 FinalPosition = 0;

	/// True if player crossed the finish line (not DNF)
	UPROPERTY(BlueprintReadWrite, Category = "Position")
	bool bFinished = false;

	// ---- Computed Properties ----

	/** @return True if race was completed without collisions */
	bool IsCleanRace() const { return Collisions == 0 && bFinished; }

	/** @return Number of positions gained (positive) or lost (negative) */
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

// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MGBalancingSubsystem.h - Dynamic Game Balancing System
 * =============================================================================
 *
 * OVERVIEW:
 * ---------
 * This file defines the core game balancing subsystem for Midnight Grind. Game
 * balancing is the process of adjusting game parameters to ensure the game feels
 * fair, fun, and appropriately challenging for players of different skill levels.
 *
 * Think of this subsystem as the "behind the scenes" manager that can adjust:
 * - How much money (credits) players earn from races
 * - How hard the AI opponents are to beat
 * - How quickly players progress through the game
 * - What rewards players receive for different actions
 *
 * WHY IS GAME BALANCING IMPORTANT?
 * --------------------------------
 * Without proper balancing:
 * - New players might find the game too hard and quit
 * - Experienced players might find it too easy and get bored
 * - The in-game economy might become broken (too easy or too hard to earn money)
 * - Progression might feel tedious or unrewarding
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. GAME INSTANCE SUBSYSTEM:
 *    This class inherits from UGameInstanceSubsystem, which means it persists
 *    across level changes. Unlike actors that are destroyed when loading a new
 *    level, this subsystem stays alive for the entire game session. This is
 *    important because balance settings need to remain consistent.
 *
 * 2. BLUEPRINTABLE:
 *    Many properties and functions are marked with UPROPERTY/UFUNCTION macros
 *    that include "BlueprintReadWrite" or "BlueprintCallable". This allows
 *    designers to access and modify these values from Blueprint scripts without
 *    needing to write C++ code.
 *
 * 3. ADAPTIVE/DYNAMIC DIFFICULTY:
 *    The system can automatically adjust difficulty based on how well the player
 *    is doing. If they keep losing, it gets easier. If they keep winning, it
 *    gets harder. This keeps the game in the "flow zone" where it's challenging
 *    but not frustrating.
 *
 * 4. REMOTE CONFIG:
 *    The system supports receiving balance updates from a server. This is useful
 *    for "live ops" - adjusting game balance after the game is released without
 *    requiring players to download a patch.
 *
 * 5. DELEGATES/EVENTS:
 *    The DECLARE_DYNAMIC_MULTICAST_DELEGATE macros define events that other
 *    parts of the game can subscribe to. For example, when difficulty changes,
 *    the UI might need to update to show the new difficulty level.
 *
 * MAIN COMPONENTS:
 * ----------------
 * - EMGBalanceCategory: Categories of parameters that can be balanced
 * - EMGDifficultyTier: Preset difficulty levels from Beginner to Master
 * - FMGBalanceParameter: A single adjustable game parameter
 * - FMGDifficultyProfile: Settings for a specific difficulty level
 * - FMGEconomyBalance: All economy-related balance values
 * - FMGPlayerMetrics: Tracked player performance data
 * - FMGAdaptiveDifficultyState: State of the automatic difficulty adjustment
 * - UMGBalancingSubsystem: The main subsystem class that manages all of this
 *
 * USAGE EXAMPLE:
 * --------------
 * @code
 * // Get the subsystem from the game instance
 * UMGBalancingSubsystem* BalanceSystem = GetGameInstance()->GetSubsystem<UMGBalancingSubsystem>();
 *
 * // Check what difficulty the player is on
 * EMGDifficultyTier CurrentTier = BalanceSystem->GetDifficultyTier();
 *
 * // Get the current reward multiplier
 * float RewardMultiplier = BalanceSystem->GetParameter("RewardMultiplier");
 *
 * // Record a race result for adaptive difficulty
 * BalanceSystem->RecordRaceResult(Position, TotalRacers, RaceTimeSeconds);
 * @endcode
 *
 * @see UMGDynamicDifficultySubsystem for more detailed difficulty adjustments
 * @see UMGEconomySubsystem for the actual economy implementation
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGBalancingSubsystem.generated.h"

/**
 * Dynamic Game Balancing System
 * - Monitors player performance and adjusts difficulty curves
 * - Balances economy values based on player progression
 * - Tracks meta-game statistics for tuning decisions
 * - Provides tools for live-ops balancing adjustments
 */

/**
 * Categories of balance parameters that can be adjusted.
 *
 * Each category groups related parameters together for easier management.
 * For example, all credit multipliers would be under "Economy", while
 * AI behavior settings would be under "AI".
 */
UENUM(BlueprintType)
enum class EMGBalanceCategory : uint8
{
	/** In-game currency: earnings, prices, rewards */
	Economy,
	/** How hard the game is: AI skill, obstacle density */
	Difficulty,
	/** How fast players advance: XP rates, unlock speeds */
	Progression,
	/** What players receive: loot drops, bonus items */
	Rewards,
	/** Computer opponent behavior: aggression, rubber-banding */
	AI,
	/** Vehicle handling: grip, top speed, acceleration */
	Physics,
	/** Online play: skill matching, lobby formation */
	Matchmaking
};

/**
 * Preset difficulty tiers that define the overall challenge level.
 *
 * Each tier has an associated FMGDifficultyProfile that defines all the
 * specific settings (AI aggression, rubber-banding strength, etc.) for
 * that difficulty level.
 *
 * Generally, lower tiers have more assists, easier AI, and higher rewards
 * to help new players enjoy the game and learn mechanics.
 */
UENUM(BlueprintType)
enum class EMGDifficultyTier : uint8
{
	/** For brand new players - lots of assists, very easy AI */
	Beginner,
	/** Relaxed play - assists available, forgiving AI */
	Casual,
	/** The intended baseline experience for most players */
	Normal,
	/** For players seeking a challenge - fewer assists, smarter AI */
	Competitive,
	/** Hard mode - minimal assists, aggressive AI */
	Expert,
	/** Maximum challenge - no assists, ruthless AI opponents */
	Master
};

/**
 * Flags used to mark the tuning status of a balance parameter.
 *
 * These flags help designers and developers track which parameters
 * need attention. Useful during development and live ops when
 * reviewing game balance data.
 */
UENUM(BlueprintType)
enum class EMGBalanceFlag : uint8
{
	/** No specific status assigned */
	None,
	/** Parameter value is too low - game might be too hard/stingy */
	UnderTuned,
	/** Parameter is correctly tuned based on player data */
	Balanced,
	/** Parameter value is too high - game might be too easy/generous */
	OverTuned,
	/** Flagged for manual review by a designer */
	NeedsReview
};

/**
 * A single adjustable game balance parameter.
 *
 * Balance parameters are the "knobs" that designers turn to tune the game.
 * Examples: "RaceRewardMultiplier", "AIAggressionBase", "VehiclePriceScale"
 *
 * Each parameter has:
 * - A base/default value
 * - A current value (what's actually being used)
 * - Min/max limits to prevent extreme values
 * - Optional remote override from server-side config
 * - A tuning status flag for development tracking
 */
USTRUCT(BlueprintType)
struct FMGBalanceParameter
{
	GENERATED_BODY()

	/** Unique identifier for this parameter (e.g., "RaceRewardMultiplier") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ParameterID;

	/** Human-readable name shown in debug UIs and tools */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Which category this parameter belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBalanceCategory Category = EMGBalanceCategory::Economy;

	/** The default value before any adjustments */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseValue = 1.0f;

	/** The current active value being used in gameplay */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentValue = 1.0f;

	/** Minimum allowed value - prevents parameter from going below this */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue = 0.0f;

	/** Maximum allowed value - prevents parameter from exceeding this */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 10.0f;

	/** Value received from server for live-ops override. -1.0 means no override. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RemoteOverride = -1.0f;

	/** If true, use RemoteOverride instead of CurrentValue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bUseRemoteOverride = false;

	/** Current tuning status for development tracking */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGBalanceFlag Flag = EMGBalanceFlag::Balanced;
};

/**
 * Complete settings for a specific difficulty tier.
 *
 * A difficulty profile bundles all the settings that should change together
 * when a player selects a difficulty. For example, "Beginner" might have:
 * - Lower AI aggression (so AI doesn't ram you)
 * - Stronger rubber-banding for player (catch up when behind)
 * - Higher reward multiplier (earn credits faster)
 * - All assists enabled
 */
USTRUCT(BlueprintType)
struct FMGDifficultyProfile
{
	GENERATED_BODY()

	/** Which difficulty tier these settings are for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDifficultyTier Tier = EMGDifficultyTier::Normal;

	/**
	 * How aggressive AI opponents are (0.0 = passive, 2.0 = very aggressive).
	 * Affects ramming, blocking, and risky overtake attempts.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIAggressionMultiplier = 1.0f;

	/**
	 * How much AI speeds up when behind / slows down when ahead (0.0 to 1.0).
	 * Higher values keep races artificially close. Set to 0 for pure simulation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AIRubberBandingStrength = 0.5f;

	/** Multiplier for all credit rewards. Higher = faster economy progression. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RewardMultiplier = 1.0f;

	/** Multiplier for XP and career progression. Higher = level up faster. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressionSpeedMultiplier = 1.0f;

	/**
	 * How much AI skill varies between opponents in a race (0.0 to 1.0).
	 * Higher values create more spread between fastest and slowest AI.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OpponentSkillVariance = 0.2f;

	/** Whether driving assists (steering help, brake assist) are available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableAssists = true;

	/** Whether to show the optimal racing line on the track */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRacingLine = true;

	/** Whether to force automatic transmission (true) or allow manual (false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutoTransmission = true;
};

/**
 * Economy balance settings that control the in-game money flow.
 *
 * A healthy game economy is crucial for player retention. If players earn
 * too much too fast, they buy everything quickly and get bored. If they
 * earn too little, they feel like progress is too slow and give up.
 *
 * The "target" fields help designers work backward from desired outcomes:
 * "We want an average vehicle to take 15 races to afford" then set
 * earnings and prices to achieve that.
 */
USTRUCT(BlueprintType)
struct FMGEconomyBalance
{
	GENERATED_BODY()

	/** Base credits earned for completing a race (before position bonuses) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseRaceEarnings = 1000.0f;

	/** Bonus percentage added for winning (50% = 1.5x base earnings for 1st) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinBonusPercent = 50.0f;

	/** Bonus percentage for podium finish (2nd or 3rd place) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PodiumBonusPercent = 25.0f;

	/** Multiplier applied to all vehicle purchase prices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VehiclePriceMultiplier = 1.0f;

	/** Multiplier applied to all performance part prices */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PartPriceMultiplier = 1.0f;

	/** Multiplier applied to cosmetic item prices (paint, decals, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CosmeticPriceMultiplier = 1.0f;

	/** Base amount for daily login bonus */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DailyBonusBaseAmount = 500.0f;

	/** Rate at which prices increase over time (0.0 = no inflation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InflationRate = 0.0f;

	/** Design target: how many races should it take to afford an average car? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetRacesToUnlockVehicle = 15;

	/** Design target: how many hours to reach endgame content? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TargetHoursToEndgame = 40;
};

/**
 * Aggregated player performance metrics used for balance decisions.
 *
 * These metrics summarize how well the player is doing and how they're
 * engaging with the game. The adaptive difficulty system uses these to
 * decide if the game should get easier or harder.
 *
 * Note: This is different from FMGPlayerStats in MGStatsTracker.h, which
 * tracks cumulative all-time stats. These metrics focus on recent
 * performance for balance purposes.
 */
USTRUCT(BlueprintType)
struct FMGPlayerMetrics
{
	GENERATED_BODY()

	/** Player's average finishing position (1.0 = always wins, 8.0 = always last in 8-racer field) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageRacePosition = 4.0f;

	/** Percentage of races won (0.15 = 15% win rate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.15f;

	/** Percentage of races finishing in top 3 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PodiumRate = 0.35f;

	/** Percentage of races not finished (crashes, quits, timeouts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DNFRate = 0.05f;

	/** Total number of races used to calculate these metrics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRaces = 0;

	/** Average time to complete a race in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageRaceTimeSeconds = 180.0f;

	/** Average play session length in minutes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SessionLengthMinutes = 45.0f;

	/** How many races in a row the player has lost (non-podium) - important for frustration detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsecutiveLosses = 0;

	/** How many races in a row the player has won - might need harder difficulty */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ConsecutiveWins = 0;

	/** Elo-style skill rating (1000 = average, higher = better) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillRating = 1000.0f;

	/**
	 * Estimated frustration level (0.0 = happy, 1.0 = very frustrated).
	 * Calculated from consecutive losses, DNF rate, and other factors.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FrustrationIndex = 0.0f;

	/**
	 * How engaged the player is (0.0 = disengaged, 1.0+ = highly engaged).
	 * Based on session length, race frequency, and feature usage.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EngagementScore = 1.0f;
};

/**
 * A saved snapshot of balance state at a point in time.
 *
 * Snapshots are useful for:
 * - Debugging: "What were the settings when this bug happened?"
 * - A/B testing: "Let's try these settings and compare to the snapshot"
 * - Rollback: "The new settings made things worse, restore the snapshot"
 *
 * Designers can take snapshots before making changes, then restore them
 * if the changes don't work out.
 */
USTRUCT(BlueprintType)
struct FMGBalanceSnapshot
{
	GENERATED_BODY()

	/** When this snapshot was taken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/** All parameter values at the time of the snapshot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> ParameterValues;

	/** Player metrics at the time of the snapshot */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerMetrics PlayerMetrics;

	/** Optional description of why this snapshot was taken */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Notes;
};

/**
 * State of the adaptive difficulty system.
 *
 * Adaptive difficulty automatically adjusts the game challenge based on
 * player performance. The goal is to keep players winning at roughly the
 * TargetWinRate - not so much they get bored, not so little they get frustrated.
 *
 * The system uses a modifier that multiplies AI skill/aggression. Higher
 * modifier = harder game, lower modifier = easier game.
 */
USTRUCT(BlueprintType)
struct FMGAdaptiveDifficultyState
{
	GENERATED_BODY()

	/** Whether adaptive difficulty is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnabled = true;

	/**
	 * Current difficulty modifier (1.0 = baseline, 0.5 = half difficulty, 1.5 = 50% harder).
	 * Applied as a multiplier to AI skill, aggression, and similar parameters.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CurrentModifier = 1.0f;

	/**
	 * The win rate we're trying to achieve (0.25 = player should win 25% of races).
	 * A 25% win rate feels fair in an 8-racer field (random would be 12.5%).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TargetWinRate = 0.25f;

	/**
	 * How fast the system adjusts (0.0 to 1.0).
	 * Higher values = faster response but potentially jarring swings.
	 * Lower values = smoother but might take too long to respond.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AdjustmentSpeed = 0.1f;

	/** Minimum modifier value - prevents game from becoming trivially easy */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinModifier = 0.5f;

	/** Maximum modifier value - prevents game from becoming impossibly hard */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxModifier = 1.5f;

	/** Number of races since the last difficulty adjustment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RacesSinceLastAdjustment = 0;
};

// =============================================================================
// DELEGATE DECLARATIONS
// =============================================================================
// Delegates are Unreal's event system. Other classes can "subscribe" to these
// events and get notified when they fire. This is the Observer pattern.

/** Fired when any balance parameter changes. Useful for updating debug UIs. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnBalanceParameterChanged, FName, ParameterID, float, NewValue);

/** Fired when the player changes difficulty tier. UI should update to show new tier. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDifficultyTierChanged, EMGDifficultyTier, NewTier);

/** Fired when adaptive difficulty adjusts the modifier. For debugging/analytics. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnAdaptiveDifficultyAdjusted, float, NewModifier);

/** Fired when new configuration is received from the server. Apply pending changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnRemoteConfigReceived);

UCLASS()
class MIDNIGHTGRIND_API UMGBalancingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Parameter Management
	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void SetParameter(FName ParameterID, float Value);

	UFUNCTION(BlueprintPure, Category = "Balancing")
	float GetParameter(FName ParameterID) const;

	UFUNCTION(BlueprintPure, Category = "Balancing")
	FMGBalanceParameter GetParameterInfo(FName ParameterID) const;

	UFUNCTION(BlueprintPure, Category = "Balancing")
	TArray<FMGBalanceParameter> GetParametersByCategory(EMGBalanceCategory Category) const;

	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void ResetParameterToDefault(FName ParameterID);

	UFUNCTION(BlueprintCallable, Category = "Balancing")
	void ResetAllParameters();

	// Difficulty Management
	UFUNCTION(BlueprintCallable, Category = "Balancing|Difficulty")
	void SetDifficultyTier(EMGDifficultyTier Tier);

	UFUNCTION(BlueprintPure, Category = "Balancing|Difficulty")
	EMGDifficultyTier GetDifficultyTier() const { return CurrentDifficultyTier; }

	UFUNCTION(BlueprintPure, Category = "Balancing|Difficulty")
	FMGDifficultyProfile GetDifficultyProfile() const { return CurrentDifficultyProfile; }

	UFUNCTION(BlueprintPure, Category = "Balancing|Difficulty")
	FMGDifficultyProfile GetDifficultyProfileForTier(EMGDifficultyTier Tier) const;

	// Adaptive Difficulty
	UFUNCTION(BlueprintCallable, Category = "Balancing|Adaptive")
	void SetAdaptiveDifficultyEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Balancing|Adaptive")
	bool IsAdaptiveDifficultyEnabled() const { return AdaptiveDifficultyState.bEnabled; }

	UFUNCTION(BlueprintPure, Category = "Balancing|Adaptive")
	FMGAdaptiveDifficultyState GetAdaptiveDifficultyState() const { return AdaptiveDifficultyState; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Adaptive")
	void RecordRaceResult(int32 Position, int32 TotalRacers, float RaceTimeSeconds);

	UFUNCTION(BlueprintCallable, Category = "Balancing|Adaptive")
	float GetCurrentDifficultyModifier() const { return AdaptiveDifficultyState.CurrentModifier; }

	// Economy Balance
	UFUNCTION(BlueprintPure, Category = "Balancing|Economy")
	FMGEconomyBalance GetEconomyBalance() const { return EconomyBalance; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Economy")
	void SetEconomyBalance(const FMGEconomyBalance& Balance);

	UFUNCTION(BlueprintPure, Category = "Balancing|Economy")
	float CalculateAdjustedPrice(float BasePrice, EMGBalanceCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Balancing|Economy")
	float CalculateAdjustedReward(float BaseReward) const;

	// Player Metrics
	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	FMGPlayerMetrics GetPlayerMetrics() const { return PlayerMetrics; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Metrics")
	void UpdatePlayerMetrics(const FMGPlayerMetrics& Metrics);

	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	EMGDifficultyTier RecommendDifficultyTier() const;

	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	bool IsPlayerFrustrated() const;

	UFUNCTION(BlueprintPure, Category = "Balancing|Metrics")
	bool IsPlayerBored() const;

	// Remote Config
	UFUNCTION(BlueprintCallable, Category = "Balancing|Remote")
	void FetchRemoteConfig();

	UFUNCTION(BlueprintCallable, Category = "Balancing|Remote")
	void ApplyRemoteOverrides();

	UFUNCTION(BlueprintPure, Category = "Balancing|Remote")
	bool HasPendingRemoteConfig() const { return bHasPendingRemoteConfig; }

	// Snapshots
	UFUNCTION(BlueprintCallable, Category = "Balancing|Debug")
	void TakeSnapshot(const FString& Notes = TEXT(""));

	UFUNCTION(BlueprintPure, Category = "Balancing|Debug")
	TArray<FMGBalanceSnapshot> GetSnapshots() const { return Snapshots; }

	UFUNCTION(BlueprintCallable, Category = "Balancing|Debug")
	void RestoreSnapshot(int32 Index);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnBalanceParameterChanged OnBalanceParameterChanged;

	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnDifficultyTierChanged OnDifficultyTierChanged;

	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnAdaptiveDifficultyAdjusted OnAdaptiveDifficultyAdjusted;

	UPROPERTY(BlueprintAssignable, Category = "Balancing|Events")
	FMGOnRemoteConfigReceived OnRemoteConfigReceived;

protected:
	void InitializeDefaultParameters();
	void InitializeDifficultyProfiles();
	void LoadBalanceData();
	void SaveBalanceData();
	void UpdateAdaptiveDifficulty();
	void CalculateFrustrationIndex();

private:
	UPROPERTY()
	TMap<FName, FMGBalanceParameter> Parameters;

	UPROPERTY()
	TMap<EMGDifficultyTier, FMGDifficultyProfile> DifficultyProfiles;

	UPROPERTY()
	EMGDifficultyTier CurrentDifficultyTier = EMGDifficultyTier::Normal;

	UPROPERTY()
	FMGDifficultyProfile CurrentDifficultyProfile;

	UPROPERTY()
	FMGAdaptiveDifficultyState AdaptiveDifficultyState;

	UPROPERTY()
	FMGEconomyBalance EconomyBalance;

	UPROPERTY()
	FMGPlayerMetrics PlayerMetrics;

	UPROPERTY()
	TArray<FMGBalanceSnapshot> Snapshots;

	TMap<FName, float> PendingRemoteOverrides;
	bool bHasPendingRemoteConfig = false;
};

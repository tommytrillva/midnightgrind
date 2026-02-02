// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * @file MGRaceFlowManager.h
 * @brief Race Flow Manager - Orchestrates the complete race lifecycle and progression.
 *
 * The Race Flow Manager is the central coordinator for everything that happens
 * before, during, and after a race. It handles level loading, state transitions,
 * reward calculations, and integration with all the game's progression systems.
 *
 * @section overview_flow Overview
 * Think of the Race Flow Manager as the "producer" of a race event. While the
 * Game Mode is the "director" handling moment-to-moment gameplay, the Flow
 * Manager handles the bigger picture: loading the track, showing loading screens,
 * processing rewards, updating career progress, and handling the results screen.
 *
 * @section concepts_flow Key Concepts for Beginners
 *
 * 1. **Game Instance Subsystem**: This class extends UGameInstanceSubsystem,
 *    meaning there's one instance that persists across level loads. Perfect for
 *    managing race flow since we need to survive the level transition when
 *    loading a new track.
 *
 * 2. **State Machine**: The manager progresses through defined states
 *    (Idle -> LoadingTrack -> PreRace -> Racing -> ShowingResults -> etc.).
 *    Each state has specific responsibilities and valid transitions.
 *
 * 3. **Reward Breakdown**: Rather than just giving players a lump sum, the
 *    FMGRaceRewardBreakdown struct itemizes every bonus (clean race, best lap,
 *    difficulty, etc.) for a satisfying results screen.
 *
 * 4. **Unlocks**: The FMGRaceUnlock struct represents things players earn
 *    (new cars, parts, tracks, achievements) based on race performance.
 *
 * @section lifecycle_flow Race Lifecycle
 *
 * The complete flow of a race session:
 *
 * @code
 *    [Idle] -- BeginRaceLoad() --> [LoadingTrack]
 *       |                              |
 *       |                        Level loads async
 *       |                              |
 *       |   <-- OnTrackLoaded() -- [PreRace]
 *       |                              |
 *       |                        Player ready, countdown
 *       |                              |
 *       |                         [Countdown]
 *       |                              |
 *       |                          [Racing]
 *       |                              |
 *       |   <-- OnRaceFinished() -- [RaceEnding]
 *       |                              |
 *       |                        [ShowingResults]
 *       |                              |
 *       |   <-- ConfirmResults() -- [ProcessingRewards]
 *       |                              |
 *       |                        [ShowingUnlocks]
 *       |                              |
 *       |   <-- SkipToPostRace() -- [PostRace]
 *       |                              |
 *       | <-- ExitRace()/Restart() -- [Exiting]
 *       |                              |
 *       +------------------------------+
 * @endcode
 *
 * @section architecture_flow Architecture
 *
 * The Flow Manager integrates with multiple subsystems:
 *
 *    [AMGRaceGameMode]  <---> [UMGRaceFlowManager]
 *           |                        |
 *    Race Events             +-------+-------+-------+
 *    (Lap, Finish)           |       |       |       |
 *                            v       v       v       v
 *                    [Progression] [Shop] [Career] [Leaderboard]
 *                          |         |       |         |
 *                         XP      Credits  Progress  Rankings
 *
 * @section usage_flow Usage Example
 *
 * @code
 * // Starting a race from a menu:
 * UMGRaceFlowManager* FlowManager = GetGameInstance()->GetSubsystem<UMGRaceFlowManager>();
 *
 * // 1. Configure and start loading
 * FSoftObjectPath TrackPath("/Game/Maps/Downtown");
 * FMGRaceConfig Config;
 * Config.RaceType = EMGRaceType::Circuit;
 * Config.LapCount = 3;
 *
 * FlowManager->BeginRaceLoad(TrackPath, Config, PlayerVehicleID);
 *
 * // 2. Subscribe to events for UI updates
 * FlowManager->OnFlowStateChanged.AddDynamic(this, &UMyWidget::OnStateChanged);
 * FlowManager->OnRewardsProcessed.AddDynamic(this, &UMyWidget::ShowRewards);
 *
 * // 3. When race completes (called by GameMode):
 * // FlowManager->OnRaceFinished(Results);
 *
 * // 4. Player confirms results screen
 * // FlowManager->ConfirmResults();  // Triggers reward processing
 *
 * // 5. Exit when done
 * // FlowManager->ExitRace("MainMenu");  // Returns to menu
 * // -or-
 * // FlowManager->QuickRestart();  // Restart same race
 * @endcode
 *
 * @section rewards_flow Reward Calculation
 *
 * The reward system considers multiple factors:
 * - **Base Position Reward**: Credits based on finishing position (1st-8th+)
 * - **Lap Bonus**: Extra credits for longer races
 * - **Difficulty Bonus**: Higher AI difficulty = more credits
 * - **Perfect Start Bonus**: No collisions on first lap
 * - **Clean Race Bonus**: Completing without collisions
 * - **Drift Bonus**: Points from drift score (drift races)
 * - **Best Lap Bonus**: Setting the race's fastest lap
 * - **Modifier Multiplier**: Active modifiers may increase/decrease rewards
 *
 * @see AMGRaceGameMode For the game mode that runs the actual race
 * @see UMGRaceConfiguration For race setup data assets
 * @see UMGPlayerProgression For XP and leveling
 * @see UMGCareerSubsystem For career mode progress
 */

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameModes/MGRaceGameMode.h"
#include "MGRaceFlowManager.generated.h"

class UMGPlayerProgression;
class UMGShopSubsystem;
class UMGCareerSubsystem;
class UMGLeaderboardSubsystem;
class UMGRaceResultsWidget;
class UMGLoadingScreenWidget;
class AMGVehiclePawn;

/**
 * Race reward breakdown
 */
USTRUCT(BlueprintType)
struct FMGRaceRewardBreakdown
{
	GENERATED_BODY()

	/** Base position reward */
	UPROPERTY(BlueprintReadOnly)
	int64 BaseCredits = 0;

	/** Bonus for lap count */
	UPROPERTY(BlueprintReadOnly)
	int64 LapBonus = 0;

	/** Bonus for difficulty */
	UPROPERTY(BlueprintReadOnly)
	int64 DifficultyBonus = 0;

	/** Perfect start bonus */
	UPROPERTY(BlueprintReadOnly)
	int64 PerfectStartBonus = 0;

	/** Clean race bonus (no collisions) */
	UPROPERTY(BlueprintReadOnly)
	int64 CleanRaceBonus = 0;

	/** Drift score bonus */
	UPROPERTY(BlueprintReadOnly)
	int64 DriftBonus = 0;

	/** Best lap bonus */
	UPROPERTY(BlueprintReadOnly)
	int64 BestLapBonus = 0;

	/** Pink slip winnings (vehicle value) */
	UPROPERTY(BlueprintReadOnly)
	int64 PinkSlipBonus = 0;

	/** Modifier multiplier bonus/penalty */
	UPROPERTY(BlueprintReadOnly)
	float ModifierMultiplier = 1.0f;

	/** XP earned */
	UPROPERTY(BlueprintReadOnly)
	int32 XPEarned = 0;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly)
	int32 ReputationEarned = 0;

	/** Total credits */
	UPROPERTY(BlueprintReadOnly)
	int64 TotalCredits = 0;

	/** Bonuses as display strings */
	UPROPERTY(BlueprintReadOnly)
	TArray<FText> BonusDescriptions;

	/** Calculate total */
	void CalculateTotal()
	{
		int64 Subtotal = BaseCredits + LapBonus + DifficultyBonus + PerfectStartBonus +
			CleanRaceBonus + DriftBonus + BestLapBonus + PinkSlipBonus;
		TotalCredits = static_cast<int64>(Subtotal * ModifierMultiplier);
	}
};

/**
 * Unlock earned during race
 */
USTRUCT(BlueprintType)
struct FMGRaceUnlock
{
	GENERATED_BODY()

	/** Unlock type */
	UPROPERTY(BlueprintReadOnly)
	FName UnlockType;

	/** Unlocked item ID */
	UPROPERTY(BlueprintReadOnly)
	FName ItemID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Description */
	UPROPERTY(BlueprintReadOnly)
	FText Description;

	/** Rarity tier */
	UPROPERTY(BlueprintReadOnly)
	int32 Rarity = 0;
};

/**
 * Complete post-race summary
 */
USTRUCT(BlueprintType)
struct FMGPostRaceSummary
{
	GENERATED_BODY()

	/** Race results */
	UPROPERTY(BlueprintReadOnly)
	FMGRaceResults RaceResults;

	/** Detailed reward breakdown */
	UPROPERTY(BlueprintReadOnly)
	FMGRaceRewardBreakdown Rewards;

	/** New unlocks earned */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGRaceUnlock> Unlocks;

	/** Challenge progress updates */
	UPROPERTY(BlueprintReadOnly)
	TArray<FText> ChallengeProgress;

	/** Career progress update */
	UPROPERTY(BlueprintReadOnly)
	FText CareerProgressUpdate;

	/** Player level before */
	UPROPERTY(BlueprintReadOnly)
	int32 LevelBefore = 1;

	/** Player level after */
	UPROPERTY(BlueprintReadOnly)
	int32 LevelAfter = 1;

	/** XP progress before (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float XPProgressBefore = 0.0f;

	/** XP progress after (0-1) */
	UPROPERTY(BlueprintReadOnly)
	float XPProgressAfter = 0.0f;

	/** Did level up */
	UPROPERTY(BlueprintReadOnly)
	bool bLeveledUp = false;

	/** New personal best lap */
	UPROPERTY(BlueprintReadOnly)
	bool bNewPersonalBest = false;

	/** Track record broken */
	UPROPERTY(BlueprintReadOnly)
	bool bTrackRecord = false;

	/** Rival defeated (if racing against rival) */
	UPROPERTY(BlueprintReadOnly)
	FText RivalDefeated;
};

/**
 * Race flow state
 */
UENUM(BlueprintType)
enum class EMGRaceFlowState : uint8
{
	Idle,
	LoadingTrack,
	PreRace,
	Countdown,
	Racing,
	RaceEnding,
	ShowingResults,
	ProcessingRewards,
	ShowingUnlocks,
	PostRace,
	Exiting
};

/**
 * Delegate for flow state changes
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFlowStateChanged, EMGRaceFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardsProcessed, const FMGPostRaceSummary&, Summary);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceFlowComplete);

/**
 * Race Flow Manager
 * Manages the complete race flow including transitions, rewards, and progression integration
 *
 * Features:
 * - Handles loading -> pre-race -> race -> results -> post-race flow
 * - Integrates with Progression, Economy, Career subsystems
 * - Calculates detailed reward breakdowns
 * - Triggers unlocks and challenge updates
 * - Manages UI transitions (loading screen, results widget)
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceFlowManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// RACE FLOW CONTROL
	// ==========================================

	/**
	 * Begin loading a race
	 * @param TrackMapPath Level to load
	 * @param RaceConfig Race configuration
	 * @param PlayerVehicleID Vehicle player is using
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void BeginRaceLoad(const FSoftObjectPath& TrackMapPath, const FMGRaceConfig& RaceConfig, FName PlayerVehicleID);

	/**
	 * Called when track is loaded and ready
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void OnTrackLoaded();

	/**
	 * Signal race has finished (called by game mode)
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void OnRaceFinished(const FMGRaceResults& Results);

	/**
	 * Player confirmed results, process rewards
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void ConfirmResults();

	/**
	 * Skip directly to post-race
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void SkipToPostRace();

	/**
	 * Exit race and return to specified destination
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void ExitRace(FName Destination = NAME_None);

	/**
	 * Quick restart current race
	 */
	UFUNCTION(BlueprintCallable, Category = "Race Flow")
	void QuickRestart();

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Get current flow state */
	UFUNCTION(BlueprintPure, Category = "Race Flow|State")
	EMGRaceFlowState GetFlowState() const { return CurrentState; }

	/** Is race in progress */
	UFUNCTION(BlueprintPure, Category = "Race Flow|State")
	bool IsRaceInProgress() const;

	/** Can restart race */
	UFUNCTION(BlueprintPure, Category = "Race Flow|State")
	bool CanRestart() const;

	/** Get post-race summary */
	UFUNCTION(BlueprintPure, Category = "Race Flow|State")
	const FMGPostRaceSummary& GetPostRaceSummary() const { return PostRaceSummary; }

	/** Get current race config */
	UFUNCTION(BlueprintPure, Category = "Race Flow|State")
	const FMGRaceConfig& GetCurrentRaceConfig() const { return CurrentConfig; }

	// ==========================================
	// EVENTS
	// ==========================================

	/** Flow state changed */
	UPROPERTY(BlueprintAssignable, Category = "Race Flow|Events")
	FOnRaceFlowStateChanged OnFlowStateChanged;

	/** Rewards processed and summary ready */
	UPROPERTY(BlueprintAssignable, Category = "Race Flow|Events")
	FOnRewardsProcessed OnRewardsProcessed;

	/** Race flow complete (exited or restarted) */
	UPROPERTY(BlueprintAssignable, Category = "Race Flow|Events")
	FOnRaceFlowComplete OnFlowComplete;

protected:
	// ==========================================
	// INTERNAL METHODS
	// ==========================================

	/** Set new flow state */
	void SetFlowState(EMGRaceFlowState NewState);

	/** Calculate reward breakdown */
	FMGRaceRewardBreakdown CalculateRewards(const FMGRaceResults& Results);

	/** Check for new unlocks */
	TArray<FMGRaceUnlock> CheckForUnlocks(const FMGRaceResults& Results);

	/** Update challenge progress */
	TArray<FText> UpdateChallengeProgress(const FMGRaceResults& Results);

	/** Apply rewards to player */
	void ApplyRewards(const FMGRaceRewardBreakdown& Rewards);

	/** Apply unlocks to player */
	void ApplyUnlocks(const TArray<FMGRaceUnlock>& Unlocks);

	/** Submit to leaderboards */
	void SubmitToLeaderboards(const FMGRaceResults& Results);

	/** Update rival relationships */
	void UpdateRivalRelationships(const FMGRaceResults& Results);

	/** Cache subsystem references */
	void CacheSubsystems();

	/** Show loading screen */
	void ShowLoadingScreen();

	/** Hide loading screen */
	void HideLoadingScreen();

	/** Show results widget */
	void ShowResultsWidget();

	/** Build post-race summary */
	void BuildPostRaceSummary(const FMGRaceResults& Results);

private:
	// ==========================================
	// STATE
	// ==========================================

	/** Current flow state */
	EMGRaceFlowState CurrentState = EMGRaceFlowState::Idle;

	/** Current race config */
	FMGRaceConfig CurrentConfig;

	/** Player vehicle ID */
	FName CurrentPlayerVehicleID;

	/** Track being loaded */
	FSoftObjectPath PendingTrackPath;

	/** Cached race results */
	FMGRaceResults CachedResults;

	/** Post-race summary */
	FMGPostRaceSummary PostRaceSummary;

	/** Restart count this session */
	int32 RestartCount = 0;

	/** Return destination after race */
	FName ExitDestination;

	// ==========================================
	// SUBSYSTEM REFERENCES
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGPlayerProgression> ProgressionSubsystem;

	// Note: Economy handled via UMGShopSubsystem::AddCurrency() - accessed directly when needed

	UPROPERTY()
	TWeakObjectPtr<UMGCareerSubsystem> CareerSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGLeaderboardSubsystem> LeaderboardSubsystem;

	// ==========================================
	// UI
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGLoadingScreenWidget> LoadingScreen;

	UPROPERTY()
	TWeakObjectPtr<UMGRaceResultsWidget> ResultsWidget;

	// ==========================================
	// CONSTANTS
	// ==========================================

	/** Base credits by position (1st through 8th+) */
	static constexpr int64 BaseCredits[8] = {
		10000, 7500, 5000, 3500, 2500, 2000, 1500, 1000
	};

	/** XP by position */
	static constexpr int32 BaseXP[8] = {
		500, 400, 300, 250, 200, 150, 100, 50
	};

	/** Reputation by position */
	static constexpr int32 BaseReputation[8] = {
		100, 75, 50, 35, 25, 15, 10, 5
	};
};

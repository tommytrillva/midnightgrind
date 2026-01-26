// Copyright Midnight Grind. All Rights Reserved.
// Stage 51: Race Flow Subsystem - MVP Game Flow Orchestration

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGRaceFlowSubsystem.generated.h"

class UMGGameStateSubsystem;
class UMGVehicleManagerSubsystem;
class UMGRaceDirectorSubsystem;
class UMGEconomySubsystem;
class UMGGarageSubsystem;

/**
 * Race setup request from garage
 */
USTRUCT(BlueprintType)
struct FMGRaceSetupRequest
{
	GENERATED_BODY()

	/** Track ID to race on */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName TrackID;

	/** Race type (Circuit, Sprint, Drift, Drag, TimeTrial, PinkSlip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName RaceType = FName("Circuit");

	/** Number of laps (for circuit races) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 LapCount = 3;

	/** Number of AI opponents */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 AICount = 7;

	/** AI difficulty (0.0 - 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float AIDifficulty = 0.5f;

	/** Player's selected vehicle ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName PlayerVehicleID;

	/** Time of day (0 = midnight, 0.5 = noon, 1 = midnight) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float TimeOfDay = 0.0f;  // Midnight by default

	/** Weather condition (0 = clear, 1 = heavy rain) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	float Weather = 0.0f;

	/** Is this a pink slip race? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsPinkSlip = false;

	/** Vehicle being wagered (pink slip) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName PinkSlipVehicleID;

	/** Is ranked multiplayer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	bool bIsRanked = false;

	/** Session ID for multiplayer */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString SessionID;

	/** Base cash reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int64 BaseCashReward = 5000;

	/** Base reputation reward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	int32 BaseRepReward = 100;
};

/**
 * Race result returned to garage/menu
 */
USTRUCT(BlueprintType)
struct FMGRaceFlowResult
{
	GENERATED_BODY()

	/** Did the player finish? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bPlayerFinished = false;

	/** Player's final position (1-based) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 PlayerPosition = 0;

	/** Total racers */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 TotalRacers = 0;

	/** Player's total time */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float PlayerTotalTime = 0.0f;

	/** Player's best lap */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float PlayerBestLap = 0.0f;

	/** All lap times */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<float> PlayerLapTimes;

	/** Did player win? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bPlayerWon = false;

	/** Cash earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int64 CashEarned = 0;

	/** Reputation earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 ReputationEarned = 0;

	/** XP earned */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int32 XPEarned = 0;

	/** Drift score (drift races) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	int64 DriftScore = 0;

	/** Pink slip won vehicle (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FName PinkSlipWonVehicleID;

	/** Pink slip lost vehicle (if applicable) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FName PinkSlipLostVehicleID;

	/** Track record beaten? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bNewTrackRecord = false;

	/** Personal best beaten? */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bNewPersonalBest = false;

	/** Race was completed (not aborted) */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	bool bRaceCompleted = false;

	/** All racer results for leaderboard display */
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	TArray<FName> FinishOrder;
};

/**
 * AI racer info for race setup
 */
USTRUCT(BlueprintType)
struct FMGAIRacerSetup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkillLevel = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Aggression = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRival = false;
};

/**
 * Current race flow state
 */
UENUM(BlueprintType)
enum class EMGRaceFlowState : uint8
{
	/** No race active */
	Idle,
	/** Setting up race */
	Setup,
	/** Loading track */
	Loading,
	/** Pre-race grid/intro */
	PreRace,
	/** Countdown active */
	Countdown,
	/** Race in progress */
	Racing,
	/** Race finished, cooldown */
	Cooldown,
	/** Showing results */
	Results,
	/** Processing rewards */
	ProcessingRewards,
	/** Returning to garage */
	Returning,
	/** Error state */
	Error
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFlowStateChanged, EMGRaceFlowState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceSetupComplete, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceLoadProgress, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceFinished, const FMGRaceFlowResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRewardsProcessed, const FMGRaceFlowResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceError, const FString&, ErrorMessage);

/**
 * Race Flow Subsystem
 *
 * Orchestrates the entire race flow from garage to results:
 * - Receives race requests from garage/menu
 * - Handles track loading
 * - Spawns player and AI vehicles
 * - Connects to RaceGameMode for race execution
 * - Processes results and rewards
 * - Returns player to garage
 *
 * This is the main coordinator that connects all race systems for MVP.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGRaceFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMGRaceFlowSubsystem();

	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~ End USubsystem Interface

	// ==========================================
	// EVENTS
	// ==========================================

	/** Called when flow state changes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFlowStateChanged OnFlowStateChanged;

	/** Called when race setup completes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceSetupComplete OnRaceSetupComplete;

	/** Called during race loading */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceLoadProgress OnRaceLoadProgress;

	/** Called when race actually starts (after countdown) */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStarted OnRaceStarted;

	/** Called when race finishes */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceFinished OnRaceFinished;

	/** Called when rewards have been processed */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRewardsProcessed OnRewardsProcessed;

	/** Called on error */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceError OnRaceError;

	// ==========================================
	// RACE FLOW CONTROL
	// ==========================================

	/**
	 * Start race from garage
	 * Main entry point for starting any race
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	bool StartRace(const FMGRaceSetupRequest& Request);

	/**
	 * Start a quick race with default settings
	 * Convenience for testing/quick play
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	bool StartQuickRace(FName TrackID, FName VehicleID);

	/**
	 * Abort current race and return to garage
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void AbortRace();

	/**
	 * Restart current race with same settings
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void RestartRace();

	/**
	 * Continue to garage after viewing results
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void ContinueToGarage();

	/**
	 * Continue to next race (quick rematch)
	 */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow")
	void ContinueToNextRace();

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/** Get current flow state */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	EMGRaceFlowState GetFlowState() const { return CurrentState; }

	/** Is race active? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	bool IsRaceActive() const;

	/** Is loading? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	bool IsLoading() const { return CurrentState == EMGRaceFlowState::Loading; }

	/** Can start new race? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	bool CanStartRace() const;

	/** Get current race setup */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	FMGRaceSetupRequest GetCurrentSetup() const { return CurrentSetup; }

	/** Get last race result */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	FMGRaceFlowResult GetLastResult() const { return LastResult; }

	/** Get loading progress */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|State")
	float GetLoadingProgress() const { return LoadingProgress; }

	// ==========================================
	// TRACK DATA
	// ==========================================

	/** Get available tracks */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	TArray<FName> GetAvailableTracks() const;

	/** Get track display name */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	FText GetTrackDisplayName(FName TrackID) const;

	/** Get track preview image */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	UTexture2D* GetTrackPreview(FName TrackID) const;

	/** Is track unlocked? */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|Tracks")
	bool IsTrackUnlocked(FName TrackID) const;

	// ==========================================
	// AI SETUP
	// ==========================================

	/** Generate AI opponents for race */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow|AI")
	TArray<FMGAIRacerSetup> GenerateAIOpponents(int32 Count, float Difficulty, FName PlayerVehicleClass);

	/** Set AI opponents manually */
	UFUNCTION(BlueprintCallable, Category = "RaceFlow|AI")
	void SetAIOpponents(const TArray<FMGAIRacerSetup>& Opponents);

	// ==========================================
	// QUICK RACE PRESETS
	// ==========================================

	/** Get a quick race setup for testing */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetTestRaceSetup();

	/** Get sprint race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetSprintRacePreset(FName TrackID, FName VehicleID);

	/** Get circuit race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetCircuitRacePreset(FName TrackID, FName VehicleID, int32 Laps = 3);

	/** Get drift race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetDriftRacePreset(FName TrackID, FName VehicleID);

	/** Get drag race preset */
	UFUNCTION(BlueprintPure, Category = "RaceFlow|QuickRace")
	static FMGRaceSetupRequest GetDragRacePreset(FName VehicleID);

protected:
	// ==========================================
	// STATE
	// ==========================================

	/** Current flow state */
	EMGRaceFlowState CurrentState = EMGRaceFlowState::Idle;

	/** Current race setup */
	FMGRaceSetupRequest CurrentSetup;

	/** AI opponents for current race */
	TArray<FMGAIRacerSetup> CurrentAIOpponents;

	/** Last race result */
	FMGRaceFlowResult LastResult;

	/** Loading progress */
	float LoadingProgress = 0.0f;

	/** Error message if in error state */
	FString ErrorMessage;

	// ==========================================
	// CACHED SUBSYSTEMS
	// ==========================================

	UPROPERTY()
	TWeakObjectPtr<UMGGameStateSubsystem> GameStateSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGRaceDirectorSubsystem> RaceDirectorSubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGEconomySubsystem> EconomySubsystem;

	UPROPERTY()
	TWeakObjectPtr<UMGGarageSubsystem> GarageSubsystem;

	/** Cached race game mode (only valid during race) */
	UPROPERTY()
	TWeakObjectPtr<class AMGRaceGameMode> CachedRaceGameMode;

	// ==========================================
	// INTERNAL FLOW
	// ==========================================

	/** Set flow state */
	void SetFlowState(EMGRaceFlowState NewState);

	/** Setup phase */
	void ExecuteSetup();

	/** Loading phase */
	void ExecuteLoading();

	/** Pre-race phase */
	void ExecutePreRace();

	/** Start countdown */
	void ExecuteCountdown();

	/** Race phase */
	void ExecuteRacing();

	/** Cooldown phase */
	void ExecuteCooldown();

	/** Results phase */
	void ExecuteResults();

	/** Process rewards */
	void ExecuteRewardProcessing();

	/** Return to garage */
	void ExecuteReturn();

	/** Handle error */
	void HandleError(const FString& Error);

	// ==========================================
	// CALLBACKS
	// ==========================================

	/** Called when level loading completes */
	void OnLevelLoaded();

	/** Called when race game mode signals race start */
	void OnRaceGameModeStart();

	/** Called when race game mode signals race end */
	void OnRaceGameModeEnd();

	/** Calculate rewards based on result */
	void CalculateRewards(FMGRaceFlowResult& Result);

	/** Apply rewards to player progression */
	void ApplyRewards(const FMGRaceFlowResult& Result);

	// ==========================================
	// HELPERS
	// ==========================================

	/** Validate race setup */
	bool ValidateSetup(const FMGRaceSetupRequest& Request, FString& OutError);

	/** Get track level name from track ID */
	FName GetTrackLevelName(FName TrackID) const;

	/** Cache subsystem references */
	void CacheSubsystems();

	/** Bind to race game mode events */
	void BindRaceGameModeEvents();

	/** Unbind from race game mode events */
	void UnbindRaceGameModeEvents();

	/** Convert setup to game mode config */
	struct FMGRaceConfig ConvertSetupToConfig(const FMGRaceSetupRequest& Setup) const;

	/** Handle race game mode's race started event */
	UFUNCTION()
	void HandleRaceStarted();

	/** Handle race game mode's race finished event */
	UFUNCTION()
	void HandleRaceFinished(const struct FMGRaceResults& Results);
};

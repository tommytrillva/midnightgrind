// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGGameState.h - Multiplayer Race State (Replicated)
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file defines the "Game State" - the authoritative, server-controlled
 * data about the current race that ALL players need to see. Think of it as
 * the "scoreboard" and "race control" that everyone shares.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. GAME STATE (AGameStateBase):
 *    - A special Actor that exists in every multiplayer game
 *    - Spawned by the server, automatically replicated to all clients
 *    - Contains data that ALL players need to see (race phase, positions, etc.)
 *    - NOT the same as "game flow state" (that's UMGGameStateSubsystem)
 *
 *    CONFUSION WARNING: "GameState" in Unreal specifically means this
 *    replicated Actor class. Don't confuse it with general "state".
 *
 * 2. REPLICATION (Multiplayer):
 *    - Server is the "authority" - it owns the truth
 *    - Clients receive copies of replicated data automatically
 *    - UPROPERTY(Replicated) marks variables that sync to clients
 *    - ReplicatedUsing = OnRep_X calls a function when value changes on client
 *
 *    DATA FLOW:
 *    Server changes value -> Unreal replicates to clients -> OnRep_ fires on clients
 *
 * 3. SERVER AUTHORITY PATTERN:
 *    Functions prefixed with "Auth" (like AuthStartRace) should ONLY
 *    be called on the server. They modify authoritative game state.
 *    Clients call Server RPCs which eventually call these Auth functions.
 *
 * 4. RACE PHASES:
 *    The race progresses through phases:
 *    Lobby -> PreRace -> Countdown -> Racing -> Finishing -> Results -> PostRace
 *
 *    Each phase change is replicated so all players see the same state.
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *   [Server]
 *      |
 *      +-- [AMGGameState] (authority) <-- This file
 *      |        |
 *      |        +-- Race Phase, Positions, Settings
 *      |        +-- Timing (countdown, race time)
 *      |
 *      +-- [AMGPlayerState] (per-player, also replicated)
 *
 *   [Client A]     [Client B]     [Client C]
 *      |              |              |
 *      +-- Receives replicated GameState
 *      +-- Receives replicated PlayerStates
 *
 * IMPORTANT DISTINCTION:
 * - AMGGameState: REPLICATED, multiplayer race data (this file)
 * - UMGGameStateSubsystem: LOCAL ONLY, game flow (menus, loading, etc.)
 *
 * COMMON PATTERNS:
 * ----------------
 * @code
 * // Get the game state from any Actor
 * AMGGameState* GameState = GetWorld()->GetGameState<AMGGameState>();
 * if (GameState && GameState->IsRaceInProgress())
 * {
 *     TArray<FMGRacePositionEntry> Positions = GameState->GetPositions();
 *     float RaceTime = GameState->GetRaceTime();
 * }
 *
 * // On server only - start the race
 * if (HasAuthority())
 * {
 *     GameState->AuthStartRace();
 * }
 * @endcode
 *
 * EVENTS TO SUBSCRIBE TO:
 * - OnRacePhaseChanged: Race state transitions (use for UI updates)
 * - OnCountdownUpdate: Each second of countdown (show 3, 2, 1, GO!)
 * - OnRaceStart: When GO! happens (enable player input)
 * - OnRacerFinished: When any player finishes (update leaderboard)
 * - OnPositionsUpdated: Position changes (update race HUD)
 *
 * @see AMGPlayerState For individual player race data
 * @see UMGGameStateSubsystem For local (non-replicated) game flow
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MGGameState.generated.h"

class AMGPlayerState;

// ============================================================================
// RACE PHASE ENUM - Where are we in the race lifecycle?
// ============================================================================

/**
 * Global race phase visible to all clients
 *
 * This enum represents the "state machine" of a race session.
 * The server controls transitions; clients just observe.
 *
 * TYPICAL FLOW:
 * Lobby (waiting for players)
 *   -> PreRace (everyone loaded, preparing grid)
 *     -> Countdown (3, 2, 1...)
 *       -> Racing (main gameplay)
 *         -> Finishing (leader crossed finish, others still racing)
 *           -> Results (everyone done, showing standings)
 *             -> PostRace (returning to lobby or next race)
 */
UENUM(BlueprintType)
enum class EMGGlobalRacePhase : uint8
{
	/** Lobby - waiting for players */
	Lobby,
	/** All loaded, countdown starting */
	PreRace,
	/** Countdown in progress */
	Countdown,
	/** Race in progress */
	Racing,
	/** Race finishing - leader crossed line */
	Finishing,
	/** All finished, showing results */
	Results,
	/** Returning to lobby */
	PostRace
};

// ============================================================================
// RACE SETTINGS - Configuration for the current race session
// ============================================================================

/**
 * Race settings visible to all clients
 *
 * The host configures these settings in the lobby. Once set, they're
 * replicated to all players so everyone knows the race parameters.
 *
 * NOTE: These are read-only for clients. Only the server (host)
 * can modify these via AMGGameState::AuthSetRaceSettings().
 */
USTRUCT(BlueprintType)
struct FMGReplicatedRaceSettings
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	UPROPERTY(BlueprintReadOnly)
	FText TrackName;

	UPROPERTY(BlueprintReadOnly)
	int32 LapCount = 3;

	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 8;

	UPROPERTY(BlueprintReadOnly)
	bool bAllowAI = true;

	UPROPERTY(BlueprintReadOnly)
	int32 AICount = 0;

	UPROPERTY(BlueprintReadOnly)
	float AIDifficulty = 0.5f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsRanked = false;

	UPROPERTY(BlueprintReadOnly)
	FName GameModeID;

	UPROPERTY(BlueprintReadOnly)
	FText GameModeName;

	UPROPERTY(BlueprintReadOnly)
	bool bCollisionsEnabled = true;

	UPROPERTY(BlueprintReadOnly)
	int32 PerformanceIndexLimit = 0; // 0 = no limit

	UPROPERTY(BlueprintReadOnly)
	FString SessionPassword; // Empty = no password
};

// ============================================================================
// POSITION ENTRY - One racer's standing in the leaderboard
// ============================================================================

/**
 * Position entry for sorted leaderboard
 *
 * The Positions array in GameState contains one of these for each racer,
 * sorted by current race position. This provides everything needed to
 * display a race leaderboard or position indicator.
 */
USTRUCT(BlueprintType)
struct FMGRacePositionEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Position = 0;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AMGPlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 1;

	UPROPERTY(BlueprintReadOnly)
	float GapToLeader = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bHasFinished = false;

	UPROPERTY(BlueprintReadOnly)
	float FinishTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bIsAI = false;
};

// ============================================================================
// DELEGATES - Events broadcast when race state changes
// ============================================================================

/**
 * GameState Delegates
 *
 * These events fire on BOTH server and clients when race state changes.
 * This is because replicated properties trigger OnRep_ which then
 * broadcasts these delegates.
 *
 * USAGE PATTERN:
 * @code
 * void AMyHUD::BeginPlay()
 * {
 *     if (AMGGameState* GS = GetWorld()->GetGameState<AMGGameState>())
 *     {
 *         GS->OnRacePhaseChanged.AddDynamic(this, &AMyHUD::OnPhaseChanged);
 *         GS->OnCountdownUpdate.AddDynamic(this, &AMyHUD::ShowCountdown);
 *     }
 * }
 * @endcode
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGlobalRacePhaseChanged, EMGGlobalRacePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownUpdate, int32, SecondsRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRacerFinished, AMGPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllRacersFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPositionsUpdated, const TArray<FMGRacePositionEntry>&, Positions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBestLap, AMGPlayerState*, PlayerState, float, LapTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, AMGPlayerState*, PlayerState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, AMGPlayerState*, PlayerState);

// ============================================================================
// MAIN GAME STATE CLASS
// ============================================================================

/**
 * Game State - The authoritative, replicated race state
 *
 * This Actor is the "source of truth" for race information that all
 * players need to see. The server owns it; clients receive copies.
 *
 * FEATURES:
 * ---------
 * - Global race phase synchronization (everyone sees same phase)
 * - Position tracking and leaderboard
 * - Countdown synchronization (3, 2, 1, GO!)
 * - Race settings replication (track, laps, rules)
 * - Best lap tracking (who has the fastest lap?)
 *
 * KEY UNREAL CONCEPTS:
 * --------------------
 * - GetLifetimeReplicatedProps(): Defines which properties replicate
 * - ReplicatedUsing: Calls OnRep_ function when value changes on client
 * - AddPlayerState/RemovePlayerState: Called when players join/leave
 * - Tick(): Called every frame - updates race timing
 *
 * SERVER vs CLIENT:
 * -----------------
 * On SERVER: Call Auth* functions to change state
 * On CLIENT: Read-only, receive state via replication, react via events
 *
 * @see AGameStateBase The Unreal base class for game states
 */
UCLASS()
class MIDNIGHTGRIND_API AMGGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMGGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGlobalRacePhaseChanged OnRacePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCountdownUpdate OnCountdownUpdate;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRaceStart OnRaceStart;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRacerFinished OnRacerFinished;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAllRacersFinished OnAllRacersFinished;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPositionsUpdated OnPositionsUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnNewBestLap OnNewBestLap;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerLeft OnPlayerLeft;

	// ==========================================
	// RACE PHASE
	// ==========================================

	/** Get current race phase */
	UFUNCTION(BlueprintPure, Category = "Race")
	EMGGlobalRacePhase GetRacePhase() const { return CurrentRacePhase; }

	/** Is race in progress */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsRaceInProgress() const { return CurrentRacePhase == EMGGlobalRacePhase::Racing || CurrentRacePhase == EMGGlobalRacePhase::Finishing; }

	/** Is in lobby */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsInLobby() const { return CurrentRacePhase == EMGGlobalRacePhase::Lobby; }

	/** Is countdown active */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsCountdownActive() const { return CurrentRacePhase == EMGGlobalRacePhase::Countdown; }

	/** Get countdown time remaining */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetCountdownRemaining() const { return CountdownTime; }

	// ==========================================
	// RACE SETTINGS
	// ==========================================

	/** Get race settings */
	UFUNCTION(BlueprintPure, Category = "Race")
	FMGReplicatedRaceSettings GetRaceSettings() const { return RaceSettings; }

	/** Get track ID */
	UFUNCTION(BlueprintPure, Category = "Race")
	FName GetTrackID() const { return RaceSettings.TrackID; }

	/** Get lap count */
	UFUNCTION(BlueprintPure, Category = "Race")
	int32 GetLapCount() const { return RaceSettings.LapCount; }

	/** Is ranked match */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsRankedMatch() const { return RaceSettings.bIsRanked; }

	// ==========================================
	// RACE TIMING
	// ==========================================

	/** Get race elapsed time */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	float GetRaceTime() const { return RaceElapsedTime; }

	/** Get server time (for sync) */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	float GetServerTime() const { return GetServerWorldTimeSeconds(); }

	/** Get best overall lap time */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	float GetBestLapTime() const { return BestOverallLapTime; }

	/** Get best lap holder */
	UFUNCTION(BlueprintPure, Category = "Race|Timing")
	AMGPlayerState* GetBestLapHolder() const { return BestLapHolder; }

	// ==========================================
	// POSITIONS
	// ==========================================

	/** Get current positions */
	UFUNCTION(BlueprintPure, Category = "Race|Positions")
	TArray<FMGRacePositionEntry> GetPositions() const { return Positions; }

	/** Get position for player */
	UFUNCTION(BlueprintPure, Category = "Race|Positions")
	int32 GetPositionForPlayer(AMGPlayerState* PlayerState) const;

	/** Get leader */
	UFUNCTION(BlueprintPure, Category = "Race|Positions")
	AMGPlayerState* GetLeader() const;

	/** Get finished count */
	UFUNCTION(BlueprintPure, Category = "Race|Positions")
	int32 GetFinishedCount() const { return FinishedCount; }

	/** Get total racer count */
	UFUNCTION(BlueprintPure, Category = "Race|Positions")
	int32 GetTotalRacerCount() const { return TotalRacerCount; }

	// ==========================================
	// PLAYERS
	// ==========================================

	/** Get all MG player states */
	UFUNCTION(BlueprintPure, Category = "Players")
	TArray<AMGPlayerState*> GetMGPlayerStates() const;

	/** Get ready player count */
	UFUNCTION(BlueprintPure, Category = "Players")
	int32 GetReadyPlayerCount() const;

	/** Are all players ready */
	UFUNCTION(BlueprintPure, Category = "Players")
	bool AreAllPlayersReady() const;

	/** Get player count */
	UFUNCTION(BlueprintPure, Category = "Players")
	int32 GetPlayerCount() const { return PlayerArray.Num(); }

	/** Get session host */
	UFUNCTION(BlueprintPure, Category = "Players")
	AMGPlayerState* GetSessionHost() const;

	// ==========================================
	// SERVER FUNCTIONS (Authority only)
	// ==========================================

	/** Set race phase */
	void AuthSetRacePhase(EMGGlobalRacePhase NewPhase);

	/** Set race settings */
	void AuthSetRaceSettings(const FMGReplicatedRaceSettings& Settings);

	/** Start countdown */
	void AuthStartCountdown(float Duration = 3.0f);

	/** Start race */
	void AuthStartRace();

	/** Mark player finished */
	void AuthMarkPlayerFinished(AMGPlayerState* PlayerState, float FinishTime);

	/** Update positions */
	void AuthUpdatePositions(const TArray<FMGRacePositionEntry>& NewPositions);

	/** Report new best lap */
	void AuthReportBestLap(AMGPlayerState* PlayerState, float LapTime);

	/** End race */
	void AuthEndRace();

protected:
	// ==========================================
	// REPLICATED PROPERTIES
	// ==========================================

	/** Current race phase */
	UPROPERTY(ReplicatedUsing = OnRep_RacePhase, BlueprintReadOnly, Category = "Race")
	EMGGlobalRacePhase CurrentRacePhase = EMGGlobalRacePhase::Lobby;

	/** Race settings */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	FMGReplicatedRaceSettings RaceSettings;

	/** Countdown time remaining */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	float CountdownTime = 0.0f;

	/** Race elapsed time */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	float RaceElapsedTime = 0.0f;

	/** Race start server time */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	float RaceStartServerTime = 0.0f;

	/** Current positions */
	UPROPERTY(ReplicatedUsing = OnRep_Positions, BlueprintReadOnly, Category = "Race")
	TArray<FMGRacePositionEntry> Positions;

	/** Best overall lap time */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	float BestOverallLapTime = 0.0f;

	/** Best lap holder */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	TObjectPtr<AMGPlayerState> BestLapHolder;

	/** Finished racer count */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	int32 FinishedCount = 0;

	/** Total racer count */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	int32 TotalRacerCount = 0;

	// ==========================================
	// REP NOTIFIES
	// ==========================================

	UFUNCTION()
	void OnRep_RacePhase();

	UFUNCTION()
	void OnRep_Positions();

private:
	/** Previous countdown for tick detection */
	int32 PreviousCountdownTick = -1;

	/** Update countdown (server) */
	void TickCountdown(float DeltaSeconds);
};

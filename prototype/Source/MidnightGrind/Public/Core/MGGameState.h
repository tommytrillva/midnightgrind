// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGGameState.h
 * @brief Game State - Replicated race state visible to all connected clients
 *
 * The AMGGameState class manages the authoritative, replicated state of a race
 * session. Unlike UMGGameStateSubsystem (which handles local game flow), this
 * class specifically handles multiplayer race synchronization.
 *
 * @section Overview
 * GameState is spawned by the server and replicated to all clients. It contains
 * the "source of truth" for race phase, positions, timing, and settings that
 * all players need to see consistently.
 *
 * @section KeyConcepts Key Concepts
 * - **Race Phase**: The global phase of the race (Lobby, Countdown, Racing, etc.)
 *   All clients see the same phase, controlled by the server.
 * - **Positions**: The current race standings, updated by the server and replicated.
 * - **Race Settings**: Track, lap count, game mode, etc. - set by host, visible to all.
 * - **Timing**: Server-authoritative race time and countdown synchronization.
 *
 * @section Authority Server Authority
 * Only the server (Authority) can modify game state. All "Auth" prefixed functions
 * should only be called on the server. Clients receive updates via replication.
 *
 * @section Events
 * Subscribe to events for race state changes:
 * - OnRacePhaseChanged: When race transitions between phases
 * - OnCountdownUpdate: Each second tick during countdown
 * - OnRaceStart: When GO! happens
 * - OnRacerFinished: When any player crosses the finish line
 * - OnPositionsUpdated: When race positions change
 *
 * @section Usage
 * Access from any Actor:
 * @code
 * AMGGameState* GameState = GetWorld()->GetGameState<AMGGameState>();
 * if (GameState && GameState->IsRaceInProgress())
 * {
 *     TArray<FMGRacePositionEntry> Positions = GameState->GetPositions();
 *     float RaceTime = GameState->GetRaceTime();
 * }
 * @endcode
 *
 * @see AMGPlayerState For individual player race data
 * @see UMGGameStateSubsystem For local (non-replicated) game flow
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MGGameState.generated.h"

class AMGPlayerState;

/**
 * Global race phase visible to all clients
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

/**
 * Race settings visible to all clients
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

/**
 * Position entry for sorted leaderboard
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

/**
 * Delegates
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

/**
 * Game State
 * Replicated game state visible to all clients
 *
 * Features:
 * - Global race phase synchronization
 * - Position tracking and leaderboard
 * - Countdown synchronization
 * - Race settings replication
 * - Best lap tracking
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

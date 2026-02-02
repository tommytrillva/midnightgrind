// Copyright Midnight Grind. All Rights Reserved.


#pragma once
/**
 * =============================================================================
 * MG_PLYR_State.h - Per-Player Replicated Data
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * The Player State holds all the data about a specific player that other
 * players need to see. While GameState is "global race data", PlayerState
 * is "this player's race data".
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. PLAYER STATE (APlayerState):
 *    - One PlayerState per player in the game (including AI)
 *    - Replicated to all clients so everyone sees each other's data
 *    - Persists for the player's session (survives vehicle respawns)
 *    - Contains: name, score, race position, vehicle selection, etc.
 *
 *    IMPORTANT: PlayerState is REPLICATED (multiplayer data)
 *    Don't confuse with PlayerController (input handling, local only)
 *
 * 2. THE TRINITY OF PLAYER CLASSES:
 *    - PlayerController: Handles input, camera, local-only logic
 *    - PlayerState: Replicated data others see (name, score, position)
 *    - Pawn: The physical vehicle being controlled
 *
 *    Together they form a complete "player":
 *    Controller -> Controls -> Pawn (body)
 *    Controller -> Owns -> PlayerState (stats/identity)
 *
 * 3. CLIENT-SERVER FLOW:
 *    When a client wants to change their PlayerState:
 *
 *    [Client] "I want to toggle ready"
 *       |
 *       v (Server RPC)
 *    [Server] ServerToggleReady()
 *       |
 *       v (validates, then calls)
 *    AuthSetReadyState()
 *       |
 *       v (changes replicated property)
 *    ReadyState = Ready
 *       |
 *       v (automatic replication)
 *    [All Clients] receive new value
 *       |
 *       v (RepNotify)
 *    OnRep_ReadyState() fires
 *       |
 *       v (broadcasts event)
 *    OnReadyStateChanged delegate fires
 *
 * 4. RACE SNAPSHOT (FMGRaceSnapshot):
 *    A struct containing the player's current race performance:
 *    - Position, lap, checkpoint
 *    - Timing (total time, best lap, current lap)
 *    - Speed and nitro state
 *    Updated frequently and replicated to all clients.
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *   [AMGGameState]
 *         |
 *         +-- Contains array of all PlayerStates
 *         |
 *   [AMGPlayerState] x N players <-- This file
 *         |
 *         +-- Identity (name, platform ID, crew)
 *         +-- Ready state (for lobby)
 *         +-- Vehicle selection
 *         +-- Race performance (snapshot)
 *         +-- Finish position and lap times
 *
 * COMMON PATTERNS:
 * ----------------
 * @code
 * // Get your own PlayerState from PlayerController
 * AMGPlayerState* MyState = GetPlayerState<AMGPlayerState>();
 *
 * // Get all player states from GameState
 * AMGGameState* GS = GetWorld()->GetGameState<AMGGameState>();
 * TArray<AMGPlayerState*> AllPlayers = GS->GetMGPlayerStates();
 *
 * // Check if a specific player has finished
 * if (PlayerState->HasFinished())
 * {
 *     int32 FinalPosition = PlayerState->GetFinishPosition();
 *     float TotalTime = PlayerState->GetTotalRaceTime();
 * }
 *
 * // Client requesting to toggle ready state
 * if (!HasAuthority()) // Only clients call Server RPCs
 * {
 *     ServerToggleReady(); // This is a Server RPC
 * }
 * @endcode
 *
 * EVENTS TO SUBSCRIBE TO:
 * - OnReadyStateChanged: Player toggled ready in lobby
 * - OnRaceStatusChanged: Started racing, finished, DNF'd, etc.
 * - OnPositionChanged: Race position changed
 * - OnLapChanged: Completed a lap
 *
 * @see AMGGameState For global race state
 * @see AMGPlayerController For the controller that owns this state
 * @see FMGVehicleSelection For vehicle customization data
 * =============================================================================
 */

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MG_PLYR_State.generated.h"

class AMGVehiclePawn;

// ============================================================================
// READY STATE ENUM - Is the player ready to start?
// ============================================================================

/**
 * Player ready state for lobby
 *
 * In multiplayer lobbies, players must indicate they're ready before
 * the race can start. This enum tracks that state machine.
 *
 * FLOW: NotInLobby -> NotReady -> Ready -> Loading -> Loaded
 */
UENUM(BlueprintType)
enum class EMGPlayerReadyState : uint8
{
	/** Not in lobby */
	NotInLobby,
	/** In lobby, not ready */
	NotReady,
	/** Ready to race */
	Ready,
	/** Loading */
	Loading,
	/** Loaded and waiting */
	Loaded
};

// ============================================================================
// RACE STATUS ENUM - What is the player's current race state?
// ============================================================================

/**
 * Player race status
 *
 * Tracks what the player is currently doing in the race.
 * Used to determine UI display, input handling, and scoring.
 */
UENUM(BlueprintType)
enum class EMGPlayerRaceStatus : uint8
{
	/** Waiting for race to start */
	Waiting,
	/** Racing */
	Racing,
	/** Finished */
	Finished,
	/** Did not finish */
	DNF,
	/** Spectating */
	Spectating,
	/** Disconnected */
	Disconnected
};

// ============================================================================
// VEHICLE SELECTION - What car is the player using?
// ============================================================================

/**
 * Replicated vehicle selection
 *
 * When a player selects their vehicle in the lobby, this struct is
 * populated and replicated so other players can see what car they chose.
 * Used for displaying car previews in lobby and spawning the correct vehicle.
 */
USTRUCT(BlueprintType)
struct FMGVehicleSelection
{
	GENERATED_BODY()

	/// Identifier for the base vehicle definition
	UPROPERTY(BlueprintReadWrite, Category = "Vehicle")
	FName VehicleID;

	/// Identifier for the selected livery/wrap
	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	FName LiveryID;

	/// Primary body color
	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	FLinearColor PrimaryColor = FLinearColor::White;

	/// Secondary/accent color
	UPROPERTY(BlueprintReadWrite, Category = "Customization")
	FLinearColor SecondaryColor = FLinearColor::Black;

	/// Calculated performance index (PI) for matchmaking
	UPROPERTY(BlueprintReadWrite, Category = "Performance")
	int32 PerformanceIndex = 0;
};

// ============================================================================
// RACE SNAPSHOT - Current race performance data
// ============================================================================

/**
 * Race performance snapshot - replicated periodically
 *
 * This struct captures the player's current race performance at a point
 * in time. It's updated frequently on the server and replicated to clients.
 *
 * Used for:
 * - Race HUD (position, lap, time displays)
 * - Leaderboard updates
 * - Race director/camera logic
 * - Commentary/narration systems
 */
USTRUCT(BlueprintType)
struct FMGRaceSnapshot
{
	GENERATED_BODY()

	/// Current race position (1 = first)
	UPROPERTY(BlueprintReadOnly, Category = "Position")
	int32 Position = 0;

	/// Current lap number (1-indexed)
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 CurrentLap = 1;

	/// Index of the last checkpoint passed
	UPROPERTY(BlueprintReadOnly, Category = "Progress")
	int32 LastCheckpoint = 0;

	/// Total elapsed race time in seconds
	UPROPERTY(BlueprintReadOnly, Category = "Timing")
	float TotalTime = 0.0f;

	/// Best lap time achieved this race (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Timing")
	float BestLapTime = 0.0f;

	/// Current lap elapsed time (seconds)
	UPROPERTY(BlueprintReadOnly, Category = "Timing")
	float CurrentLapTime = 0.0f;

	/// Distance behind the race leader in meters
	UPROPERTY(BlueprintReadOnly, Category = "Position")
	float DistanceFromLeader = 0.0f;

	/// Current vehicle speed
	UPROPERTY(BlueprintReadOnly, Category = "Vehicle")
	float CurrentSpeed = 0.0f;

	/// Whether nitro boost is currently active
	UPROPERTY(BlueprintReadOnly, Category = "Nitro")
	bool bUsingNitro = false;

	/// Remaining nitro amount (0.0 to 1.0)
	UPROPERTY(BlueprintReadOnly, Category = "Nitro")
	float NitroAmount = 1.0f;
};

// ============================================================================
// DELEGATES - Events broadcast when player state changes
// ============================================================================

/**
 * PlayerState Delegates
 *
 * These fire on both server and clients when this player's state changes.
 * Useful for updating UI elements that show other players' status.
 *
 * NOTE: The first parameter is the PlayerState itself, allowing handlers
 * to identify WHICH player changed (important when tracking multiple players).
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerReadyStateChanged, AMGPlayerState*, PlayerState, EMGPlayerReadyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRaceStatusChanged, AMGPlayerState*, PlayerState, EMGPlayerRaceStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPositionChanged, AMGPlayerState*, PlayerState, int32, NewPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLapChanged, AMGPlayerState*, PlayerState, int32, NewLap);

// ============================================================================
// MAIN PLAYER STATE CLASS
// ============================================================================

/**
 * Player State - Replicated per-player data visible to all
 *
 * Each player (and AI) in the game has a PlayerState that holds their
 * identity and race performance. This data is replicated so all players
 * can see each other's names, positions, vehicles, etc.
 *
 * FEATURES:
 * ---------
 * - Race position/lap tracking (where are they in the race?)
 * - Vehicle selection replication (what car did they choose?)
 * - Ready state for lobbies (are they ready to start?)
 * - Performance stats (lap times, finish position)
 *
 * KEY FUNCTIONS:
 * --------------
 * Read functions (client-safe):
 * - GetRacePosition(), GetCurrentLap(), GetBestLapTime()
 * - GetReadyState(), IsReady(), HasFinished()
 * - GetVehicleSelection(), GetDisplayName()
 *
 * Server RPCs (client calls, server executes):
 * - ServerToggleReady() - Request ready state change
 * - ServerSelectVehicle() - Request vehicle selection
 *
 * Auth functions (server-only, called by GameMode):
 * - AuthUpdateRaceSnapshot() - Update position/lap/time
 * - AuthSetRaceStatus() - Change racing/finished/DNF
 * - AuthRecordLapTime() - Save completed lap time
 *
 * @see APlayerState The Unreal base class
 * @see AMGGameState For global race state
 */
UCLASS()
class MIDNIGHTGRIND_API AMGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMGPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerReadyStateChanged OnReadyStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerRaceStatusChanged OnRaceStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerPositionChanged OnPositionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerLapChanged OnLapChanged;

	// ==========================================
	// IDENTITY
	// ==========================================

	/** Get Steam/platform ID */
	UFUNCTION(BlueprintPure, Category = "Identity")
	FString GetPlatformID() const { return PlatformID; }

	/** Get display name */
	UFUNCTION(BlueprintPure, Category = "Identity")
	FString GetDisplayName() const { return GetPlayerName(); }

	/** Get profile level */
	UFUNCTION(BlueprintPure, Category = "Identity")
	int32 GetProfileLevel() const { return ProfileLevel; }

	/** Get crew/club name */
	UFUNCTION(BlueprintPure, Category = "Identity")
	FString GetCrewName() const { return CrewName; }

	/** Is host of current session */
	UFUNCTION(BlueprintPure, Category = "Identity")
	bool IsSessionHost() const { return bIsSessionHost; }

	// ==========================================
	// LOBBY STATE
	// ==========================================

	/** Get ready state */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	EMGPlayerReadyState GetReadyState() const { return ReadyState; }

	/** Set ready state (server) */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReadyState(EMGPlayerReadyState NewState);

	/** Is ready */
	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsReady() const { return ReadyState == EMGPlayerReadyState::Ready || ReadyState == EMGPlayerReadyState::Loaded; }

	/** Server RPC to toggle ready */
	UFUNCTION(Server, Reliable, Category = "Lobby")
	void ServerToggleReady();

	// ==========================================
	// VEHICLE SELECTION
	// ==========================================

	/** Get selected vehicle */
	UFUNCTION(BlueprintPure, Category = "Vehicle")
	FMGVehicleSelection GetVehicleSelection() const { return VehicleSelection; }

	/** Set vehicle selection (local -> server) */
	UFUNCTION(BlueprintCallable, Category = "Vehicle")
	void SelectVehicle(const FMGVehicleSelection& Selection);

	/** Server RPC to update vehicle selection */
	UFUNCTION(Server, Reliable, Category = "Vehicle")
	void ServerSelectVehicle(const FMGVehicleSelection& Selection);

	// ==========================================
	// RACE STATE
	// ==========================================

	/** Get race status */
	UFUNCTION(BlueprintPure, Category = "Race")
	EMGPlayerRaceStatus GetRaceStatus() const { return RaceStatus; }

	/** Get current position */
	UFUNCTION(BlueprintPure, Category = "Race")
	int32 GetRacePosition() const { return RaceSnapshot.Position; }

	/** Get current lap */
	UFUNCTION(BlueprintPure, Category = "Race")
	int32 GetCurrentLap() const { return RaceSnapshot.CurrentLap; }

	/** Get race snapshot */
	UFUNCTION(BlueprintPure, Category = "Race")
	FMGRaceSnapshot GetRaceSnapshot() const { return RaceSnapshot; }

	/** Get best lap time */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetBestLapTime() const { return RaceSnapshot.BestLapTime; }

	/** Get total race time */
	UFUNCTION(BlueprintPure, Category = "Race")
	float GetTotalRaceTime() const { return RaceSnapshot.TotalTime; }

	/** Get all lap times */
	UFUNCTION(BlueprintPure, Category = "Race")
	TArray<float> GetLapTimes() const { return LapTimes; }

	/** Get finish position (0 if not finished) */
	UFUNCTION(BlueprintPure, Category = "Race")
	int32 GetFinishPosition() const { return FinishPosition; }

	/** Has finished race */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool HasFinished() const { return RaceStatus == EMGPlayerRaceStatus::Finished; }

	/** Is DNF */
	UFUNCTION(BlueprintPure, Category = "Race")
	bool IsDNF() const { return RaceStatus == EMGPlayerRaceStatus::DNF; }

	// ==========================================
	// SERVER FUNCTIONS (Authority only)
	// ==========================================

	/** Update race snapshot (called by game mode) */
	void AuthUpdateRaceSnapshot(const FMGRaceSnapshot& NewSnapshot);

	/** Set race status */
	void AuthSetRaceStatus(EMGPlayerRaceStatus NewStatus);

	/** Record lap time */
	void AuthRecordLapTime(float LapTime);

	/** Set finish position */
	void AuthSetFinishPosition(int32 Position);

	/** Set as session host */
	void AuthSetSessionHost(bool bIsHost);

	/** Initialize from platform data */
	void AuthInitializeFromPlatform(const FString& InPlatformID, const FString& InDisplayName, int32 InLevel, const FString& InCrew);

protected:
	// ==========================================
	// REPLICATED PROPERTIES
	// ==========================================

	/** Platform-specific ID (Steam ID, etc) */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Identity")
	FString PlatformID;

	/** Profile level */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Identity")
	int32 ProfileLevel = 1;

	/** Crew/club name */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Identity")
	FString CrewName;

	/** Is session host */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Identity")
	bool bIsSessionHost = false;

	/** Ready state */
	UPROPERTY(ReplicatedUsing = OnRep_ReadyState, BlueprintReadOnly, Category = "Lobby")
	EMGPlayerReadyState ReadyState = EMGPlayerReadyState::NotInLobby;

	/** Vehicle selection */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Vehicle")
	FMGVehicleSelection VehicleSelection;

	/** Race status */
	UPROPERTY(ReplicatedUsing = OnRep_RaceStatus, BlueprintReadOnly, Category = "Race")
	EMGPlayerRaceStatus RaceStatus = EMGPlayerRaceStatus::Waiting;

	/** Race snapshot */
	UPROPERTY(ReplicatedUsing = OnRep_RaceSnapshot, BlueprintReadOnly, Category = "Race")
	FMGRaceSnapshot RaceSnapshot;

	/** All lap times */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	TArray<float> LapTimes;

	/** Final finish position */
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Race")
	int32 FinishPosition = 0;

	// ==========================================
	// REP NOTIFIES
	// ==========================================

	UFUNCTION()
	void OnRep_ReadyState();

	UFUNCTION()
	void OnRep_RaceStatus();

	UFUNCTION()
	void OnRep_RaceSnapshot();

private:
	/** Previous position for change detection */
	int32 PreviousPosition = 0;

	/** Previous lap for change detection */
	int32 PreviousLap = 0;
};

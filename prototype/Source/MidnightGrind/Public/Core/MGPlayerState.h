// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPlayerState.h
 * @brief Player State - Replicated per-player data visible to all clients
 *
 * AMGPlayerState stores and replicates individual player information during
 * a multiplayer session. Each connected player (and AI racer) has a PlayerState
 * that tracks their identity, vehicle selection, ready state, and race performance.
 *
 * @section Overview
 * PlayerState is the multiplayer-aware representation of a player. It's spawned
 * when a player joins and persists throughout their session. All players can see
 * each other's PlayerState data (names, positions, vehicles, etc.).
 *
 * @section KeyData Key Data Tracked
 * - **Identity**: Platform ID, display name, profile level, crew/club membership
 * - **Lobby State**: Ready/not ready status for pre-race coordination
 * - **Vehicle Selection**: Which car and livery the player has selected
 * - **Race Performance**: Position, lap, checkpoint, lap times, finish status
 *
 * @section Replication Replication Flow
 * - Clients call Server RPCs (ServerToggleReady, ServerSelectVehicle) to request changes
 * - Server validates and applies changes via "Auth" functions
 * - Changes replicate automatically to all clients via RepNotify
 * - Clients receive OnRep_ callbacks and broadcast events for UI updates
 *
 * @section Events
 * Subscribe to track player changes:
 * - OnReadyStateChanged: Player toggled ready in lobby
 * - OnRaceStatusChanged: Player started racing, finished, DNF'd, etc.
 * - OnPositionChanged: Player's race position changed
 * - OnLapChanged: Player completed a lap
 *
 * @section Usage
 * Get a player's state:
 * @code
 * // From a PlayerController
 * AMGPlayerState* MyState = GetPlayerState<AMGPlayerState>();
 *
 * // Get all player states from GameState
 * AMGGameState* GS = GetWorld()->GetGameState<AMGGameState>();
 * TArray<AMGPlayerState*> AllPlayers = GS->GetMGPlayerStates();
 *
 * // Check race status
 * if (PlayerState->HasFinished())
 * {
 *     int32 FinalPosition = PlayerState->GetFinishPosition();
 *     float TotalTime = PlayerState->GetTotalRaceTime();
 * }
 * @endcode
 *
 * @see AMGGameState For global race state
 * @see FMGVehicleSelection For vehicle customization data
 * @see FMGRaceSnapshot For detailed race performance data
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MGPlayerState.generated.h"

class AMGVehiclePawn;

/**
 * Player ready state for lobby
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

/**
 * Player race status
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

/**
 * Replicated vehicle selection
 */
USTRUCT(BlueprintType)
struct FMGVehicleSelection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName VehicleID;

	UPROPERTY(BlueprintReadWrite)
	FName LiveryID;

	UPROPERTY(BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Black;

	UPROPERTY(BlueprintReadWrite)
	int32 PerformanceIndex = 0;
};

/**
 * Race performance snapshot - replicated periodically
 */
USTRUCT(BlueprintType)
struct FMGRaceSnapshot
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Position = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 LastCheckpoint = 0;

	UPROPERTY(BlueprintReadOnly)
	float TotalTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float BestLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentLapTime = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float DistanceFromLeader = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	float CurrentSpeed = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bUsingNitro = false;

	UPROPERTY(BlueprintReadOnly)
	float NitroAmount = 1.0f;
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerReadyStateChanged, AMGPlayerState*, PlayerState, EMGPlayerReadyState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRaceStatusChanged, AMGPlayerState*, PlayerState, EMGPlayerRaceStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerPositionChanged, AMGPlayerState*, PlayerState, int32, NewPosition);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLapChanged, AMGPlayerState*, PlayerState, int32, NewLap);

/**
 * Player State
 * Replicated player data visible to all clients
 *
 * Features:
 * - Race position/lap tracking
 * - Vehicle selection replication
 * - Ready state for lobbies
 * - Performance stats
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

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSessionManager.generated.h"

/**
 * Session state
 */
UENUM(BlueprintType)
enum class EMGSessionState : uint8
{
	/** Not connected to any session */
	Idle,
	/** Searching for session */
	Searching,
	/** Creating session */
	Creating,
	/** Joining session */
	Joining,
	/** In lobby waiting for race */
	InLobby,
	/** Loading race */
	Loading,
	/** Racing */
	InRace,
	/** Post-race results */
	PostRace,
	/** Disconnecting */
	Disconnecting,
	/** Error state */
	Error
};

/**
 * Session type
 */
UENUM(BlueprintType)
enum class EMGSessionType : uint8
{
	/** Quick match */
	QuickMatch,
	/** Ranked competitive */
	Ranked,
	/** Private lobby */
	Private,
	/** Crew session */
	Crew,
	/** Tournament match */
	Tournament
};

/**
 * Matchmaking preferences
 */
USTRUCT(BlueprintType)
struct FMGMatchmakingPrefs
{
	GENERATED_BODY()

	/** Session type */
	UPROPERTY(BlueprintReadWrite)
	EMGSessionType SessionType = EMGSessionType::QuickMatch;

	/** Preferred race type */
	UPROPERTY(BlueprintReadWrite)
	FName RaceType;

	/** Preferred track (empty = any) */
	UPROPERTY(BlueprintReadWrite)
	FName PreferredTrack;

	/** Vehicle class restriction (empty = any) */
	UPROPERTY(BlueprintReadWrite)
	FName VehicleClass;

	/** Max ping allowed */
	UPROPERTY(BlueprintReadWrite)
	int32 MaxPing = 150;

	/** Include rivals in matchmaking */
	UPROPERTY(BlueprintReadWrite)
	bool bIncludeRivals = true;

	/** Region preference (empty = auto) */
	UPROPERTY(BlueprintReadWrite)
	FString Region;

	/** Lap count preference (0 = host decides) */
	UPROPERTY(BlueprintReadWrite)
	int32 PreferredLaps = 0;

	/** Allow pink slip races */
	UPROPERTY(BlueprintReadWrite)
	bool bAllowPinkSlip = false;
};

/**
 * Player in session
 */
USTRUCT(BlueprintType)
struct FMGSessionPlayer
{
	GENERATED_BODY()

	/** Unique player ID */
	UPROPERTY(BlueprintReadOnly)
	FString PlayerID;

	/** Display name */
	UPROPERTY(BlueprintReadOnly)
	FText DisplayName;

	/** Player level */
	UPROPERTY(BlueprintReadOnly)
	int32 Level = 1;

	/** Ranked rating (if applicable) */
	UPROPERTY(BlueprintReadOnly)
	int32 Rating = 0;

	/** Crew tag */
	UPROPERTY(BlueprintReadOnly)
	FString CrewTag;

	/** Selected vehicle */
	UPROPERTY(BlueprintReadOnly)
	FName SelectedVehicle;

	/** Vehicle performance index */
	UPROPERTY(BlueprintReadOnly)
	int32 VehiclePI = 0;

	/** Is ready */
	UPROPERTY(BlueprintReadOnly)
	bool bReady = false;

	/** Is host */
	UPROPERTY(BlueprintReadOnly)
	bool bIsHost = false;

	/** Ping to server */
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;

	/** Is local player */
	UPROPERTY(BlueprintReadOnly)
	bool bIsLocal = false;
};

/**
 * Session info
 */
USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	/** Session ID */
	UPROPERTY(BlueprintReadOnly)
	FString SessionID;

	/** Session type */
	UPROPERTY(BlueprintReadOnly)
	EMGSessionType Type = EMGSessionType::QuickMatch;

	/** Host player ID */
	UPROPERTY(BlueprintReadOnly)
	FString HostID;

	/** Current track */
	UPROPERTY(BlueprintReadOnly)
	FName TrackID;

	/** Race type */
	UPROPERTY(BlueprintReadOnly)
	FName RaceType;

	/** Lap count */
	UPROPERTY(BlueprintReadOnly)
	int32 LapCount = 3;

	/** Vehicle class restriction */
	UPROPERTY(BlueprintReadOnly)
	FName VehicleClass;

	/** Max players */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxPlayers = 8;

	/** Current players */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGSessionPlayer> Players;

	/** Is pink slip enabled */
	UPROPERTY(BlueprintReadOnly)
	bool bPinkSlip = false;

	/** Is ranked */
	UPROPERTY(BlueprintReadOnly)
	bool bRanked = false;

	/** Region */
	UPROPERTY(BlueprintReadOnly)
	FString Region;

	/** Countdown active */
	UPROPERTY(BlueprintReadOnly)
	bool bCountdownActive = false;

	/** Countdown remaining */
	UPROPERTY(BlueprintReadOnly)
	float CountdownRemaining = 0.0f;

	/** Get player count */
	int32 GetPlayerCount() const { return Players.Num(); }

	/** Is full */
	bool IsFull() const { return Players.Num() >= MaxPlayers; }
};

/**
 * Session search result
 */
USTRUCT(BlueprintType)
struct FMGSessionSearchResult
{
	GENERATED_BODY()

	/** Session info */
	UPROPERTY(BlueprintReadOnly)
	FMGSessionInfo SessionInfo;

	/** Average ping */
	UPROPERTY(BlueprintReadOnly)
	int32 Ping = 0;

	/** Match quality (0-100) */
	UPROPERTY(BlueprintReadOnly)
	int32 MatchQuality = 0;

	/** Contains rival */
	UPROPERTY(BlueprintReadOnly)
	bool bHasRival = false;
};

/**
 * Delegate declarations
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionStateChanged, EMGSessionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionCreated, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionJoined, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionUpdated, const FMGSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerJoined, const FMGSessionPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeft, const FMGSessionPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerReady, const FMGSessionPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSearchResults, const TArray<FMGSessionSearchResult>&, Results);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownStarted, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountdownCancelled);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStarting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionError, const FText&, ErrorMessage);

/**
 * Session Manager
 * Handles multiplayer session creation, matchmaking, and lobby management
 *
 * Features:
 * - Quick match and ranked matchmaking
 * - Private lobby creation
 * - Crew/friend sessions
 * - Lobby management (ready, vehicle select)
 * - Host migration
 * - Session persistence and reconnection
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// SESSION CREATION
	// ==========================================

	/**
	 * Create a new session
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool CreateSession(EMGSessionType Type, FName TrackID, int32 LapCount, FName VehicleClass, int32 MaxPlayers = 8);

	/**
	 * Create private lobby
	 */
	UFUNCTION(BlueprintCallable, Category = "Session")
	bool CreatePrivateLobby(int32 MaxPlayers = 8);

	// ==========================================
	// MATCHMAKING
	// ==========================================

	/**
	 * Start matchmaking with preferences
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool StartMatchmaking(const FMGMatchmakingPrefs& Preferences);

	/**
	 * Cancel matchmaking
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

	/**
	 * Search for sessions
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void SearchSessions(const FMGMatchmakingPrefs& Filters);

	/**
	 * Join session by ID
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool JoinSession(const FString& SessionID);

	/**
	 * Join session from search result
	 */
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool JoinSessionFromResult(const FMGSessionSearchResult& Result);

	/**
	 * Get matchmaking time
	 */
	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	float GetMatchmakingTime() const { return MatchmakingTime; }

	// ==========================================
	// LOBBY MANAGEMENT
	// ==========================================

	/**
	 * Set local player ready
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bReady);

	/**
	 * Set selected vehicle
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetSelectedVehicle(FName VehicleID, int32 PerformanceIndex);

	/**
	 * Leave current session
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void LeaveSession();

	/**
	 * Kick player (host only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool KickPlayer(const FString& PlayerID);

	/**
	 * Change track (host only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool ChangeTrack(FName NewTrackID);

	/**
	 * Change lap count (host only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool ChangeLapCount(int32 NewLapCount);

	/**
	 * Start countdown (host only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	bool StartCountdown();

	/**
	 * Cancel countdown (host only)
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void CancelCountdown();

	/**
	 * Invite friend
	 */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void InvitePlayer(const FString& PlayerID);

	// ==========================================
	// STATE QUERIES
	// ==========================================

	/**
	 * Get current session state
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	EMGSessionState GetSessionState() const { return CurrentState; }

	/**
	 * Get current session info
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	const FMGSessionInfo& GetCurrentSession() const { return CurrentSession; }

	/**
	 * Is in session
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool IsInSession() const;

	/**
	 * Is host
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool IsHost() const;

	/**
	 * Is local player ready
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool IsLocalPlayerReady() const;

	/**
	 * Are all players ready
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool AreAllPlayersReady() const;

	/**
	 * Get local player
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	FMGSessionPlayer GetLocalPlayer() const;

	/**
	 * Can start race (host check)
	 */
	UFUNCTION(BlueprintPure, Category = "Session|State")
	bool CanStartRace() const;

	// ==========================================
	// EVENTS
	// ==========================================

	/** Session state changed */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionStateChanged OnSessionStateChanged;

	/** Session created */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionCreated OnSessionCreated;

	/** Session joined */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionJoined OnSessionJoined;

	/** Session updated */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionUpdated OnSessionUpdated;

	/** Player joined */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnPlayerJoined OnPlayerJoined;

	/** Player left */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnPlayerLeft OnPlayerLeft;

	/** Player ready state changed */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnPlayerReady OnPlayerReady;

	/** Search results received */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSearchResults OnSearchResults;

	/** Countdown started */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnCountdownStarted OnCountdownStarted;

	/** Countdown cancelled */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnCountdownCancelled OnCountdownCancelled;

	/** Race is starting (load level) */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnRaceStarting OnRaceStarting;

	/** Session error */
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FOnSessionError OnSessionError;

protected:
	// ==========================================
	// INTERNAL
	// ==========================================

	/** Set session state */
	void SetSessionState(EMGSessionState NewState);

	/** Handle matchmaking tick */
	void TickMatchmaking(float DeltaTime);

	/** Handle countdown tick */
	void TickCountdown(float DeltaTime);

	/** Generate session ID */
	FString GenerateSessionID() const;

	/** Create local player data */
	FMGSessionPlayer CreateLocalPlayerData() const;

	/** Find best session from search results */
	int32 FindBestSession(const TArray<FMGSessionSearchResult>& Results) const;

	/** Handle host migration */
	void HandleHostMigration();

	/** Timer callback */
	void OnTick();

private:
	/** Current state */
	EMGSessionState CurrentState = EMGSessionState::Idle;

	/** Current session info */
	FMGSessionInfo CurrentSession;

	/** Matchmaking preferences */
	FMGMatchmakingPrefs CurrentMatchmakingPrefs;

	/** Matchmaking elapsed time */
	float MatchmakingTime = 0.0f;

	/** Countdown remaining */
	float CountdownRemaining = 0.0f;

	/** Is countdown active */
	bool bCountdownActive = false;

	/** Local player ID */
	FString LocalPlayerID;

	/** Timer handle */
	FTimerHandle TickTimer;

	/** Countdown duration */
	static constexpr float DefaultCountdownDuration = 10.0f;

	/** Max matchmaking time before expanding search */
	static constexpr float MatchmakingExpandTime = 30.0f;
};

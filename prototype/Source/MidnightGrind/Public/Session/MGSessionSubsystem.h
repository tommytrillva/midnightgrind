// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSessionSubsystem.generated.h"

/**
 * Session System - Online Multiplayer Management
 * - Session creation and discovery
 * - Matchmaking with skill-based matching
 * - Lobby management
 * - Friend invites and party system
 * - Quick join and playlist queuing
 */

UENUM(BlueprintType)
enum class EMGSessionState : uint8
{
	None,
	Creating,
	InLobby,
	Starting,
	InProgress,
	Finishing,
	PostRace
};

UENUM(BlueprintType)
enum class EMGMatchmakingState : uint8
{
	Idle,
	Searching,
	Found,
	Joining,
	Failed,
	Cancelled
};

UENUM(BlueprintType)
enum class EMGLobbyPrivacy : uint8
{
	Public,          // Anyone can join
	FriendsOnly,     // Only friends can join
	InviteOnly,      // Requires invite
	Closed           // No one can join
};

USTRUCT(BlueprintType)
struct FMGSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText HostDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSessionState State = EMGSessionState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLobbyPrivacy Privacy = EMGLobbyPrivacy::Public;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CurrentTrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AverageSkillRating = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Region;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRanked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFull = false;
};

USTRUCT(BlueprintType)
struct FMGLobbyPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillRating = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SelectedVehicle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VehiclePI = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsFriend = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrewMember = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamIndex = -1;
};

USTRUCT(BlueprintType)
struct FMGMatchmakingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlaylistID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRankedOnly = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPingMS = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCrossplay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkillRange = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SearchTimeout = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExpandSearchOverTime = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> PreferredRegions;
};

USTRUCT(BlueprintType)
struct FMGPartyMember
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLeader = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsInGame = false;
};

USTRUCT(BlueprintType)
struct FMGPartyInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PartyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGPartyMember> Members;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSize = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPublic = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSessionStateChanged, EMGSessionState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMatchmakingStateChanged, EMGMatchmakingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSessionFound, const FMGSessionInfo&, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerJoined, const FMGLobbyPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPlayerLeft, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMGOnAllPlayersReady);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnPartyUpdated, const FMGPartyInfo&, Party);

UCLASS()
class MIDNIGHTGRIND_API UMGSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Session Management
	UFUNCTION(BlueprintCallable, Category = "Session")
	void CreateSession(const FMGSessionInfo& Settings);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void JoinSession(const FString& SessionID);

	UFUNCTION(BlueprintCallable, Category = "Session")
	void LeaveSession();

	UFUNCTION(BlueprintPure, Category = "Session")
	FMGSessionInfo GetCurrentSession() const { return CurrentSession; }

	UFUNCTION(BlueprintPure, Category = "Session")
	EMGSessionState GetSessionState() const { return CurrentSession.State; }

	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsInSession() const { return CurrentSession.State != EMGSessionState::None; }

	UFUNCTION(BlueprintPure, Category = "Session")
	bool IsHost() const { return bIsHost; }

	// Matchmaking
	UFUNCTION(BlueprintCallable, Category = "Session|Matchmaking")
	void StartMatchmaking(const FMGMatchmakingSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "Session|Matchmaking")
	void CancelMatchmaking();

	UFUNCTION(BlueprintPure, Category = "Session|Matchmaking")
	EMGMatchmakingState GetMatchmakingState() const { return MatchmakingState; }

	UFUNCTION(BlueprintPure, Category = "Session|Matchmaking")
	float GetMatchmakingTime() const { return MatchmakingTime; }

	UFUNCTION(BlueprintPure, Category = "Session|Matchmaking")
	int32 GetPlayersInQueue() const { return PlayersInQueue; }

	// Quick Play
	UFUNCTION(BlueprintCallable, Category = "Session|QuickPlay")
	void QuickPlay(FName PlaylistID);

	UFUNCTION(BlueprintCallable, Category = "Session|QuickPlay")
	void QuickPlayRanked();

	// Lobby Management
	UFUNCTION(BlueprintPure, Category = "Session|Lobby")
	TArray<FMGLobbyPlayer> GetLobbyPlayers() const { return LobbyPlayers; }

	UFUNCTION(BlueprintCallable, Category = "Session|Lobby")
	void SetReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Session|Lobby")
	void SetVehicle(FName VehicleID, int32 PI);

	UFUNCTION(BlueprintCallable, Category = "Session|Lobby")
	void SetTeam(int32 TeamIndex);

	UFUNCTION(BlueprintPure, Category = "Session|Lobby")
	bool AreAllPlayersReady() const;

	UFUNCTION(BlueprintPure, Category = "Session|Lobby")
	int32 GetReadyPlayerCount() const;

	// Host Controls
	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void SetTrack(FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void SetGameMode(FName ModeID);

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void SetPrivacy(EMGLobbyPrivacy Privacy);

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void KickPlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void StartRace();

	UFUNCTION(BlueprintCallable, Category = "Session|Host")
	void RandomizeTrack();

	// Party System
	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void CreateParty();

	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void JoinParty(const FString& PartyID);

	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void LeaveParty();

	UFUNCTION(BlueprintCallable, Category = "Session|Party")
	void InviteToParty(const FString& PlayerID);

	UFUNCTION(BlueprintPure, Category = "Session|Party")
	FMGPartyInfo GetPartyInfo() const { return CurrentParty; }

	UFUNCTION(BlueprintPure, Category = "Session|Party")
	bool IsInParty() const { return !CurrentParty.PartyID.IsEmpty(); }

	UFUNCTION(BlueprintPure, Category = "Session|Party")
	bool IsPartyLeader() const;

	// Invites
	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void InvitePlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void InviteFriends();

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void InviteCrew();

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void AcceptInvite(const FString& InviteID);

	UFUNCTION(BlueprintCallable, Category = "Session|Invite")
	void DeclineInvite(const FString& InviteID);

	// Server Browser
	UFUNCTION(BlueprintCallable, Category = "Session|Browser")
	void RefreshSessionList();

	UFUNCTION(BlueprintPure, Category = "Session|Browser")
	TArray<FMGSessionInfo> GetAvailableSessions() const { return AvailableSessions; }

	UFUNCTION(BlueprintCallable, Category = "Session|Browser")
	void FilterSessions(FName GameModeFilter, bool bHideFullSessions, int32 MaxPing);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnSessionStateChanged OnSessionStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnMatchmakingStateChanged OnMatchmakingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnSessionFound OnSessionFound;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnPlayerJoined OnPlayerJoined;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnPlayerLeft OnPlayerLeft;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnAllPlayersReady OnAllPlayersReady;

	UPROPERTY(BlueprintAssignable, Category = "Session|Events")
	FMGOnPartyUpdated OnPartyUpdated;

protected:
	void UpdateMatchmaking(float DeltaTime);
	void SetSessionState(EMGSessionState NewState);
	void SetMatchmakingState(EMGMatchmakingState NewState);
	void SimulateMatchFound();

private:
	FMGSessionInfo CurrentSession;
	FMGPartyInfo CurrentParty;
	FMGMatchmakingSettings CurrentMatchmakingSettings;
	TArray<FMGLobbyPlayer> LobbyPlayers;
	TArray<FMGSessionInfo> AvailableSessions;
	EMGMatchmakingState MatchmakingState = EMGMatchmakingState::Idle;
	FTimerHandle MatchmakingTimerHandle;
	float MatchmakingTime = 0.0f;
	int32 PlayersInQueue = 0;
	bool bIsHost = false;
	FString LocalPlayerID;
};

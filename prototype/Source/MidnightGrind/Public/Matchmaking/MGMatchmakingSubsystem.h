// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGMatchmakingSubsystem.generated.h"

// Forward declarations
class UMGMultiplayerSubsystem;

UENUM(BlueprintType)
enum class EMGMatchmakingState : uint8
{
	Idle,
	SearchingForMatch,
	MatchFound,
	JoiningLobby,
	InLobby,
	StartingMatch,
	InMatch,
	Cancelled,
	Failed
};

UENUM(BlueprintType)
enum class EMGMatchType : uint8
{
	QuickRace,
	Ranked,
	Private,
	Tournament,
	Custom,
	Crew,
	FreeroamPublic
};

UENUM(BlueprintType)
enum class EMGMatchmakingRegion : uint8
{
	Automatic,
	NorthAmerica,
	SouthAmerica,
	Europe,
	Asia,
	Oceania,
	MiddleEast,
	Africa
};

UENUM(BlueprintType)
enum class EMGSkillTier : uint8
{
	Bronze,
	Silver,
	Gold,
	Platinum,
	Diamond,
	Champion,
	Legend
};

UENUM(BlueprintType)
enum class EMGLobbyPlayerState : uint8
{
	NotReady,
	Ready,
	Loading,
	InGame,
	Spectating,
	Disconnected
};

USTRUCT(BlueprintType)
struct FMGMatchmakingPreferences
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchmakingRegion PreferredRegion = EMGMatchmakingRegion::Automatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreferredRaceMode = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PreferredTrack = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayers = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowCrossPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowBackfill = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSkillBasedMatchmaking = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPingThreshold = 150;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> VehicleClassRestrictions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomSettings;
};

USTRUCT(BlueprintType)
struct FMGPlayerSkillRating
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MMR = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGSkillTier Tier = EMGSkillTier::Bronze;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Division = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WinStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LossStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalRankedRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SeasonLosses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRankedRace;
};

USTRUCT(BlueprintType)
struct FMGMatchmakingTicket
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TicketID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGMatchmakingPreferences Preferences;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerSkillRating SkillRating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SearchTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SearchExpansionLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPing = 0;
};

USTRUCT(BlueprintType)
struct FMGLobbyPlayer
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGLobbyPlayerState State = EMGLobbyPlayerState::NotReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGPlayerSkillRating SkillRating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SelectedVehicle = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TeamIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsHost = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCrossPlayPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime JoinedTime;
};

USTRUCT(BlueprintType)
struct FMGLobbySettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceMode = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPrivate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Password;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowSpectators = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxSpectators = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCollisionsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCatchupEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CountdownTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> AllowedVehicleClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FString> CustomRules;
};

USTRUCT(BlueprintType)
struct FMGMatchLobby
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LobbyID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString HostPlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGLobbySettings Settings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLobbyPlayer> Players;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLobbyPlayer> Spectators;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchmakingRegion Region = EMGMatchmakingRegion::Automatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CreatedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMatchStarting = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CountdownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 AverageSkillMMR = 0;
};

USTRUCT(BlueprintType)
struct FMGMatchResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MatchID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchType MatchType = EMGMatchType::QuickRace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RaceMode = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> FinalStandings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> MMRChanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> RankPointChanges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TotalRaceTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGServerInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ServerAddress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Port = 7777;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMatchmakingRegion Region = EMGMatchmakingRegion::Automatic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDedicated = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAvailable = true;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingStateChanged, EMGMatchmakingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFound, const FMGMatchLobby&, Lobby);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchmakingProgressUpdated, float, SearchTimeSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyUpdated, const FMGMatchLobby&, Lobby);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerJoinedLobby, const FString&, LobbyID, const FMGLobbyPlayer&, Player);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeftLobby, const FString&, LobbyID, const FString&, PlayerID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerStateChanged, const FString&, PlayerID, EMGLobbyPlayerState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyCountdownStarted, float, CountdownTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchStarting);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, const FMGMatchResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSkillRatingUpdated, const FMGPlayerSkillRating&, OldRating, const FMGPlayerSkillRating&, NewRating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnKickedFromLobby);

UCLASS()
class MIDNIGHTGRIND_API UMGMatchmakingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Matchmaking
	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool StartMatchmaking(const FMGMatchmakingPreferences& Preferences);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	void CancelMatchmaking();

	UFUNCTION(BlueprintCallable, Category = "Matchmaking")
	bool IsMatchmaking() const { return MatchmakingState == EMGMatchmakingState::SearchingForMatch; }

	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	EMGMatchmakingState GetMatchmakingState() const { return MatchmakingState; }

	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	float GetMatchmakingTime() const;

	UFUNCTION(BlueprintPure, Category = "Matchmaking")
	FMGMatchmakingTicket GetCurrentTicket() const { return CurrentTicket; }

	// Lobby Management
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool CreateLobby(const FMGLobbySettings& Settings, EMGMatchType MatchType);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool JoinLobby(const FString& LobbyID, const FString& Password = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool JoinLobbyByCode(const FString& JoinCode);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	void LeaveLobby();

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Lobby")
	bool IsInLobby() const { return bInLobby; }

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Lobby")
	FMGMatchLobby GetCurrentLobby() const { return CurrentLobby; }

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Lobby")
	FString GenerateLobbyJoinCode() const;

	// Lobby Host Functions
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool UpdateLobbySettings(const FMGLobbySettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool KickPlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool BanPlayer(const FString& PlayerID);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool TransferHost(const FString& NewHostPlayerID);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	bool StartLobbyCountdown();

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Lobby")
	void CancelLobbyCountdown();

	// Player Functions
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Player")
	void SetPlayerReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Player")
	void SetSelectedVehicle(FName VehicleID);

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Player")
	void SetTeam(int32 TeamIndex);

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	bool IsLocalPlayerHost() const;

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	bool IsLocalPlayerReady() const;

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	bool AreAllPlayersReady() const;

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Player")
	int32 GetReadyPlayerCount() const;

	// Skill Rating
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Skill")
	FMGPlayerSkillRating GetLocalPlayerSkillRating() const { return LocalPlayerSkill; }

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Skill")
	void UpdateSkillRatingFromMatch(const FMGMatchResult& MatchResult);

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Skill")
	EMGSkillTier CalculateTierFromMMR(int32 MMR) const;

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Skill")
	int32 CalculateMMRChange(int32 CurrentMMR, int32 OpponentAvgMMR, int32 Position, int32 TotalPlayers) const;

	// Server Browser
	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Browser")
	void RefreshServerList();

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Browser")
	void RefreshLobbyList(EMGMatchType TypeFilter);

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Browser")
	TArray<FMGServerInfo> GetAvailableServers() const { return AvailableServers; }

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Browser")
	TArray<FMGMatchLobby> GetAvailableLobbies() const { return AvailableLobbies; }

	// Region
	UFUNCTION(BlueprintPure, Category = "Matchmaking|Region")
	EMGMatchmakingRegion GetBestRegion() const;

	UFUNCTION(BlueprintCallable, Category = "Matchmaking|Region")
	void PingAllRegions();

	UFUNCTION(BlueprintPure, Category = "Matchmaking|Region")
	int32 GetRegionPing(EMGMatchmakingRegion Region) const;

	// Match History
	UFUNCTION(BlueprintPure, Category = "Matchmaking|History")
	TArray<FMGMatchResult> GetRecentMatches(int32 Count = 10) const;

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchmakingStateChanged OnMatchmakingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchFound OnMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchmakingFailed OnMatchmakingFailed;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchmakingProgressUpdated OnMatchmakingProgressUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnLobbyUpdated OnLobbyUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnPlayerJoinedLobby OnPlayerJoinedLobby;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnPlayerLeftLobby OnPlayerLeftLobby;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnPlayerStateChanged OnPlayerStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnLobbyCountdownStarted OnLobbyCountdownStarted;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchStarting OnMatchStarting;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnMatchEnded OnMatchEnded;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnSkillRatingUpdated OnSkillRatingUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Matchmaking|Events")
	FOnKickedFromLobby OnKickedFromLobby;

protected:
	void SetMatchmakingState(EMGMatchmakingState NewState);
	void OnMatchmakingTick();
	void ExpandSearchCriteria();
	void OnCountdownTick();
	void SimulateMatchFound();
	FMGLobbyPlayer CreateLocalPlayer() const;

	UPROPERTY()
	EMGMatchmakingState MatchmakingState = EMGMatchmakingState::Idle;

	UPROPERTY()
	FMGMatchmakingTicket CurrentTicket;

	UPROPERTY()
	FMGPlayerSkillRating LocalPlayerSkill;

	UPROPERTY()
	FMGMatchLobby CurrentLobby;

	UPROPERTY()
	bool bInLobby = false;

	UPROPERTY()
	TArray<FMGServerInfo> AvailableServers;

	UPROPERTY()
	TArray<FMGMatchLobby> AvailableLobbies;

	UPROPERTY()
	TArray<FMGMatchResult> MatchHistory;

	UPROPERTY()
	TMap<EMGMatchmakingRegion, int32> RegionPings;

	FTimerHandle MatchmakingTimerHandle;
	FTimerHandle CountdownTimerHandle;

	float SearchExpansionInterval = 10.0f;
	float MaxSearchTime = 120.0f;
};

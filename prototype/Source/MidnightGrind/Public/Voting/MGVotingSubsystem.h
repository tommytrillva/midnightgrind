// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVotingSubsystem.h
 * @brief Voting System - Democratic decision-making for multiplayer lobbies and sessions
 *
 * FOR ENTRY-LEVEL DEVELOPERS:
 * ==========================
 * This system handles all voting mechanics in multiplayer sessions. When players need to
 * collectively decide on something (which track to race, whether to kick a disruptive player,
 * etc.), this subsystem manages the entire voting process.
 *
 * Think of it like the voting systems in Counter-Strike (vote to kick), Call of Duty
 * (map voting), or any multiplayer game where players collectively make decisions.
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * --------------------------
 *
 * 1. WHAT IS A "VOTE SESSION"?
 *    - A vote session (FMGVoteSession) is an active vote in progress.
 *    - It has a type (map selection, kick player, etc.), options to choose from,
 *      a time limit, and rules for determining the winner.
 *    - Only ONE vote can be active at a time (see ActiveVote member variable).
 *
 * 2. VOTE TYPES (EMGVoteType):
 *    - MapSelection: Players vote on which track to race next
 *    - GameMode: Vote on race type (circuit, sprint, drift, etc.)
 *    - RaceSettings: Vote on race options (laps, weather, etc.)
 *    - KickPlayer: Vote to remove a disruptive player
 *    - SkipRace: Vote to skip current race/return to lobby
 *    - RestartRace: Vote to restart the current race
 *    - EndSession: Vote to end the multiplayer session entirely
 *    - Custom: For any other voting needs
 *
 * 3. VOTE STATES (EMGVoteState):
 *    - Inactive: No vote happening
 *    - Pending: Vote initialized but not yet started (server sync)
 *    - Active: Vote is live, players can cast votes
 *    - Passed: Vote succeeded (enough votes for winning option)
 *    - Failed: Vote failed (not enough votes or threshold not met)
 *    - Cancelled: Vote was manually cancelled (by host usually)
 *    - Expired: Vote ran out of time
 *
 * 4. WHAT IS A "THRESHOLD"?
 *    - The threshold is the percentage of votes needed to pass.
 *    - PassThreshold of 0.5 means 50% of votes needed to win.
 *    - KickVoteThreshold of 0.6 means 60% must agree to kick a player.
 *    - Higher thresholds for serious actions (kicking) prevent abuse.
 *
 * 5. HOW MAP VOTING WORKS:
 *    - System selects several maps (usually 4) from AvailableMaps
 *    - Recent maps can be excluded to ensure variety
 *    - Players vote for their preferred map
 *    - Winner is determined when time expires or everyone votes
 *    - OnMapVoteResult fires with the selected map
 *
 * 6. NETWORKING CONSIDERATIONS:
 *    - Voting must sync across all players in a multiplayer game.
 *    - The host/server manages the authoritative vote state.
 *    - Functions like ReceiveVoteStart, ReceiveVoteCast handle incoming
 *      network messages from other players.
 *    - When you cast a vote, it's sent to the server, validated, then
 *      broadcast to all players via ReceiveVoteCast.
 *
 * SYSTEM ARCHITECTURE:
 * -------------------
 *
 * VOTE LIFECYCLE:
 * 1. StartVote() or StartMapVote() is called
 * 2. ActiveVote is populated, timer starts
 * 3. OnVoteStarted delegate fires (UI shows voting panel)
 * 4. Players call CastVote() with their choice
 * 5. OnVoteUpdated fires as votes come in (UI updates counts)
 * 6. OnVoteTick() runs each frame, updating TimeRemaining
 * 7. When time expires or all voted: ProcessVoteEnd()
 * 8. DetermineResult() finds the winner
 * 9. OnVoteEnded fires with results
 * 10. For map votes, OnMapVoteResult fires with selected map
 *
 * CONFIGURATION (FMGVotingConfig):
 * - Vote durations: How long each vote type lasts
 * - Pass thresholds: Required percentage to pass
 * - MinPlayersForVote: Prevents voting with too few players
 * - VoteCooldown: Time between votes to prevent spam
 * - bHostCanOverride: Whether host can bypass voting
 *
 * IMPORTANT FUNCTIONS:
 * -------------------
 * - StartVote/StartMapVote: Initiate a vote
 * - CastVote/ChangeVote: Submit player's choice
 * - CanStartVote: Check if voting is allowed right now
 * - GetWinningOption: Get current leader or final winner
 * - RegisterMap: Add maps to the available pool
 *
 * @see FMGVoteSession - Active vote state and options
 * @see FMGVotingConfig - Voting system configuration
 * @see FMGMapVoteData - Map information for map voting
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGVotingSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGVoteType : uint8
{
	MapSelection,
	GameMode,
	RaceSettings,
	KickPlayer,
	SkipRace,
	RestartRace,
	EndSession,
	Custom
};

UENUM(BlueprintType)
enum class EMGVoteState : uint8
{
	Inactive,
	Pending,
	Active,
	Passed,
	Failed,
	Cancelled,
	Expired
};

USTRUCT(BlueprintType)
struct FMGVoteOption
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Thumbnail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 VoteCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Voters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDefault = false;
};

USTRUCT(BlueprintType)
struct FMGVoteSession
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid VoteID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVoteType VoteType = EMGVoteType::MapSelection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGVoteState State = EMGVoteState::Inactive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VoteTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText VoteDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGVoteOption> Options;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName InitiatorID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeRemaining = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalVoters = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredVotes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PassThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WinningOptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowAbstain = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowResultsLive = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAllowVoteChange = true;
};

USTRUCT(BlueprintType)
struct FMGMapVoteData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MapID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> MapPreview;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TrackLength = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxPlayers = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> SupportedModes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsNightTrack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasWeather = false;
};

USTRUCT(BlueprintType)
struct FMGVotingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DefaultVoteDuration = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MapVoteDuration = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KickVoteDuration = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float KickVoteThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SkipVoteThreshold = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MinPlayersForVote = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float VoteCooldown = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxMapOptions = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRandomizeMapOrder = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bExcludeRecentMaps = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecentMapsToExclude = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHostCanOverride = true;
};

USTRUCT(BlueprintType)
struct FMGPlayerVote
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OptionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime VoteTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAbstained = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteStarted, const FMGVoteSession&, Vote);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoteUpdated, const FMGVoteSession&, Vote, FName, OptionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnVoteEnded, const FMGVoteSession&, Vote, bool, bPassed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteCancelled, FGuid, VoteID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMapVoteResult, const FMGMapVoteData&, SelectedMap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoteTimeUpdate, float, TimeRemaining);

UCLASS()
class MIDNIGHTGRIND_API UMGVotingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Vote Management
	UFUNCTION(BlueprintCallable, Category = "Voting|Management")
	FGuid StartVote(EMGVoteType Type, const TArray<FMGVoteOption>& Options, float Duration = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Voting|Management")
	bool CancelVote(FGuid VoteID);

	UFUNCTION(BlueprintCallable, Category = "Voting|Management")
	bool ForceEndVote(FGuid VoteID);

	UFUNCTION(BlueprintPure, Category = "Voting|Management")
	bool IsVoteActive() const;

	UFUNCTION(BlueprintPure, Category = "Voting|Management")
	FMGVoteSession GetActiveVote() const { return ActiveVote; }

	UFUNCTION(BlueprintPure, Category = "Voting|Management")
	bool CanStartVote(EMGVoteType Type) const;

	// Casting Votes
	UFUNCTION(BlueprintCallable, Category = "Voting|Cast")
	bool CastVote(FName OptionID);

	UFUNCTION(BlueprintCallable, Category = "Voting|Cast")
	bool ChangeVote(FName NewOptionID);

	UFUNCTION(BlueprintCallable, Category = "Voting|Cast")
	bool Abstain();

	UFUNCTION(BlueprintPure, Category = "Voting|Cast")
	bool HasVoted() const;

	UFUNCTION(BlueprintPure, Category = "Voting|Cast")
	FName GetMyVote() const;

	// Map Voting
	UFUNCTION(BlueprintCallable, Category = "Voting|Map")
	FGuid StartMapVote(const TArray<FMGMapVoteData>& MapOptions);

	UFUNCTION(BlueprintCallable, Category = "Voting|Map")
	FGuid StartRandomMapVote(int32 NumOptions = 4);

	UFUNCTION(BlueprintCallable, Category = "Voting|Map")
	void RegisterMap(const FMGMapVoteData& MapData);

	UFUNCTION(BlueprintPure, Category = "Voting|Map")
	TArray<FMGMapVoteData> GetAvailableMaps() const { return AvailableMaps; }

	UFUNCTION(BlueprintPure, Category = "Voting|Map")
	FMGMapVoteData GetMapData(FName MapID) const;

	// Quick Votes
	UFUNCTION(BlueprintCallable, Category = "Voting|Quick")
	FGuid StartKickVote(FName TargetPlayerID, const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Voting|Quick")
	FGuid StartSkipVote();

	UFUNCTION(BlueprintCallable, Category = "Voting|Quick")
	FGuid StartRestartVote();

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "Voting|Config")
	void SetConfig(const FMGVotingConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "Voting|Config")
	FMGVotingConfig GetConfig() const { return Config; }

	// Results
	UFUNCTION(BlueprintPure, Category = "Voting|Results")
	FMGVoteOption GetWinningOption() const;

	UFUNCTION(BlueprintPure, Category = "Voting|Results")
	TArray<FMGVoteOption> GetSortedResults() const;

	UFUNCTION(BlueprintPure, Category = "Voting|Results")
	float GetOptionVotePercentage(FName OptionID) const;

	UFUNCTION(BlueprintPure, Category = "Voting|Results")
	int32 GetTotalVotesCast() const;

	// Player Management
	UFUNCTION(BlueprintCallable, Category = "Voting|Players")
	void SetLocalPlayer(FName PlayerID, const FString& PlayerName, bool bIsHost);

	UFUNCTION(BlueprintCallable, Category = "Voting|Players")
	void AddPlayer(FName PlayerID, const FString& PlayerName);

	UFUNCTION(BlueprintCallable, Category = "Voting|Players")
	void RemovePlayer(FName PlayerID);

	UFUNCTION(BlueprintPure, Category = "Voting|Players")
	int32 GetPlayerCount() const { return Players.Num(); }

	// Network
	UFUNCTION(BlueprintCallable, Category = "Voting|Network")
	void ReceiveVoteStart(const FMGVoteSession& Vote);

	UFUNCTION(BlueprintCallable, Category = "Voting|Network")
	void ReceiveVoteCast(FName PlayerID, FName OptionID);

	UFUNCTION(BlueprintCallable, Category = "Voting|Network")
	void ReceiveVoteEnd(FGuid VoteID, FName WinningOption, bool bPassed);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Voting|Events")
	FOnVoteStarted OnVoteStarted;

	UPROPERTY(BlueprintAssignable, Category = "Voting|Events")
	FOnVoteUpdated OnVoteUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Voting|Events")
	FOnVoteEnded OnVoteEnded;

	UPROPERTY(BlueprintAssignable, Category = "Voting|Events")
	FOnVoteCancelled OnVoteCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Voting|Events")
	FOnMapVoteResult OnMapVoteResult;

	UPROPERTY(BlueprintAssignable, Category = "Voting|Events")
	FOnVoteTimeUpdate OnVoteTimeUpdate;

protected:
	void OnVoteTick();
	void ProcessVoteEnd();
	FMGVoteOption CreateYesNoOptions();
	void UpdateVoteCounts();
	bool DetermineResult();
	void AddToRecentMaps(FName MapID);
	TArray<FMGMapVoteData> SelectRandomMaps(int32 Count);

	UPROPERTY()
	FMGVoteSession ActiveVote;

	UPROPERTY()
	TMap<FName, FMGPlayerVote> PlayerVotes;

	UPROPERTY()
	TMap<FName, FString> Players;

	UPROPERTY()
	TArray<FMGMapVoteData> AvailableMaps;

	UPROPERTY()
	TArray<FName> RecentMaps;

	UPROPERTY()
	FMGVotingConfig Config;

	UPROPERTY()
	FName LocalPlayerID;

	UPROPERTY()
	FString LocalPlayerName;

	UPROPERTY()
	bool bIsHost = false;

	UPROPERTY()
	float LastVoteTime = 0.0f;

	FTimerHandle VoteTickHandle;
};

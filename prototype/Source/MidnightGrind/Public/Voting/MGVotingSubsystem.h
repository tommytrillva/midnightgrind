// Copyright Midnight Grind. All Rights Reserved.

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

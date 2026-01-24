// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrewBattlesSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCrewBattleState : uint8
{
	None,
	Scheduled,
	Matchmaking,
	Preparing,
	InProgress,
	Completed,
	Cancelled,
	Disputed
};

UENUM(BlueprintType)
enum class EMGCrewBattleType : uint8
{
	Ranked,
	Casual,
	Tournament,
	Rivalry,
	Territory,
	Weekly
};

UENUM(BlueprintType)
enum class EMGCrewBattleFormat : uint8
{
	BestOf1,
	BestOf3,
	BestOf5,
	PointBased,
	Elimination
};

USTRUCT(BlueprintType)
struct FMGCrewBattleParticipant
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrewRating = 1500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RosterPlayerIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> RosterPlayerNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bForfeit = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> CrewLogo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CrewColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGCrewBattleRound
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoundNumber = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TrackID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TrackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName GameModeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 LapCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WinnerCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> PlayerPositions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> PlayerTimes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Crew1Points = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Crew2Points = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

USTRUCT(BlueprintType)
struct FMGCrewBattle
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid BattleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewBattleState State = EMGCrewBattleState::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewBattleType BattleType = EMGCrewBattleType::Ranked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewBattleFormat Format = EMGCrewBattleFormat::BestOf3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCrewBattleParticipant Crew1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGCrewBattleParticipant Crew2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGCrewBattleRound> Rounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentRound = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredWins = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName WinnerCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ScheduledTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RatingChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> MapPool;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> BannedMaps;
};

USTRUCT(BlueprintType)
struct FMGCrewBattleChallenge
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ChallengeID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ChallengerCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ChallengerCrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DefenderCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DefenderCrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewBattleType BattleType = EMGCrewBattleType::Ranked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewBattleFormat Format = EMGCrewBattleFormat::BestOf3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ProposedTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime ExpiresAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WagerAmount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAccepted = false;
};

USTRUCT(BlueprintType)
struct FMGCrewBattleHistory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid BattleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName OpponentCrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OpponentCrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrewBattleType BattleType = EMGCrewBattleType::Ranked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bWon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreFor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ScoreAgainst = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RatingChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> TracksPlayed;
};

USTRUCT(BlueprintType)
struct FMGCrewBattleStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalBattles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Losses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BestWinStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoundsWon = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RoundsLost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MostBeatenCrew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BeatenCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName RivalCrew;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RivalLosses = 0;
};

USTRUCT(BlueprintType)
struct FMGCrewLeaderboardEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rank = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CrewTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Rating = 1500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Losses = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> CrewLogo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor CrewColor = FLinearColor::White;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewBattleMatchFound, const FMGCrewBattle&, Battle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewBattleStarted, const FMGCrewBattle&, Battle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewBattleRoundComplete, const FMGCrewBattle&, Battle, const FMGCrewBattleRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewBattleComplete, const FMGCrewBattle&, Battle, bool, bWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrewChallengeReceived, const FMGCrewBattleChallenge&, Challenge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewChallengeResponse, FGuid, ChallengeID, bool, bAccepted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrewRatingChanged, FName, CrewID, int32, NewRating);

UCLASS()
class MIDNIGHTGRIND_API UMGCrewBattlesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Matchmaking
	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Matchmaking")
	bool StartMatchmaking(EMGCrewBattleType BattleType, EMGCrewBattleFormat Format);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Matchmaking")
	void CancelMatchmaking();

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Matchmaking")
	bool IsMatchmaking() const { return bIsMatchmaking; }

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Matchmaking")
	float GetMatchmakingTime() const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Matchmaking")
	int32 GetEstimatedWaitTime() const;

	// Challenges
	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Challenge")
	FGuid SendChallenge(FName TargetCrewID, EMGCrewBattleType BattleType, EMGCrewBattleFormat Format, FDateTime ProposedTime, int32 WagerAmount = 0);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Challenge")
	bool AcceptChallenge(FGuid ChallengeID);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Challenge")
	bool DeclineChallenge(FGuid ChallengeID);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Challenge")
	bool CancelChallenge(FGuid ChallengeID);

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Challenge")
	TArray<FMGCrewBattleChallenge> GetIncomingChallenges() const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Challenge")
	TArray<FMGCrewBattleChallenge> GetOutgoingChallenges() const;

	// Active Battle
	UFUNCTION(BlueprintPure, Category = "CrewBattles|Active")
	bool IsInBattle() const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Active")
	FMGCrewBattle GetActiveBattle() const { return ActiveBattle; }

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Active")
	void SetRosterReady(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Active")
	bool SetRoster(const TArray<FName>& PlayerIDs);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Active")
	void BanMap(FName TrackID);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Active")
	void SelectMap(FName TrackID);

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Active")
	TArray<FName> GetAvailableMaps() const;

	// Round Management
	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Round")
	void StartNextRound();

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Round")
	void ReportRoundResult(const FMGCrewBattleRound& RoundResult);

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Round")
	FMGCrewBattleRound GetCurrentRound() const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Round")
	int32 GetRoundsToWin() const;

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Round")
	void Forfeit();

	// History & Stats
	UFUNCTION(BlueprintPure, Category = "CrewBattles|History")
	TArray<FMGCrewBattleHistory> GetBattleHistory(int32 MaxEntries = 20) const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|History")
	TArray<FMGCrewBattleHistory> GetHistoryVsCrew(FName CrewID) const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|History")
	FMGCrewBattleStats GetCrewStats() const { return Stats; }

	UFUNCTION(BlueprintPure, Category = "CrewBattles|History")
	int32 GetHeadToHeadRecord(FName CrewID, int32& OutWins, int32& OutLosses) const;

	// Leaderboard
	UFUNCTION(BlueprintPure, Category = "CrewBattles|Leaderboard")
	TArray<FMGCrewLeaderboardEntry> GetTopCrews(int32 Count = 50) const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Leaderboard")
	int32 GetCrewLeaderboardPosition() const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Leaderboard")
	FMGCrewLeaderboardEntry GetCrewLeaderboardEntry(FName CrewID) const;

	// Rating
	UFUNCTION(BlueprintPure, Category = "CrewBattles|Rating")
	int32 GetCrewRating() const;

	UFUNCTION(BlueprintPure, Category = "CrewBattles|Rating")
	int32 PredictRatingChange(int32 OpponentRating, bool bWin) const;

	// Crew Info
	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Crew")
	void SetLocalCrewInfo(FName CrewID, const FString& CrewName, const FString& CrewTag, int32 Rating);

	// Network
	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Network")
	void ReceiveBattleUpdate(const FMGCrewBattle& Battle);

	UFUNCTION(BlueprintCallable, Category = "CrewBattles|Network")
	void ReceiveChallenge(const FMGCrewBattleChallenge& Challenge);

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewBattleMatchFound OnCrewBattleMatchFound;

	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewBattleStarted OnCrewBattleStarted;

	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewBattleRoundComplete OnCrewBattleRoundComplete;

	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewBattleComplete OnCrewBattleComplete;

	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewChallengeReceived OnCrewChallengeReceived;

	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewChallengeResponse OnCrewChallengeResponse;

	UPROPERTY(BlueprintAssignable, Category = "CrewBattles|Events")
	FOnCrewRatingChanged OnCrewRatingChanged;

protected:
	void OnBattleTick();
	void ProcessBattleCompletion(bool bWon);
	int32 CalculateRatingChange(int32 CrewRating, int32 OpponentRating, bool bWon) const;
	void UpdateStats(const FMGCrewBattle& Battle, bool bWon);
	void AddToHistory(const FMGCrewBattle& Battle, bool bWon);
	void SaveBattleData();
	void LoadBattleData();
	void CheckExpiredChallenges();
	int32 CalculateRoundPoints(const TMap<FName, int32>& Positions, bool bIsOurCrew) const;

	UPROPERTY()
	FMGCrewBattle ActiveBattle;

	UPROPERTY()
	TArray<FMGCrewBattleChallenge> IncomingChallenges;

	UPROPERTY()
	TArray<FMGCrewBattleChallenge> OutgoingChallenges;

	UPROPERTY()
	TArray<FMGCrewBattleHistory> BattleHistory;

	UPROPERTY()
	TArray<FMGCrewLeaderboardEntry> CachedLeaderboard;

	UPROPERTY()
	FMGCrewBattleStats Stats;

	UPROPERTY()
	FName LocalCrewID;

	UPROPERTY()
	FString LocalCrewName;

	UPROPERTY()
	FString LocalCrewTag;

	UPROPERTY()
	int32 LocalCrewRating = 1500;

	UPROPERTY()
	int32 LeaderboardPosition = 0;

	UPROPERTY()
	bool bIsMatchmaking = false;

	UPROPERTY()
	float MatchmakingStartTime = 0.0f;

	FTimerHandle BattleTickHandle;
};

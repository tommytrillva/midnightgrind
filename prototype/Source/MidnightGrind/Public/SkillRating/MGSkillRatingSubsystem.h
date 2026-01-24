// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSkillRatingSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGRankTier : uint8
{
	Unranked,
	Bronze,
	Silver,
	Gold,
	Platinum,
	Diamond,
	Master,
	Grandmaster,
	Legend
};

UENUM(BlueprintType)
enum class EMGRankDivision : uint8
{
	IV,
	III,
	II,
	I
};

UENUM(BlueprintType)
enum class EMGRatingCategory : uint8
{
	Overall,
	CircuitRacing,
	SprintRacing,
	Drifting,
	TimeAttack,
	TeamRacing
};

UENUM(BlueprintType)
enum class EMGPlacementStatus : uint8
{
	NotStarted,
	InProgress,
	Completed
};

USTRUCT(BlueprintType)
struct FMGRank
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRankTier Tier = EMGRankTier::Unranked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRankDivision Division = EMGRankDivision::IV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPoints = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsToNextDivision = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PointsInCurrentDivision = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RankName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> RankIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RankColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct FMGSkillRating
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRatingCategory Category = EMGRatingCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MMR = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Uncertainty = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank CurrentRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank PeakRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GamesPlayed = 0;

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
	int32 WorstLossStreak = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastPlayed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlacementStatus PlacementStatus = EMGPlacementStatus::NotStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlacementGamesPlayed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PlacementGamesRequired = 10;
};

USTRUCT(BlueprintType)
struct FMGMatchResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid MatchID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGRatingCategory Category = EMGRatingCategory::Overall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalPlayers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AverageOpponentMMR = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RatingChange = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NewMMR = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RankPointsChange = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bPromoted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDemoted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank OldRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank NewRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;
};

USTRUCT(BlueprintType)
struct FMGOpponentRating
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MMR = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank Rank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Position = 0;
};

USTRUCT(BlueprintType)
struct FMGSeasonStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SeasonID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank HighestRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGRank FinalRank;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PeakMMR = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalGames = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Wins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WinRate = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RewardsEarned;
};

USTRUCT(BlueprintType)
struct FMGRatingConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseKFactor = 32.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PlacementKFactor = 64.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float UncertaintyDecay = 0.98f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinUncertainty = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxUncertainty = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InactivityUncertaintyGain = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InactivityDaysThreshold = 14;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PromotionBonus = 25;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DemotionProtectionGames = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StreakBonusMultiplier = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxStreakBonus = 5;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRatingChanged, EMGRatingCategory, Category, float, NewRating);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRankChanged, EMGRatingCategory, Category, const FMGRank&, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPromoted, EMGRatingCategory, Category, const FMGRank&, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDemoted, EMGRatingCategory, Category, const FMGRank&, NewRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchResultProcessed, const FMGMatchResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlacementCompleted, EMGRatingCategory, Category, const FMGRank&, InitialRank);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSeasonEnded);

UCLASS()
class MIDNIGHTGRIND_API UMGSkillRatingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	// Rating Access
	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	FMGSkillRating GetRating(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	TMap<EMGRatingCategory, FMGSkillRating> GetAllRatings() const { return Ratings; }

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	FMGRank GetRank(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	float GetMMR(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Access")
	float GetDisplayRating(EMGRatingCategory Category) const;

	// Match Processing
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Match")
	FMGMatchResult ProcessMatchResult(EMGRatingCategory Category, int32 Position, const TArray<FMGOpponentRating>& Opponents);

	UFUNCTION(BlueprintCallable, Category = "SkillRating|Match")
	FMGMatchResult ProcessSimpleResult(EMGRatingCategory Category, bool bWon, float OpponentMMR);

	UFUNCTION(BlueprintPure, Category = "SkillRating|Match")
	float PredictRatingChange(EMGRatingCategory Category, int32 ExpectedPosition, float AverageOpponentMMR) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Match")
	float CalculateWinProbability(float PlayerMMR, float OpponentMMR) const;

	// Rank Info
	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	FMGRank GetRankFromMMR(float MMR) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	int32 GetRankPointsForMMR(float MMR) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	float GetProgressToNextDivision(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	bool IsInPromotionSeries(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	bool IsInDemotionZone(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Rank")
	TArray<FMGRank> GetAllRankTiers() const;

	// Placement
	UFUNCTION(BlueprintPure, Category = "SkillRating|Placement")
	bool IsInPlacements(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Placement")
	int32 GetPlacementGamesRemaining(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Placement")
	float GetPlacementProgress(EMGRatingCategory Category) const;

	// History
	UFUNCTION(BlueprintPure, Category = "SkillRating|History")
	TArray<FMGMatchResult> GetMatchHistory(EMGRatingCategory Category, int32 MaxEntries = 20) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|History")
	TArray<FMGSeasonStats> GetSeasonHistory() const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|History")
	FMGSeasonStats GetCurrentSeasonStats() const;

	// Leaderboard Position
	UFUNCTION(BlueprintPure, Category = "SkillRating|Leaderboard")
	int32 GetLeaderboardPosition(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Leaderboard")
	int32 GetRegionalPosition(EMGRatingCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "SkillRating|Leaderboard")
	float GetTopPercentile(EMGRatingCategory Category) const;

	// Season
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Season")
	void StartNewSeason(FName SeasonID);

	UFUNCTION(BlueprintCallable, Category = "SkillRating|Season")
	void EndSeason();

	UFUNCTION(BlueprintPure, Category = "SkillRating|Season")
	FName GetCurrentSeasonID() const { return CurrentSeasonID; }

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Config")
	void SetConfig(const FMGRatingConfig& NewConfig);

	UFUNCTION(BlueprintPure, Category = "SkillRating|Config")
	FMGRatingConfig GetConfig() const { return Config; }

	// Persistence
	UFUNCTION(BlueprintCallable, Category = "SkillRating|Save")
	void SaveRatingData();

	UFUNCTION(BlueprintCallable, Category = "SkillRating|Save")
	void LoadRatingData();

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnRatingChanged OnRatingChanged;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnRankChanged OnRankChanged;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnPromoted OnPromoted;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnDemoted OnDemoted;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnMatchResultProcessed OnMatchResultProcessed;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnPlacementCompleted OnPlacementCompleted;

	UPROPERTY(BlueprintAssignable, Category = "SkillRating|Events")
	FOnSeasonEnded OnSeasonEnded;

protected:
	float CalculateRatingChange(float PlayerMMR, float PlayerUncertainty, float OpponentMMR, float Score, bool bIsPlacement) const;
	float CalculateExpectedScore(float PlayerMMR, float OpponentMMR) const;
	void UpdateRank(FMGSkillRating& Rating);
	void ApplySoftReset(FMGSkillRating& Rating);
	void UpdateStreak(FMGSkillRating& Rating, bool bWon);
	void InitializeRatings();
	void InitializeRankThresholds();
	void CheckInactivity();

	UPROPERTY()
	TMap<EMGRatingCategory, FMGSkillRating> Ratings;

	UPROPERTY()
	TMap<EMGRatingCategory, TArray<FMGMatchResult>> MatchHistory;

	UPROPERTY()
	TArray<FMGSeasonStats> SeasonHistory;

	UPROPERTY()
	FMGSeasonStats CurrentSeasonStats;

	UPROPERTY()
	FMGRatingConfig Config;

	UPROPERTY()
	FName CurrentSeasonID;

	UPROPERTY()
	TArray<int32> RankThresholds;

	UPROPERTY()
	int32 GlobalLeaderboardPosition = 0;

	UPROPERTY()
	int32 RegionalLeaderboardPosition = 0;

	UPROPERTY()
	float TopPercentile = 100.0f;
};

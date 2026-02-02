// Copyright Midnight Grind. All Rights Reserved.

#include "SkillRating/MGSkillRatingSubsystem.h"

void UMGSkillRatingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set default configuration
	Config.BaseKFactor = 32.0f;
	Config.PlacementKFactor = 64.0f;
	Config.UncertaintyDecay = 0.98f;
	Config.MinUncertainty = 100.0f;
	Config.MaxUncertainty = 350.0f;
	Config.InactivityUncertaintyGain = 10.0f;
	Config.InactivityDaysThreshold = 14;
	Config.PromotionBonus = 25;
	Config.DemotionProtectionGames = 3;
	Config.StreakBonusMultiplier = 0.1f;
	Config.MaxStreakBonus = 5;

	InitializeRankThresholds();
	InitializeRatings();
	LoadRatingData();
}

void UMGSkillRatingSubsystem::Deinitialize()
{
	SaveRatingData();
	Super::Deinitialize();
}

bool UMGSkillRatingSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

FMGSkillRating UMGSkillRatingSubsystem::GetRating(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	return Rating ? *Rating : FMGSkillRating();
}

FMGRank UMGSkillRatingSubsystem::GetRank(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (Rating && Rating->PlacementStatus == EMGPlacementStatus::Completed)
	{
		return Rating->CurrentRank;
	}
	return FMGRank();
}

float UMGSkillRatingSubsystem::GetMMR(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	return Rating ? Rating->MMR : 1500.0f;
}

float UMGSkillRatingSubsystem::GetDisplayRating(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating)
	{
		return 0.0f;
	}

	// During placements, don't show MMR
	if (Rating->PlacementStatus != EMGPlacementStatus::Completed)
	{
		return 0.0f;
	}

	// Display rating is MMR minus some uncertainty buffer
	return FMath::Max(0.0f, Rating->MMR - Rating->Uncertainty * 0.5f);
}

FMGMatchResult UMGSkillRatingSubsystem::ProcessMatchResult(EMGRatingCategory Category, int32 Position, const TArray<FMGOpponentRating>& Opponents)
{
	FMGMatchResult Result;
	Result.MatchID = FGuid::NewGuid();
	Result.Category = Category;
	Result.Position = Position;
	Result.TotalPlayers = Opponents.Num() + 1;
	Result.Timestamp = FDateTime::UtcNow();

	FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating)
	{
		return Result;
	}

	Result.OldRank = Rating->CurrentRank;

	// Calculate average opponent MMR
	float TotalOpponentMMR = 0.0f;
	for (const FMGOpponentRating& Opp : Opponents)
	{
		TotalOpponentMMR += Opp.MMR;
	}
	Result.AverageOpponentMMR = Opponents.Num() > 0 ? TotalOpponentMMR / Opponents.Num() : 1500.0f;

	// Calculate score based on position (1.0 for 1st, 0.0 for last)
	float Score = 1.0f - ((float)(Position - 1) / (float)(Result.TotalPlayers - 1));

	// Calculate rating change
	bool bIsPlacement = Rating->PlacementStatus != EMGPlacementStatus::Completed;
	float RatingChange = CalculateRatingChange(Rating->MMR, Rating->Uncertainty, Result.AverageOpponentMMR, Score, bIsPlacement);

	// Apply streak bonus/penalty
	if (Rating->CurrentStreak != 0)
	{
		int32 StreakBonus = FMath::Min(FMath::Abs(Rating->CurrentStreak), Config.MaxStreakBonus);
		if ((Rating->CurrentStreak > 0 && Score >= 0.5f) || (Rating->CurrentStreak < 0 && Score < 0.5f))
		{
			RatingChange *= (1.0f + StreakBonus * Config.StreakBonusMultiplier);
		}
	}

	// Apply rating change
	Rating->MMR += RatingChange;
	Rating->MMR = FMath::Clamp(Rating->MMR, 0.0f, 5000.0f);

	// Update uncertainty
	Rating->Uncertainty *= Config.UncertaintyDecay;
	Rating->Uncertainty = FMath::Clamp(Rating->Uncertainty, Config.MinUncertainty, Config.MaxUncertainty);

	// Update stats
	Rating->GamesPlayed++;
	Rating->LastPlayed = FDateTime::UtcNow();

	bool bWon = Position == 1;
	if (bWon)
	{
		Rating->Wins++;
	}
	else
	{
		Rating->Losses++;
	}
	Rating->WinRate = Rating->GamesPlayed > 0 ? (float)Rating->Wins / (float)Rating->GamesPlayed : 0.0f;

	UpdateStreak(*Rating, bWon);

	// Handle placements
	if (Rating->PlacementStatus == EMGPlacementStatus::NotStarted)
	{
		Rating->PlacementStatus = EMGPlacementStatus::InProgress;
	}

	if (Rating->PlacementStatus == EMGPlacementStatus::InProgress)
	{
		Rating->PlacementGamesPlayed++;
		if (Rating->PlacementGamesPlayed >= Rating->PlacementGamesRequired)
		{
			Rating->PlacementStatus = EMGPlacementStatus::Completed;
			UpdateRank(*Rating);
			OnPlacementCompleted.Broadcast(Category, Rating->CurrentRank);
		}
	}
	else
	{
		// Update rank for non-placement games
		FMGRank OldRank = Rating->CurrentRank;
		UpdateRank(*Rating);

		if ((int32)Rating->CurrentRank.Tier > (int32)OldRank.Tier ||
			((int32)Rating->CurrentRank.Tier == (int32)OldRank.Tier &&
			 (int32)Rating->CurrentRank.Division < (int32)OldRank.Division))
		{
			Result.bPromoted = true;
			OnPromoted.Broadcast(Category, Rating->CurrentRank);
		}
		else if ((int32)Rating->CurrentRank.Tier < (int32)OldRank.Tier ||
				 ((int32)Rating->CurrentRank.Tier == (int32)OldRank.Tier &&
				  (int32)Rating->CurrentRank.Division > (int32)OldRank.Division))
		{
			Result.bDemoted = true;
			OnDemoted.Broadcast(Category, Rating->CurrentRank);
		}

		if (OldRank.Tier != Rating->CurrentRank.Tier || OldRank.Division != Rating->CurrentRank.Division)
		{
			OnRankChanged.Broadcast(Category, Rating->CurrentRank);
		}
	}

	// Update peak rank
	if ((int32)Rating->CurrentRank.Tier > (int32)Rating->PeakRank.Tier ||
		((int32)Rating->CurrentRank.Tier == (int32)Rating->PeakRank.Tier &&
		 (int32)Rating->CurrentRank.Division < (int32)Rating->PeakRank.Division))
	{
		Rating->PeakRank = Rating->CurrentRank;
	}

	Result.RatingChange = RatingChange;
	Result.NewMMR = Rating->MMR;
	Result.NewRank = Rating->CurrentRank;
	Result.RankPointsChange = Rating->CurrentRank.RankPoints - Result.OldRank.RankPoints;

	// Add to history
	TArray<FMGMatchResult>& History = MatchHistory.FindOrAdd(Category);
	History.Insert(Result, 0);
	const int32 MaxHistorySize = 50;
	if (History.Num() > MaxHistorySize)
	{
		History.SetNum(MaxHistorySize);
	}

	OnRatingChanged.Broadcast(Category, Rating->MMR);
	OnMatchResultProcessed.Broadcast(Result);

	SaveRatingData();

	return Result;
}

FMGMatchResult UMGSkillRatingSubsystem::ProcessSimpleResult(EMGRatingCategory Category, bool bWon, float OpponentMMR)
{
	TArray<FMGOpponentRating> Opponents;
	FMGOpponentRating Opp;
	Opp.MMR = OpponentMMR;
	Opp.Position = bWon ? 2 : 1;
	Opponents.Add(Opp);

	return ProcessMatchResult(Category, bWon ? 1 : 2, Opponents);
}

float UMGSkillRatingSubsystem::PredictRatingChange(EMGRatingCategory Category, int32 ExpectedPosition, float AverageOpponentMMR) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating)
	{
		return 0.0f;
	}

	float Score = 1.0f - ((float)(ExpectedPosition - 1) / 7.0f); // Assume 8 players
	bool bIsPlacement = Rating->PlacementStatus != EMGPlacementStatus::Completed;

	return CalculateRatingChange(Rating->MMR, Rating->Uncertainty, AverageOpponentMMR, Score, bIsPlacement);
}

float UMGSkillRatingSubsystem::CalculateWinProbability(float PlayerMMR, float OpponentMMR) const
{
	return 1.0f / (1.0f + FMath::Pow(10.0f, (OpponentMMR - PlayerMMR) / 400.0f));
}

FMGRank UMGSkillRatingSubsystem::GetRankFromMMR(float MMR) const
{
	FMGRank Rank;
	Rank.RankPoints = GetRankPointsForMMR(MMR);

	// Determine tier and division based on thresholds
	if (MMR < 800)
	{
		Rank.Tier = EMGRankTier::Bronze;
	}
	else if (MMR < 1200)
	{
		Rank.Tier = EMGRankTier::Silver;
	}
	else if (MMR < 1600)
	{
		Rank.Tier = EMGRankTier::Gold;
	}
	else if (MMR < 2000)
	{
		Rank.Tier = EMGRankTier::Platinum;
	}
	else if (MMR < 2400)
	{
		Rank.Tier = EMGRankTier::Diamond;
	}
	else if (MMR < 2800)
	{
		Rank.Tier = EMGRankTier::Master;
	}
	else if (MMR < 3200)
	{
		Rank.Tier = EMGRankTier::Grandmaster;
	}
	else
	{
		Rank.Tier = EMGRankTier::Legend;
	}

	// Calculate division within tier
	float TierWidth = 400.0f;
	float TierBase = (float)((int32)Rank.Tier - 1) * TierWidth + 400.0f;
	float PositionInTier = MMR - TierBase;
	float DivisionWidth = TierWidth / 4.0f;

	int32 DivisionIndex = FMath::Clamp((int32)(PositionInTier / DivisionWidth), 0, 3);
	Rank.Division = (EMGRankDivision)(3 - DivisionIndex); // IV, III, II, I

	// Calculate points in current division
	Rank.PointsInCurrentDivision = (int32)FMath::Fmod(PositionInTier, DivisionWidth);
	Rank.PointsToNextDivision = (int32)DivisionWidth;

	// Set display info
	FString TierName;
	switch (Rank.Tier)
	{
	case EMGRankTier::Bronze: TierName = "Bronze"; Rank.RankColor = FLinearColor(0.8f, 0.5f, 0.2f, 1.0f); break;
	case EMGRankTier::Silver: TierName = "Silver"; Rank.RankColor = FLinearColor(0.75f, 0.75f, 0.8f, 1.0f); break;
	case EMGRankTier::Gold: TierName = "Gold"; Rank.RankColor = FLinearColor(1.0f, 0.84f, 0.0f, 1.0f); break;
	case EMGRankTier::Platinum: TierName = "Platinum"; Rank.RankColor = FLinearColor(0.3f, 0.8f, 0.8f, 1.0f); break;
	case EMGRankTier::Diamond: TierName = "Diamond"; Rank.RankColor = FLinearColor(0.7f, 0.9f, 1.0f, 1.0f); break;
	case EMGRankTier::Master: TierName = "Master"; Rank.RankColor = FLinearColor(0.6f, 0.2f, 0.8f, 1.0f); break;
	case EMGRankTier::Grandmaster: TierName = "Grandmaster"; Rank.RankColor = FLinearColor(1.0f, 0.2f, 0.2f, 1.0f); break;
	case EMGRankTier::Legend: TierName = "Legend"; Rank.RankColor = FLinearColor(1.0f, 0.9f, 0.5f, 1.0f); break;
	default: TierName = "Unranked"; break;
	}

	FString DivisionString;
	switch (Rank.Division)
	{
	case EMGRankDivision::IV: DivisionString = "IV"; break;
	case EMGRankDivision::III: DivisionString = "III"; break;
	case EMGRankDivision::II: DivisionString = "II"; break;
	case EMGRankDivision::I: DivisionString = "I"; break;
	}

	Rank.RankName = FText::FromString(FString::Printf(TEXT("%s %s"), *TierName, *DivisionString));

	return Rank;
}

int32 UMGSkillRatingSubsystem::GetRankPointsForMMR(float MMR) const
{
	return FMath::Max(0, (int32)(MMR - 400.0f));
}

float UMGSkillRatingSubsystem::GetProgressToNextDivision(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating || Rating->PlacementStatus != EMGPlacementStatus::Completed)
	{
		return 0.0f;
	}

	return (float)Rating->CurrentRank.PointsInCurrentDivision / (float)Rating->CurrentRank.PointsToNextDivision;
}

bool UMGSkillRatingSubsystem::IsInPromotionSeries(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating)
	{
		return false;
	}

	float Progress = GetProgressToNextDivision(Category);
	return Progress >= 0.9f;
}

bool UMGSkillRatingSubsystem::IsInDemotionZone(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating)
	{
		return false;
	}

	return Rating->CurrentRank.PointsInCurrentDivision < Config.DemotionProtectionGames * 10;
}

TArray<FMGRank> UMGSkillRatingSubsystem::GetAllRankTiers() const
{
	TArray<FMGRank> Ranks;

	for (int32 Tier = (int32)EMGRankTier::Bronze; Tier <= (int32)EMGRankTier::Legend; Tier++)
	{
		for (int32 Div = (int32)EMGRankDivision::IV; Div >= (int32)EMGRankDivision::I; Div--)
		{
			float MMR = 400.0f + (Tier - 1) * 400.0f + (3 - Div) * 100.0f + 50.0f;
			FMGRank Rank = GetRankFromMMR(MMR);
			Ranks.Add(Rank);
		}
	}

	return Ranks;
}

bool UMGSkillRatingSubsystem::IsInPlacements(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	return Rating && Rating->PlacementStatus != EMGPlacementStatus::Completed;
}

int32 UMGSkillRatingSubsystem::GetPlacementGamesRemaining(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating)
	{
		return 0;
	}

	return FMath::Max(0, Rating->PlacementGamesRequired - Rating->PlacementGamesPlayed);
}

float UMGSkillRatingSubsystem::GetPlacementProgress(EMGRatingCategory Category) const
{
	const FMGSkillRating* Rating = Ratings.Find(Category);
	if (!Rating || Rating->PlacementGamesRequired == 0)
	{
		return 0.0f;
	}

	return (float)Rating->PlacementGamesPlayed / (float)Rating->PlacementGamesRequired;
}

TArray<FMGMatchResult> UMGSkillRatingSubsystem::GetMatchHistory(EMGRatingCategory Category, int32 MaxEntries) const
{
	const TArray<FMGMatchResult>* History = MatchHistory.Find(Category);
	if (!History)
	{
		return TArray<FMGMatchResult>();
	}

	TArray<FMGMatchResult> Result;
	int32 Count = FMath::Min(MaxEntries, History->Num());
	for (int32 i = 0; i < Count; i++)
	{
		Result.Add((*History)[i]);
	}
	return Result;
}

TArray<FMGSeasonStats> UMGSkillRatingSubsystem::GetSeasonHistory() const
{
	return SeasonHistory;
}

FMGSeasonStats UMGSkillRatingSubsystem::GetCurrentSeasonStats() const
{
	return CurrentSeasonStats;
}

int32 UMGSkillRatingSubsystem::GetLeaderboardPosition(EMGRatingCategory Category) const
{
	return GlobalLeaderboardPosition;
}

int32 UMGSkillRatingSubsystem::GetRegionalPosition(EMGRatingCategory Category) const
{
	return RegionalLeaderboardPosition;
}

float UMGSkillRatingSubsystem::GetTopPercentile(EMGRatingCategory Category) const
{
	return TopPercentile;
}

void UMGSkillRatingSubsystem::StartNewSeason(FName SeasonID)
{
	// Archive current season if one exists
	if (!CurrentSeasonID.IsNone())
	{
		EndSeason();
	}

	CurrentSeasonID = SeasonID;
	CurrentSeasonStats = FMGSeasonStats();
	CurrentSeasonStats.SeasonID = SeasonID;

	// Apply soft reset to all ratings
	for (auto& Pair : Ratings)
	{
		ApplySoftReset(Pair.Value);
	}

	SaveRatingData();
}

void UMGSkillRatingSubsystem::EndSeason()
{
	// Finalize current season stats
	for (const auto& Pair : Ratings)
	{
		const FMGSkillRating& Rating = Pair.Value;
		if ((int32)Rating.CurrentRank.Tier > (int32)CurrentSeasonStats.HighestRank.Tier)
		{
			CurrentSeasonStats.HighestRank = Rating.CurrentRank;
		}
		CurrentSeasonStats.FinalRank = Rating.CurrentRank;
		if (Rating.MMR > CurrentSeasonStats.PeakMMR)
		{
			CurrentSeasonStats.PeakMMR = Rating.MMR;
		}
		CurrentSeasonStats.TotalGames = Rating.GamesPlayed;
		CurrentSeasonStats.Wins = Rating.Wins;
		CurrentSeasonStats.WinRate = Rating.WinRate;
	}

	SeasonHistory.Insert(CurrentSeasonStats, 0);

	OnSeasonEnded.Broadcast();
}

void UMGSkillRatingSubsystem::SetConfig(const FMGRatingConfig& NewConfig)
{
	Config = NewConfig;
}

void UMGSkillRatingSubsystem::SaveRatingData()
{
	// This would integrate with save game system
}

void UMGSkillRatingSubsystem::LoadRatingData()
{
	// This would integrate with save game system
}

float UMGSkillRatingSubsystem::CalculateRatingChange(float PlayerMMR, float PlayerUncertainty, float OpponentMMR, float Score, bool bIsPlacement) const
{
	float ExpectedScore = CalculateExpectedScore(PlayerMMR, OpponentMMR);
	float K = bIsPlacement ? Config.PlacementKFactor : Config.BaseKFactor;

	// Adjust K factor based on uncertainty
	float UncertaintyMultiplier = PlayerUncertainty / Config.MinUncertainty;
	K *= FMath::Clamp(UncertaintyMultiplier, 1.0f, 2.0f);

	return K * (Score - ExpectedScore);
}

float UMGSkillRatingSubsystem::CalculateExpectedScore(float PlayerMMR, float OpponentMMR) const
{
	return 1.0f / (1.0f + FMath::Pow(10.0f, (OpponentMMR - PlayerMMR) / 400.0f));
}

void UMGSkillRatingSubsystem::UpdateRank(FMGSkillRating& Rating)
{
	Rating.CurrentRank = GetRankFromMMR(Rating.MMR);
}

void UMGSkillRatingSubsystem::ApplySoftReset(FMGSkillRating& Rating)
{
	// Soft reset pulls rating toward 1500
	const float ResetPoint = 1500.0f;
	const float ResetStrength = 0.5f;

	Rating.MMR = Rating.MMR + (ResetPoint - Rating.MMR) * ResetStrength;

	// Reset uncertainty
	Rating.Uncertainty = Config.MaxUncertainty;

	// Reset placement
	Rating.PlacementStatus = EMGPlacementStatus::NotStarted;
	Rating.PlacementGamesPlayed = 0;

	// Keep stats but reset streak
	Rating.CurrentStreak = 0;

	UpdateRank(Rating);
}

void UMGSkillRatingSubsystem::UpdateStreak(FMGSkillRating& Rating, bool bWon)
{
	if (bWon)
	{
		if (Rating.CurrentStreak >= 0)
		{
			Rating.CurrentStreak++;
			Rating.BestWinStreak = FMath::Max(Rating.BestWinStreak, Rating.CurrentStreak);
		}
		else
		{
			Rating.CurrentStreak = 1;
		}
	}
	else
	{
		if (Rating.CurrentStreak <= 0)
		{
			Rating.CurrentStreak--;
			Rating.WorstLossStreak = FMath::Min(Rating.WorstLossStreak, Rating.CurrentStreak);
		}
		else
		{
			Rating.CurrentStreak = -1;
		}
	}
}

void UMGSkillRatingSubsystem::InitializeRatings()
{
	// Initialize ratings for all categories
	TArray<EMGRatingCategory> Categories = {
		EMGRatingCategory::Overall,
		EMGRatingCategory::CircuitRacing,
		EMGRatingCategory::SprintRacing,
		EMGRatingCategory::Drifting,
		EMGRatingCategory::TimeAttack,
		EMGRatingCategory::TeamRacing
	};

	for (EMGRatingCategory Category : Categories)
	{
		FMGSkillRating Rating;
		Rating.Category = Category;
		Rating.MMR = 1500.0f;
		Rating.Uncertainty = Config.MaxUncertainty;
		Rating.PlacementStatus = EMGPlacementStatus::NotStarted;
		Rating.PlacementGamesRequired = 10;
		Ratings.Add(Category, Rating);
	}
}

void UMGSkillRatingSubsystem::InitializeRankThresholds()
{
	RankThresholds.Add(0);    // Unranked
	RankThresholds.Add(400);  // Bronze
	RankThresholds.Add(800);  // Silver
	RankThresholds.Add(1200); // Gold
	RankThresholds.Add(1600); // Platinum
	RankThresholds.Add(2000); // Diamond
	RankThresholds.Add(2400); // Master
	RankThresholds.Add(2800); // Grandmaster
	RankThresholds.Add(3200); // Legend
}

void UMGSkillRatingSubsystem::CheckInactivity()
{
	FDateTime Now = FDateTime::UtcNow();

	for (auto& Pair : Ratings)
	{
		FMGSkillRating& Rating = Pair.Value;

		if (Rating.LastPlayed.GetTicks() > 0)
		{
			FTimespan TimeSincePlayed = Now - Rating.LastPlayed;
			int32 DaysSincePlayed = (int32)TimeSincePlayed.GetTotalDays();

			if (DaysSincePlayed > Config.InactivityDaysThreshold)
			{
				int32 ExtraDays = DaysSincePlayed - Config.InactivityDaysThreshold;
				Rating.Uncertainty = FMath::Min(
					Rating.Uncertainty + Config.InactivityUncertaintyGain * ExtraDays,
					Config.MaxUncertainty
				);
			}
		}
	}
}

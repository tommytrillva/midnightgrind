// Copyright Midnight Grind. All Rights Reserved.

#include "Balancing/MGBalancingSubsystem.h"

void UMGBalancingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeDefaultParameters();
	InitializeDifficultyProfiles();
	LoadBalanceData();
	SetDifficultyTier(CurrentDifficultyTier);
}

void UMGBalancingSubsystem::Deinitialize()
{
	SaveBalanceData();
	Super::Deinitialize();
}

void UMGBalancingSubsystem::SetParameter(FName ParameterID, float Value)
{
	if (FMGBalanceParameter* Param = Parameters.Find(ParameterID))
	{
		float ClampedValue = FMath::Clamp(Value, Param->MinValue, Param->MaxValue);
		if (!FMath::IsNearlyEqual(Param->CurrentValue, ClampedValue))
		{
			Param->CurrentValue = ClampedValue;
			OnBalanceParameterChanged.Broadcast(ParameterID, ClampedValue);
			SaveBalanceData();
		}
	}
}

float UMGBalancingSubsystem::GetParameter(FName ParameterID) const
{
	if (const FMGBalanceParameter* Param = Parameters.Find(ParameterID))
	{
		if (Param->bUseRemoteOverride && Param->RemoteOverride >= 0.0f)
			return Param->RemoteOverride;
		return Param->CurrentValue;
	}
	return 1.0f;
}

FMGBalanceParameter UMGBalancingSubsystem::GetParameterInfo(FName ParameterID) const
{
	if (const FMGBalanceParameter* Param = Parameters.Find(ParameterID))
		return *Param;
	return FMGBalanceParameter();
}

TArray<FMGBalanceParameter> UMGBalancingSubsystem::GetParametersByCategory(EMGBalanceCategory Category) const
{
	TArray<FMGBalanceParameter> Result;
	for (const auto& Pair : Parameters)
	{
		if (Pair.Value.Category == Category)
			Result.Add(Pair.Value);
	}
	return Result;
}

void UMGBalancingSubsystem::ResetParameterToDefault(FName ParameterID)
{
	if (FMGBalanceParameter* Param = Parameters.Find(ParameterID))
	{
		SetParameter(ParameterID, Param->BaseValue);
	}
}

void UMGBalancingSubsystem::ResetAllParameters()
{
	for (auto& Pair : Parameters)
	{
		Pair.Value.CurrentValue = Pair.Value.BaseValue;
	}
	SaveBalanceData();
}

void UMGBalancingSubsystem::SetDifficultyTier(EMGDifficultyTier Tier)
{
	if (CurrentDifficultyTier != Tier)
	{
		CurrentDifficultyTier = Tier;
		OnDifficultyTierChanged.Broadcast(Tier);
	}

	if (const FMGDifficultyProfile* Profile = DifficultyProfiles.Find(Tier))
	{
		CurrentDifficultyProfile = *Profile;
	}

	SaveBalanceData();
}

FMGDifficultyProfile UMGBalancingSubsystem::GetDifficultyProfileForTier(EMGDifficultyTier Tier) const
{
	if (const FMGDifficultyProfile* Profile = DifficultyProfiles.Find(Tier))
		return *Profile;
	return FMGDifficultyProfile();
}

void UMGBalancingSubsystem::SetAdaptiveDifficultyEnabled(bool bEnabled)
{
	AdaptiveDifficultyState.bEnabled = bEnabled;
	SaveBalanceData();
}

void UMGBalancingSubsystem::RecordRaceResult(int32 Position, int32 TotalRacers, float RaceTimeSeconds)
{
	// Update metrics
	PlayerMetrics.TotalRaces++;
	PlayerMetrics.AverageRaceTimeSeconds =
		(PlayerMetrics.AverageRaceTimeSeconds * (PlayerMetrics.TotalRaces - 1) + RaceTimeSeconds) / PlayerMetrics.TotalRaces;

	float PositionPercent = TotalRacers > 1 ? (float)(TotalRacers - Position) / (float)(TotalRacers - 1) : 1.0f;
	PlayerMetrics.AverageRacePosition =
		(PlayerMetrics.AverageRacePosition * (PlayerMetrics.TotalRaces - 1) + Position) / PlayerMetrics.TotalRaces;

	bool bWon = Position == 1;
	bool bPodium = Position <= 3;

	if (bWon)
	{
		PlayerMetrics.ConsecutiveWins++;
		PlayerMetrics.ConsecutiveLosses = 0;
		PlayerMetrics.WinRate = (PlayerMetrics.WinRate * (PlayerMetrics.TotalRaces - 1) + 1.0f) / PlayerMetrics.TotalRaces;
	}
	else
	{
		PlayerMetrics.ConsecutiveWins = 0;
		PlayerMetrics.ConsecutiveLosses++;
		PlayerMetrics.WinRate = (PlayerMetrics.WinRate * (PlayerMetrics.TotalRaces - 1)) / PlayerMetrics.TotalRaces;
	}

	if (bPodium)
	{
		PlayerMetrics.PodiumRate = (PlayerMetrics.PodiumRate * (PlayerMetrics.TotalRaces - 1) + 1.0f) / PlayerMetrics.TotalRaces;
	}
	else
	{
		PlayerMetrics.PodiumRate = (PlayerMetrics.PodiumRate * (PlayerMetrics.TotalRaces - 1)) / PlayerMetrics.TotalRaces;
	}

	// Update adaptive difficulty
	if (AdaptiveDifficultyState.bEnabled)
	{
		UpdateAdaptiveDifficulty();
	}

	CalculateFrustrationIndex();
	SaveBalanceData();
}

void UMGBalancingSubsystem::SetEconomyBalance(const FMGEconomyBalance& Balance)
{
	EconomyBalance = Balance;
	SaveBalanceData();
}

float UMGBalancingSubsystem::CalculateAdjustedPrice(float BasePrice, EMGBalanceCategory Category) const
{
	float Multiplier = 1.0f;

	switch (Category)
	{
	case EMGBalanceCategory::Economy:
		Multiplier = EconomyBalance.VehiclePriceMultiplier;
		break;
	default:
		Multiplier = GetParameter(FName(TEXT("PriceMultiplier")));
		break;
	}

	return BasePrice * Multiplier * (1.0f + EconomyBalance.InflationRate);
}

float UMGBalancingSubsystem::CalculateAdjustedReward(float BaseReward) const
{
	float DifficultyMultiplier = CurrentDifficultyProfile.RewardMultiplier;
	float AdaptiveMultiplier = AdaptiveDifficultyState.bEnabled ? AdaptiveDifficultyState.CurrentModifier : 1.0f;

	return BaseReward * DifficultyMultiplier * AdaptiveMultiplier;
}

void UMGBalancingSubsystem::UpdatePlayerMetrics(const FMGPlayerMetrics& Metrics)
{
	PlayerMetrics = Metrics;
	CalculateFrustrationIndex();
	SaveBalanceData();
}

EMGDifficultyTier UMGBalancingSubsystem::RecommendDifficultyTier() const
{
	float SkillRating = PlayerMetrics.SkillRating;
	float WinRate = PlayerMetrics.WinRate;

	if (PlayerMetrics.TotalRaces < 5)
		return EMGDifficultyTier::Beginner;

	if (SkillRating < 800 || WinRate < 0.1f)
		return EMGDifficultyTier::Casual;
	else if (SkillRating < 1000 || WinRate < 0.2f)
		return EMGDifficultyTier::Normal;
	else if (SkillRating < 1200 || WinRate < 0.3f)
		return EMGDifficultyTier::Competitive;
	else if (SkillRating < 1500 || WinRate < 0.4f)
		return EMGDifficultyTier::Expert;
	else
		return EMGDifficultyTier::Master;
}

bool UMGBalancingSubsystem::IsPlayerFrustrated() const
{
	return PlayerMetrics.FrustrationIndex > 0.7f ||
		   PlayerMetrics.ConsecutiveLosses >= 5 ||
		   (PlayerMetrics.WinRate < 0.05f && PlayerMetrics.TotalRaces > 10);
}

bool UMGBalancingSubsystem::IsPlayerBored() const
{
	return PlayerMetrics.ConsecutiveWins >= 7 ||
		   (PlayerMetrics.WinRate > 0.6f && PlayerMetrics.TotalRaces > 20) ||
		   PlayerMetrics.EngagementScore < 0.3f;
}

void UMGBalancingSubsystem::FetchRemoteConfig()
{
	// Would fetch from remote config service
	// For now, simulate
	bHasPendingRemoteConfig = false;
}

void UMGBalancingSubsystem::ApplyRemoteOverrides()
{
	for (const auto& Pair : PendingRemoteOverrides)
	{
		if (FMGBalanceParameter* Param = Parameters.Find(Pair.Key))
		{
			Param->RemoteOverride = Pair.Value;
			Param->bUseRemoteOverride = true;
		}
	}

	PendingRemoteOverrides.Empty();
	bHasPendingRemoteConfig = false;
	OnRemoteConfigReceived.Broadcast();
}

void UMGBalancingSubsystem::TakeSnapshot(const FString& Notes)
{
	FMGBalanceSnapshot Snapshot;
	Snapshot.Timestamp = FDateTime::UtcNow();
	Snapshot.Notes = Notes;
	Snapshot.PlayerMetrics = PlayerMetrics;

	for (const auto& Pair : Parameters)
	{
		Snapshot.ParameterValues.Add(Pair.Key, Pair.Value.CurrentValue);
	}

	Snapshots.Add(Snapshot);

	// Keep only last 20 snapshots
	if (Snapshots.Num() > 20)
	{
		Snapshots.RemoveAt(0);
	}
}

void UMGBalancingSubsystem::RestoreSnapshot(int32 Index)
{
	if (Snapshots.IsValidIndex(Index))
	{
		const FMGBalanceSnapshot& Snapshot = Snapshots[Index];

		for (const auto& Pair : Snapshot.ParameterValues)
		{
			SetParameter(Pair.Key, Pair.Value);
		}

		PlayerMetrics = Snapshot.PlayerMetrics;
	}
}

void UMGBalancingSubsystem::InitializeDefaultParameters()
{
	// Economy Parameters
	FMGBalanceParameter RaceEarnings;
	RaceEarnings.ParameterID = FName(TEXT("BaseRaceEarnings"));
	RaceEarnings.DisplayName = FText::FromString(TEXT("Base Race Earnings"));
	RaceEarnings.Category = EMGBalanceCategory::Economy;
	RaceEarnings.BaseValue = 1000.0f;
	RaceEarnings.CurrentValue = 1000.0f;
	RaceEarnings.MinValue = 100.0f;
	RaceEarnings.MaxValue = 10000.0f;
	Parameters.Add(RaceEarnings.ParameterID, RaceEarnings);

	FMGBalanceParameter PriceMultiplier;
	PriceMultiplier.ParameterID = FName(TEXT("PriceMultiplier"));
	PriceMultiplier.DisplayName = FText::FromString(TEXT("Global Price Multiplier"));
	PriceMultiplier.Category = EMGBalanceCategory::Economy;
	PriceMultiplier.BaseValue = 1.0f;
	PriceMultiplier.CurrentValue = 1.0f;
	PriceMultiplier.MinValue = 0.5f;
	PriceMultiplier.MaxValue = 2.0f;
	Parameters.Add(PriceMultiplier.ParameterID, PriceMultiplier);

	// Difficulty Parameters
	FMGBalanceParameter AIAggression;
	AIAggression.ParameterID = FName(TEXT("AIAggression"));
	AIAggression.DisplayName = FText::FromString(TEXT("AI Aggression Level"));
	AIAggression.Category = EMGBalanceCategory::AI;
	AIAggression.BaseValue = 1.0f;
	AIAggression.CurrentValue = 1.0f;
	AIAggression.MinValue = 0.1f;
	AIAggression.MaxValue = 2.0f;
	Parameters.Add(AIAggression.ParameterID, AIAggression);

	FMGBalanceParameter RubberBanding;
	RubberBanding.ParameterID = FName(TEXT("RubberBanding"));
	RubberBanding.DisplayName = FText::FromString(TEXT("Rubber Banding Strength"));
	RubberBanding.Category = EMGBalanceCategory::AI;
	RubberBanding.BaseValue = 0.5f;
	RubberBanding.CurrentValue = 0.5f;
	RubberBanding.MinValue = 0.0f;
	RubberBanding.MaxValue = 1.0f;
	Parameters.Add(RubberBanding.ParameterID, RubberBanding);

	// Progression Parameters
	FMGBalanceParameter XPMultiplier;
	XPMultiplier.ParameterID = FName(TEXT("XPMultiplier"));
	XPMultiplier.DisplayName = FText::FromString(TEXT("XP Gain Multiplier"));
	XPMultiplier.Category = EMGBalanceCategory::Progression;
	XPMultiplier.BaseValue = 1.0f;
	XPMultiplier.CurrentValue = 1.0f;
	XPMultiplier.MinValue = 0.5f;
	XPMultiplier.MaxValue = 3.0f;
	Parameters.Add(XPMultiplier.ParameterID, XPMultiplier);

	FMGBalanceParameter UnlockRate;
	UnlockRate.ParameterID = FName(TEXT("UnlockRate"));
	UnlockRate.DisplayName = FText::FromString(TEXT("Content Unlock Rate"));
	UnlockRate.Category = EMGBalanceCategory::Progression;
	UnlockRate.BaseValue = 1.0f;
	UnlockRate.CurrentValue = 1.0f;
	UnlockRate.MinValue = 0.5f;
	UnlockRate.MaxValue = 2.0f;
	Parameters.Add(UnlockRate.ParameterID, UnlockRate);

	// Matchmaking Parameters
	FMGBalanceParameter SkillBandwidth;
	SkillBandwidth.ParameterID = FName(TEXT("SkillBandwidth"));
	SkillBandwidth.DisplayName = FText::FromString(TEXT("Matchmaking Skill Bandwidth"));
	SkillBandwidth.Category = EMGBalanceCategory::Matchmaking;
	SkillBandwidth.BaseValue = 200.0f;
	SkillBandwidth.CurrentValue = 200.0f;
	SkillBandwidth.MinValue = 50.0f;
	SkillBandwidth.MaxValue = 500.0f;
	Parameters.Add(SkillBandwidth.ParameterID, SkillBandwidth);
}

void UMGBalancingSubsystem::InitializeDifficultyProfiles()
{
	// Beginner
	FMGDifficultyProfile Beginner;
	Beginner.Tier = EMGDifficultyTier::Beginner;
	Beginner.AIAggressionMultiplier = 0.5f;
	Beginner.AIRubberBandingStrength = 0.8f;
	Beginner.RewardMultiplier = 1.2f;
	Beginner.ProgressionSpeedMultiplier = 1.3f;
	Beginner.OpponentSkillVariance = 0.1f;
	Beginner.bEnableAssists = true;
	Beginner.bShowRacingLine = true;
	Beginner.bAutoTransmission = true;
	DifficultyProfiles.Add(EMGDifficultyTier::Beginner, Beginner);

	// Casual
	FMGDifficultyProfile Casual;
	Casual.Tier = EMGDifficultyTier::Casual;
	Casual.AIAggressionMultiplier = 0.7f;
	Casual.AIRubberBandingStrength = 0.6f;
	Casual.RewardMultiplier = 1.1f;
	Casual.ProgressionSpeedMultiplier = 1.15f;
	Casual.OpponentSkillVariance = 0.15f;
	Casual.bEnableAssists = true;
	Casual.bShowRacingLine = true;
	Casual.bAutoTransmission = true;
	DifficultyProfiles.Add(EMGDifficultyTier::Casual, Casual);

	// Normal
	FMGDifficultyProfile Normal;
	Normal.Tier = EMGDifficultyTier::Normal;
	Normal.AIAggressionMultiplier = 1.0f;
	Normal.AIRubberBandingStrength = 0.4f;
	Normal.RewardMultiplier = 1.0f;
	Normal.ProgressionSpeedMultiplier = 1.0f;
	Normal.OpponentSkillVariance = 0.2f;
	Normal.bEnableAssists = true;
	Normal.bShowRacingLine = false;
	Normal.bAutoTransmission = true;
	DifficultyProfiles.Add(EMGDifficultyTier::Normal, Normal);

	// Competitive
	FMGDifficultyProfile Competitive;
	Competitive.Tier = EMGDifficultyTier::Competitive;
	Competitive.AIAggressionMultiplier = 1.2f;
	Competitive.AIRubberBandingStrength = 0.2f;
	Competitive.RewardMultiplier = 1.15f;
	Competitive.ProgressionSpeedMultiplier = 0.9f;
	Competitive.OpponentSkillVariance = 0.25f;
	Competitive.bEnableAssists = false;
	Competitive.bShowRacingLine = false;
	Competitive.bAutoTransmission = false;
	DifficultyProfiles.Add(EMGDifficultyTier::Competitive, Competitive);

	// Expert
	FMGDifficultyProfile Expert;
	Expert.Tier = EMGDifficultyTier::Expert;
	Expert.AIAggressionMultiplier = 1.4f;
	Expert.AIRubberBandingStrength = 0.1f;
	Expert.RewardMultiplier = 1.25f;
	Expert.ProgressionSpeedMultiplier = 0.8f;
	Expert.OpponentSkillVariance = 0.3f;
	Expert.bEnableAssists = false;
	Expert.bShowRacingLine = false;
	Expert.bAutoTransmission = false;
	DifficultyProfiles.Add(EMGDifficultyTier::Expert, Expert);

	// Master
	FMGDifficultyProfile Master;
	Master.Tier = EMGDifficultyTier::Master;
	Master.AIAggressionMultiplier = 1.6f;
	Master.AIRubberBandingStrength = 0.0f;
	Master.RewardMultiplier = 1.5f;
	Master.ProgressionSpeedMultiplier = 0.7f;
	Master.OpponentSkillVariance = 0.35f;
	Master.bEnableAssists = false;
	Master.bShowRacingLine = false;
	Master.bAutoTransmission = false;
	DifficultyProfiles.Add(EMGDifficultyTier::Master, Master);
}

void UMGBalancingSubsystem::LoadBalanceData()
{
	// Would load from local/cloud save
}

void UMGBalancingSubsystem::SaveBalanceData()
{
	// Would save to local/cloud
}

void UMGBalancingSubsystem::UpdateAdaptiveDifficulty()
{
	AdaptiveDifficultyState.RacesSinceLastAdjustment++;

	// Only adjust every 3 races minimum
	if (AdaptiveDifficultyState.RacesSinceLastAdjustment < 3)
		return;

	float CurrentWinRate = PlayerMetrics.WinRate;
	float TargetWinRate = AdaptiveDifficultyState.TargetWinRate;
	float Difference = CurrentWinRate - TargetWinRate;

	// If winning too much, increase difficulty
	// If losing too much, decrease difficulty
	float Adjustment = -Difference * AdaptiveDifficultyState.AdjustmentSpeed;
	float NewModifier = FMath::Clamp(
		AdaptiveDifficultyState.CurrentModifier + Adjustment,
		AdaptiveDifficultyState.MinModifier,
		AdaptiveDifficultyState.MaxModifier
	);

	if (!FMath::IsNearlyEqual(AdaptiveDifficultyState.CurrentModifier, NewModifier, 0.01f))
	{
		AdaptiveDifficultyState.CurrentModifier = NewModifier;
		AdaptiveDifficultyState.RacesSinceLastAdjustment = 0;
		OnAdaptiveDifficultyAdjusted.Broadcast(NewModifier);
	}
}

void UMGBalancingSubsystem::CalculateFrustrationIndex()
{
	float FrustrationScore = 0.0f;

	// Consecutive losses contribute heavily
	FrustrationScore += FMath::Min(PlayerMetrics.ConsecutiveLosses * 0.15f, 0.6f);

	// Low win rate contributes
	if (PlayerMetrics.WinRate < 0.1f && PlayerMetrics.TotalRaces > 10)
		FrustrationScore += 0.2f;

	// High DNF rate contributes
	FrustrationScore += PlayerMetrics.DNFRate * 0.3f;

	// Low podium rate
	if (PlayerMetrics.PodiumRate < 0.2f && PlayerMetrics.TotalRaces > 10)
		FrustrationScore += 0.1f;

	PlayerMetrics.FrustrationIndex = FMath::Clamp(FrustrationScore, 0.0f, 1.0f);

	// Update engagement inversely
	PlayerMetrics.EngagementScore = 1.0f - (PlayerMetrics.FrustrationIndex * 0.5f);
}

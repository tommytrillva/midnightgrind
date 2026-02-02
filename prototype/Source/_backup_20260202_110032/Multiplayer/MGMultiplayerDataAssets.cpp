// Copyright Midnight Grind. All Rights Reserved.

#include "Multiplayer/MGMultiplayerDataAssets.h"

FMGMatchmakingBracket UMGMultiplayerSettingsAsset::GetBracketForReputation(int32 Reputation, bool bRanked) const
{
	const TArray<FMGMatchmakingBracket>& Brackets = bRanked ? RankedBrackets : QuickMatchBrackets;

	for (const FMGMatchmakingBracket& Bracket : Brackets)
	{
		if (Reputation >= Bracket.MinReputation && Reputation <= Bracket.MaxReputation)
		{
			return Bracket;
		}
	}

	// Return first bracket as fallback
	if (Brackets.Num() > 0)
	{
		return Brackets[0];
	}

	return FMGMatchmakingBracket();
}

void UMGMultiplayerSettingsAsset::CalculateRewards(int32 Position, bool bRanked, bool bCleanRace, bool bPersonalBest, int32& OutCash, int32& OutReputation) const
{
	OutCash = 0;
	OutReputation = 0;

	// Get base rewards for position (0-indexed internally)
	int32 PositionIndex = Position - 1;

	if (PositionIndex >= 0 && PositionIndex < RaceRewards.CashByPosition.Num())
	{
		OutCash = RaceRewards.CashByPosition[PositionIndex];
	}

	if (PositionIndex >= 0 && PositionIndex < RaceRewards.ReputationByPosition.Num())
	{
		OutReputation = RaceRewards.ReputationByPosition[PositionIndex];
	}

	// Add bonuses
	if (bCleanRace)
	{
		OutCash += RaceRewards.CleanRaceBonus;
	}

	if (bPersonalBest)
	{
		OutCash += RaceRewards.PersonalBestBonus;
	}

	// Apply ranked multiplier
	if (bRanked)
	{
		OutCash = FMath::RoundToInt(OutCash * RaceRewards.RankedMultiplier);
		OutReputation = FMath::RoundToInt(OutReputation * RaceRewards.RankedMultiplier);
	}
}

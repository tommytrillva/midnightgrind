// Copyright Midnight Grind. All Rights Reserved.

#include "AI/MGAIDriverProfile.h"

// ==========================================
// UMGAIDriverProfile
// ==========================================

int32 UMGAIDriverProfile::GetOverallRating() const
{
	float Rating = 0.0f;

	// Skill contribution (40%)
	Rating += Skill.SkillLevel * 40.0f;

	// Speed contribution (30%)
	Rating += Speed.BaseSpeedPercent * 30.0f;

	// Consistency contribution (20%)
	Rating += Skill.Consistency * 20.0f;

	// Racecraft contribution (10%)
	float RacecraftAvg = (Racecraft.Awareness + Racecraft.Anticipation + Racecraft.GapFinding) / 3.0f;
	Rating += RacecraftAvg * 10.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

int32 UMGAIDriverProfile::GetSpeedRating() const
{
	float Rating = 0.0f;

	Rating += Speed.BaseSpeedPercent * 50.0f;
	Rating += Speed.CornerSpeedMultiplier * 25.0f;
	Rating += Skill.CornerExitSpeed * 25.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

int32 UMGAIDriverProfile::GetAggressionRating() const
{
	float Rating = 0.0f;

	Rating += Aggression.Aggression * 40.0f;
	Rating += Aggression.OvertakeAggression * 30.0f;
	Rating += Aggression.RiskTaking * 30.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

int32 UMGAIDriverProfile::GetConsistencyRating() const
{
	float Rating = 0.0f;

	Rating += Skill.Consistency * 50.0f;
	Rating += (1.0f - Skill.MistakeFrequency) * 30.0f;
	Rating += Skill.RecoverySkill * 20.0f;

	return FMath::RoundToInt(FMath::Clamp(Rating, 0.0f, 100.0f));
}

void UMGAIDriverProfile::ApplyDifficultyModifier(float Modifier)
{
	// Adjust skill parameters
	Skill.SkillLevel = FMath::Clamp(Skill.SkillLevel * Modifier, 0.0f, 1.0f);
	Skill.BrakingAccuracy = FMath::Clamp(Skill.BrakingAccuracy * Modifier, 0.0f, 1.0f);
	Skill.LineAccuracy = FMath::Clamp(Skill.LineAccuracy * Modifier, 0.0f, 1.0f);
	Skill.Consistency = FMath::Clamp(Skill.Consistency * Modifier, 0.0f, 1.0f);

	// Adjust reaction time (inverse - lower modifier = slower reactions)
	Skill.ReactionTime = FMath::Clamp(Skill.ReactionTime / Modifier, 0.1f, 1.0f);

	// Adjust speed
	Speed.BaseSpeedPercent = FMath::Clamp(Speed.BaseSpeedPercent * Modifier, 0.5f, 1.1f);
	Speed.CornerSpeedMultiplier = FMath::Clamp(Speed.CornerSpeedMultiplier * Modifier, 0.7f, 1.1f);

	// Adjust mistake frequency (inverse)
	Skill.MistakeFrequency = FMath::Clamp(Skill.MistakeFrequency / Modifier, 0.0f, 1.0f);
}

// ==========================================
// UMGAIDriverRoster
// ==========================================

TArray<UMGAIDriverProfile*> UMGAIDriverRoster::GetRandomDrivers(int32 Count, float MinSkill, float MaxSkill) const
{
	TArray<UMGAIDriverProfile*> EligibleDrivers;

	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver && Driver->Skill.SkillLevel >= MinSkill && Driver->Skill.SkillLevel <= MaxSkill)
		{
			EligibleDrivers.Add(Driver);
		}
	}

	// Shuffle
	for (int32 i = EligibleDrivers.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		EligibleDrivers.Swap(i, j);
	}

	// Take requested count
	TArray<UMGAIDriverProfile*> Result;
	for (int32 i = 0; i < FMath::Min(Count, EligibleDrivers.Num()); i++)
	{
		Result.Add(EligibleDrivers[i]);
	}

	return Result;
}

TArray<UMGAIDriverProfile*> UMGAIDriverRoster::GetDriversByPersonality(EMGDriverPersonality Personality) const
{
	TArray<UMGAIDriverProfile*> Result;

	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver && Driver->Personality == Personality)
		{
			Result.Add(Driver);
		}
	}

	return Result;
}

UMGAIDriverProfile* UMGAIDriverRoster::GetDriverByName(const FString& Name) const
{
	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver && Driver->ShortName == Name)
		{
			return Driver;
		}
	}
	return nullptr;
}

TArray<UMGAIDriverProfile*> UMGAIDriverRoster::GetDriversForBracket(int32 PlayerRating, int32 Variance) const
{
	TArray<UMGAIDriverProfile*> Result;

	int32 MinRating = PlayerRating - Variance;
	int32 MaxRating = PlayerRating + Variance;

	for (UMGAIDriverProfile* Driver : Drivers)
	{
		if (Driver)
		{
			int32 DriverRating = Driver->GetOverallRating();
			if (DriverRating >= MinRating && DriverRating <= MaxRating)
			{
				Result.Add(Driver);
			}
		}
	}

	return Result;
}

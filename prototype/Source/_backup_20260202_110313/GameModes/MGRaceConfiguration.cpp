// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/MGRaceConfiguration.h"

UMGRaceConfiguration::UMGRaceConfiguration()
{
	// Initialize with sensible defaults for a fun 30-60 min session
	RaceID = NAME_None;
	DisplayName = FText::FromString(TEXT("Untitled Race"));
	RaceType = EMGRaceType::Circuit;
	NumberOfLaps = 3;
	NumberOfOpponents = 5;
	BaseDifficulty = 0.5f;
	TimeOfDay = 20.0f; // 8 PM - midnight grind hours
	bEnableRubberBanding = true;
	bAllowRespawn = true;
}

FPrimaryAssetId UMGRaceConfiguration::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("RaceConfiguration"), GetFName());
}

FMGRaceConfig UMGRaceConfiguration::CreateRaceConfig() const
{
	FMGRaceConfig Config;
	Config.TrackID = TrackID;
	Config.RaceType = RaceType;
	Config.NumberOfLaps = NumberOfLaps;
	Config.TimeLimitSeconds = TimeLimitSeconds;
	Config.MaxRacers = NumberOfOpponents + 1; // +1 for player
	Config.bAllowRespawn = bAllowRespawn;
	Config.RespawnPenaltySeconds = RespawnPenaltySeconds;

	return Config;
}

bool UMGRaceConfiguration::CanPlayerAccess(int32 PlayerLevel, int32 PlayerReputation, const TArray<FName>& CompletedRaces) const
{
	// Check level requirement
	if (PlayerLevel < RequiredLevel)
	{
		return false;
	}

	// Check reputation requirement
	if (PlayerReputation < RequiredReputation)
	{
		return false;
	}

	// Check prerequisite races
	for (const FName& RequiredRace : RequiredCompletedRaces)
	{
		if (!CompletedRaces.Contains(RequiredRace))
		{
			return false;
		}
	}

	return true;
}

FString UMGRaceConfiguration::GetTimeOfDayString() const
{
	int32 Hour = FMath::FloorToInt(TimeOfDay);
	int32 Minute = FMath::FloorToInt((TimeOfDay - Hour) * 60.0f);

	FString Period = (Hour >= 12) ? TEXT("PM") : TEXT("AM");
	int32 DisplayHour = Hour % 12;
	if (DisplayHour == 0) DisplayHour = 12;

	return FString::Printf(TEXT("%d:%02d %s"), DisplayHour, Minute, *Period);
}

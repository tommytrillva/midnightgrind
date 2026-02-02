// Copyright Midnight Grind. All Rights Reserved.

#include "Progression/MGPlayerProgression.h"

void UMGPlayerProgression::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize default crew reputations
	for (int32 i = 1; i <= static_cast<int32>(EMGCrew::Apex); ++i)
	{
		EMGCrew Crew = static_cast<EMGCrew>(i);
		FMGCrewReputation Rep;
		Rep.Crew = Crew;
		CrewReputations.Add(Crew, Rep);
	}

	UE_LOG(LogTemp, Log, TEXT("MGPlayerProgression initialized"));
}

void UMGPlayerProgression::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// LEVEL & XP
// ==========================================

void UMGPlayerProgression::AddXP(int64 Amount, bool bNotify)
{
	if (Amount <= 0)
	{
		return;
	}

	LevelProgression.CurrentXP += Amount;
	LevelProgression.TotalXPEarned += Amount;

	if (bNotify)
	{
		OnXPGained.Broadcast(Amount, LevelProgression.TotalXPEarned);
	}

	CheckLevelUp();
}

void UMGPlayerProgression::CheckLevelUp()
{
	int32 OldLevel = LevelProgression.CurrentLevel;
	bool bLeveledUp = false;

	while (LevelProgression.CurrentXP >= LevelProgression.GetXPForNextLevel())
	{
		LevelProgression.CurrentXP -= LevelProgression.GetXPForNextLevel();
		LevelProgression.CurrentLevel++;
		bLeveledUp = true;
	}

	if (bLeveledUp)
	{
		OnLevelUp.Broadcast(LevelProgression.CurrentLevel, OldLevel);

		// Check for new unlocks
		CheckAndGrantNewUnlocks();

		UE_LOG(LogTemp, Log, TEXT("Player leveled up! %d -> %d"), OldLevel, LevelProgression.CurrentLevel);
	}
}

// ==========================================
// REPUTATION
// ==========================================

int32 UMGPlayerProgression::GetCrewReputation(EMGCrew Crew) const
{
	if (const FMGCrewReputation* Rep = CrewReputations.Find(Crew))
	{
		return Rep->ReputationPoints;
	}
	return 0;
}

EMGReputationTier UMGPlayerProgression::GetCrewReputationTier(EMGCrew Crew) const
{
	if (const FMGCrewReputation* Rep = CrewReputations.Find(Crew))
	{
		return Rep->Tier;
	}
	return EMGReputationTier::Unknown;
}

void UMGPlayerProgression::AddCrewReputation(EMGCrew Crew, int32 Amount)
{
	if (Crew == EMGCrew::None || Amount == 0)
	{
		return;
	}

	FMGCrewReputation& Rep = GetOrCreateCrewReputation(Crew);
	Rep.ReputationPoints += Amount;
	Rep.ReputationPoints = FMath::Max(0, Rep.ReputationPoints);

	EMGReputationTier OldTier = Rep.Tier;
	Rep.Tier = CalculateReputationTier(Rep.ReputationPoints);

	OnReputationChanged.Broadcast(Crew, Rep.ReputationPoints, Rep.Tier);

	if (OldTier != Rep.Tier)
	{
		// Check for new unlocks when tier changes
		CheckAndGrantNewUnlocks();

		UE_LOG(LogTemp, Log, TEXT("Reputation tier changed with %s: %s -> %s"),
			*GetCrewName(Crew).ToString(),
			*GetReputationTierName(OldTier).ToString(),
			*GetReputationTierName(Rep.Tier).ToString());
	}
}

int32 UMGPlayerProgression::GetTotalReputation() const
{
	int32 Total = 0;
	for (const auto& Pair : CrewReputations)
	{
		Total += Pair.Value.ReputationPoints;
	}
	return Total;
}

EMGCrew UMGPlayerProgression::GetPrimaryCrew() const
{
	EMGCrew Primary = EMGCrew::None;
	int32 HighestRep = 0;

	for (const auto& Pair : CrewReputations)
	{
		if (Pair.Value.ReputationPoints > HighestRep)
		{
			HighestRep = Pair.Value.ReputationPoints;
			Primary = Pair.Key;
		}
	}

	return Primary;
}

TArray<FMGCrewReputation> UMGPlayerProgression::GetAllCrewReputations() const
{
	TArray<FMGCrewReputation> Result;
	for (const auto& Pair : CrewReputations)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

FText UMGPlayerProgression::GetReputationTierName(EMGReputationTier Tier)
{
	switch (Tier)
	{
		case EMGReputationTier::Unknown:	return NSLOCTEXT("Reputation", "Unknown", "Unknown");
		case EMGReputationTier::Rookie:		return NSLOCTEXT("Reputation", "Rookie", "Rookie");
		case EMGReputationTier::Known:		return NSLOCTEXT("Reputation", "Known", "Known");
		case EMGReputationTier::Respected:	return NSLOCTEXT("Reputation", "Respected", "Respected");
		case EMGReputationTier::Feared:		return NSLOCTEXT("Reputation", "Feared", "Feared");
		case EMGReputationTier::Legend:		return NSLOCTEXT("Reputation", "Legend", "Legend");
		default:							return NSLOCTEXT("Reputation", "Unknown", "Unknown");
	}
}

FText UMGPlayerProgression::GetCrewName(EMGCrew Crew)
{
	switch (Crew)
	{
		case EMGCrew::None:		return NSLOCTEXT("Crew", "None", "Unaffiliated");
		case EMGCrew::Midnight:	return NSLOCTEXT("Crew", "Midnight", "Midnight Runners");
		case EMGCrew::Velocity:	return NSLOCTEXT("Crew", "Velocity", "Team Velocity");
		case EMGCrew::Chrome:	return NSLOCTEXT("Crew", "Chrome", "Chrome Kings");
		case EMGCrew::Shadow:	return NSLOCTEXT("Crew", "Shadow", "Shadow Syndicate");
		case EMGCrew::Apex:		return NSLOCTEXT("Crew", "Apex", "Apex Racing");
		default:				return NSLOCTEXT("Crew", "Unknown", "Unknown");
	}
}

EMGReputationTier UMGPlayerProgression::CalculateReputationTier(int32 ReputationPoints)
{
	if (ReputationPoints >= 8000) return EMGReputationTier::Legend;
	if (ReputationPoints >= 4000) return EMGReputationTier::Feared;
	if (ReputationPoints >= 1500) return EMGReputationTier::Respected;
	if (ReputationPoints >= 500)  return EMGReputationTier::Known;
	if (ReputationPoints >= 100)  return EMGReputationTier::Rookie;
	return EMGReputationTier::Unknown;
}

FMGCrewReputation& UMGPlayerProgression::GetOrCreateCrewReputation(EMGCrew Crew)
{
	if (!CrewReputations.Contains(Crew))
	{
		FMGCrewReputation NewRep;
		NewRep.Crew = Crew;
		CrewReputations.Add(Crew, NewRep);
	}
	return CrewReputations[Crew];
}

// ==========================================
// UNLOCKS
// ==========================================

bool UMGPlayerProgression::IsUnlocked(FName UnlockID) const
{
	for (const FMGUnlock& Unlock : Unlocks)
	{
		if (Unlock.UnlockID == UnlockID)
		{
			return true;
		}
	}
	return false;
}

bool UMGPlayerProgression::MeetsUnlockRequirements(const FMGUnlockRequirement& Requirement) const
{
	// Check level
	if (LevelProgression.CurrentLevel < Requirement.RequiredLevel)
	{
		return false;
	}

	// Check crew reputation
	if (Requirement.RequiredCrew != EMGCrew::None && Requirement.RequiredCrewReputation > 0)
	{
		if (GetCrewReputation(Requirement.RequiredCrew) < Requirement.RequiredCrewReputation)
		{
			return false;
		}
	}

	// Check wins
	if (Statistics.TotalWins < Requirement.RequiredWins)
	{
		return false;
	}

	// Check races
	if (Statistics.TotalRaces < Requirement.RequiredRaces)
	{
		return false;
	}

	// Check prerequisite unlocks
	for (const FName& PrereqID : Requirement.RequiredUnlocks)
	{
		if (!IsUnlocked(PrereqID))
		{
			return false;
		}
	}

	return true;
}

bool UMGPlayerProgression::GrantUnlock(const FMGUnlock& Unlock)
{
	if (IsUnlocked(Unlock.UnlockID))
	{
		return false; // Already unlocked
	}

	FMGUnlock NewUnlock = Unlock;
	NewUnlock.UnlockedAt = FDateTime::Now();
	Unlocks.Add(NewUnlock);

	OnUnlockAcquired.Broadcast(NewUnlock);

	UE_LOG(LogTemp, Log, TEXT("Unlocked: %s"), *Unlock.DisplayName.ToString());
	return true;
}

TArray<FMGUnlock> UMGPlayerProgression::GetUnlocksByType(EMGUnlockType Type) const
{
	TArray<FMGUnlock> Result;
	for (const FMGUnlock& Unlock : Unlocks)
	{
		if (Unlock.Type == Type)
		{
			Result.Add(Unlock);
		}
	}
	return Result;
}

TArray<FMGUnlock> UMGPlayerProgression::CheckAndGrantNewUnlocks()
{
	TArray<FMGUnlock> NewUnlocks;

	for (const FMGUnlockRequirement& Req : UnlockRequirements)
	{
		if (!IsUnlocked(Req.UnlockID) && MeetsUnlockRequirements(Req))
		{
			FMGUnlock NewUnlock;
			NewUnlock.UnlockID = Req.UnlockID;
			NewUnlock.Type = Req.Type;
			NewUnlock.DisplayName = Req.DisplayName;

			if (GrantUnlock(NewUnlock))
			{
				NewUnlocks.Add(NewUnlock);
			}
		}
	}

	return NewUnlocks;
}

// ==========================================
// STATISTICS
// ==========================================

void UMGPlayerProgression::RecordRaceResult(int32 Position, int32 TotalRacers, EMGCrew RaceCrew, FName RaceTypeID)
{
	Statistics.TotalRaces++;

	bool bWon = Position == 1;
	bool bPodium = Position <= 3;

	if (bWon)
	{
		Statistics.TotalWins++;
	}

	if (bPodium)
	{
		Statistics.TotalPodiums++;
	}

	// Track race type
	if (RaceTypeID == "Circuit")
	{
		Statistics.CircuitRaces++;
	}
	else if (RaceTypeID == "Sprint")
	{
		Statistics.SprintRaces++;
	}
	else if (RaceTypeID == "Drift")
	{
		Statistics.DriftEvents++;
	}
	else if (RaceTypeID == "Drag")
	{
		Statistics.DragRaces++;
	}
	else if (RaceTypeID == "TimeTrial")
	{
		Statistics.TimeTrials++;
	}

	// Update crew stats
	if (RaceCrew != EMGCrew::None)
	{
		FMGCrewReputation& Rep = GetOrCreateCrewReputation(RaceCrew);
		Rep.RacesForCrew++;
		if (bWon)
		{
			Rep.WinsForCrew++;
		}

		// Grant reputation based on performance
		int32 RepGain = 0;
		if (bWon)
		{
			RepGain = 100;
		}
		else if (bPodium)
		{
			RepGain = 50;
		}
		else if (Position <= TotalRacers / 2)
		{
			RepGain = 25;
		}
		else
		{
			RepGain = 10;
		}

		AddCrewReputation(RaceCrew, RepGain);
	}

	// Grant XP based on performance
	int64 XPGain = 50; // Base XP for finishing
	if (bWon)
	{
		XPGain += 200;
	}
	else if (bPodium)
	{
		XPGain += 100;
	}

	// Bonus XP for beating more opponents
	XPGain += (TotalRacers - Position) * 10;

	AddXP(XPGain);
}

void UMGPlayerProgression::AddDistanceDriven(float DistanceKm)
{
	Statistics.TotalDistanceDrivenKm += DistanceKm;
}

void UMGPlayerProgression::RecordTopSpeed(float SpeedMPH)
{
	if (SpeedMPH > Statistics.TopSpeedAchievedMPH)
	{
		Statistics.TopSpeedAchievedMPH = SpeedMPH;
	}
}

void UMGPlayerProgression::RecordDriftScore(float Score)
{
	Statistics.TotalDriftScore += Score;
	if (Score > Statistics.BestDriftScore)
	{
		Statistics.BestDriftScore = Score;
	}
}

void UMGPlayerProgression::RecordPinkSlipResult(bool bWon)
{
	if (bWon)
	{
		Statistics.PinkSlipWins++;
	}
	else
	{
		Statistics.PinkSlipLosses++;
	}
}

void UMGPlayerProgression::AddPlayTime(float Seconds)
{
	Statistics.TotalPlayTime += FTimespan::FromSeconds(Seconds);
}

// ==========================================
// PLAYER PROFILE
// ==========================================

void UMGPlayerProgression::SetPlayerName(const FString& Name)
{
	if (!Name.IsEmpty() && Name.Len() <= 20)
	{
		PlayerName = Name;
	}
}

FText UMGPlayerProgression::GetPlayerTitle() const
{
	// Title based on level and reputation
	if (LevelProgression.CurrentLevel >= 50)
	{
		return NSLOCTEXT("Title", "Champion", "Street Champion");
	}
	else if (LevelProgression.CurrentLevel >= 30)
	{
		return NSLOCTEXT("Title", "Veteran", "Street Veteran");
	}
	else if (LevelProgression.CurrentLevel >= 15)
	{
		return NSLOCTEXT("Title", "Racer", "Street Racer");
	}
	else if (LevelProgression.CurrentLevel >= 5)
	{
		return NSLOCTEXT("Title", "Driver", "Underground Driver");
	}

	return NSLOCTEXT("Title", "Rookie", "Rookie");
}

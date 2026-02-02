// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGRivalsIntegration.h"
#include "GameModes/MGRaceGameMode.h"
#include "Career/MGCareerSubsystem.h"
#include "Progression/MGProgressionSubsystem.h"
#include "Engine/GameInstance.h"

void UMGRivalsIntegration::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CacheSubsystems();
}

void UMGRivalsIntegration::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// ENCOUNTER PROCESSING
// ==========================================

void UMGRivalsIntegration::ProcessRaceResults(const FMGRaceResults& RaceResults)
{
	// Find player's result
	int32 PlayerPosition = -1;
	float PlayerTime = 0.0f;

	for (const FMGRacerData& Racer : RaceResults.RacerResults)
	{
		if (!Racer.bIsAI)
		{
			PlayerPosition = Racer.Position;
			PlayerTime = Racer.TotalTime;
			break;
		}
	}

	if (PlayerPosition < 0)
	{
		return;
	}

	// Process each AI racer for potential rivalry updates
	for (const FMGRacerData& Racer : RaceResults.RacerResults)
	{
		if (Racer.bIsAI)
		{
			FName RacerID = FName(*Racer.DisplayName.ToString());

			// Check if this creates or updates a rivalry
			if (!IsRival(RacerID))
			{
				if (ShouldCreateRivalry(RacerID, RaceResults))
				{
					CreateRival(RacerID, Racer.DisplayName);
				}
				else
				{
					continue;
				}
			}

			// Determine event type
			float TimeGap = PlayerTime - Racer.TotalTime;
			EMGRivalryEvent EventType;

			if (PlayerPosition < Racer.Position)
			{
				// Player won against rival
				if (TimeGap < -10.0f)
				{
					EventType = EMGRivalryEvent::Domination;
				}
				else if (FMath::Abs(TimeGap) < 0.5f)
				{
					EventType = EMGRivalryEvent::PhotoFinish;
				}
				else
				{
					EventType = EMGRivalryEvent::Victory;
				}
			}
			else
			{
				// Player lost to rival
				if (TimeGap > 10.0f)
				{
					EventType = EMGRivalryEvent::Humiliation;
				}
				else if (FMath::Abs(TimeGap) < 0.5f)
				{
					EventType = EMGRivalryEvent::PhotoFinish;
				}
				else
				{
					EventType = EMGRivalryEvent::Defeat;
				}
			}

			RecordRivalryEvent(RacerID, EventType, TimeGap);
		}
	}
}

void UMGRivalsIntegration::RecordRivalryEvent(FName RivalID, EMGRivalryEvent EventType, float TimeGap)
{
	if (!RivalProfiles.Contains(RivalID))
	{
		return;
	}

	FMGRivalProfile& Profile = RivalProfiles[RivalID];

	// Create encounter record
	FMGRivalEncounter Encounter;
	Encounter.RivalID = RivalID;
	Encounter.EventType = EventType;
	Encounter.TimeGap = TimeGap;
	Encounter.Timestamp = FDateTime::UtcNow();

	// Calculate rivalry points
	int32 PointsDelta = CalculateRivalryPoints(EventType, TimeGap);
	Encounter.RivalryPointsDelta = PointsDelta;

	// Update profile stats
	Profile.TotalEncounters++;
	Profile.LastEncounter = Encounter.Timestamp;
	Profile.RivalryPoints += PointsDelta;

	// Update win/loss tracking
	switch (EventType)
	{
	case EMGRivalryEvent::Victory:
	case EMGRivalryEvent::Domination:
	case EMGRivalryEvent::LastSecondPass:
		Profile.WinsAgainst++;
		if (Profile.CurrentStreak >= 0)
		{
			Profile.CurrentStreak++;
		}
		else
		{
			Profile.CurrentStreak = 1;
		}
		break;

	case EMGRivalryEvent::Defeat:
	case EMGRivalryEvent::Humiliation:
	case EMGRivalryEvent::LastSecondLoss:
		Profile.LossesTo++;
		if (Profile.CurrentStreak <= 0)
		{
			Profile.CurrentStreak--;
		}
		else
		{
			Profile.CurrentStreak = -1;
		}
		break;

	case EMGRivalryEvent::PhotoFinish:
		// Split decision - slight rivalry boost but no streak change
		break;

	case EMGRivalryEvent::PinkSlipVictory:
		Profile.PinkSlipsWon++;
		Profile.WinsAgainst++;
		Profile.CurrentStreak = FMath::Max(1, Profile.CurrentStreak + 1);
		break;

	case EMGRivalryEvent::PinkSlipLoss:
		Profile.PinkSlipsLost++;
		Profile.LossesTo++;
		Profile.CurrentStreak = FMath::Min(-1, Profile.CurrentStreak - 1);
		break;

	default:
		break;
	}

	// Update best streak
	Profile.BestStreak = FMath::Max(Profile.BestStreak, FMath::Abs(Profile.CurrentStreak));

	// Check for streak events
	if (Profile.CurrentStreak >= 3)
	{
		Encounter.EventType = EMGRivalryEvent::WinStreak;
	}
	else if (Profile.CurrentStreak <= -3)
	{
		Encounter.EventType = EMGRivalryEvent::LossStreak;
	}

	// Store encounter
	TArray<FMGRivalEncounter>& History = EncounterHistory.FindOrAdd(RivalID);
	History.Add(Encounter);

	// Trim history if needed
	while (History.Num() > MaxEncounterHistory)
	{
		History.RemoveAt(0);
	}

	// Update intensity
	EMGRivalryIntensity OldIntensity = Profile.Intensity;
	UpdateRivalryIntensity(RivalID);

	// Broadcast encounter
	OnRivalEncounter.Broadcast(Encounter);

	// Check for intensity change
	if (Profile.Intensity != OldIntensity)
	{
		OnRivalryIntensityChanged.Broadcast(RivalID, Profile.Intensity);

		// Queue narrative trigger
		FString NarrativeID = FString::Printf(TEXT("Rivalry_%s_%d"),
			*RivalID.ToString(), static_cast<int32>(Profile.Intensity));
		PendingNarratives.Add(FName(*NarrativeID));
	}

	// Trigger taunt
	TriggerRivalTaunt(RivalID, EventType);

	// Check milestones
	CheckMilestones(RivalID);
}

// ==========================================
// RIVAL QUERIES
// ==========================================

TArray<FMGRivalProfile> UMGRivalsIntegration::GetAllRivals() const
{
	TArray<FMGRivalProfile> Result;
	RivalProfiles.GenerateValueArray(Result);
	return Result;
}

TArray<FMGRivalProfile> UMGRivalsIntegration::GetRivalsByIntensity(EMGRivalryIntensity Intensity) const
{
	TArray<FMGRivalProfile> Result;

	for (const auto& Pair : RivalProfiles)
	{
		if (Pair.Value.Intensity == Intensity)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGRivalProfile> UMGRivalsIntegration::GetTopRivals(int32 Count) const
{
	TArray<FMGRivalProfile> AllRivals;
	RivalProfiles.GenerateValueArray(AllRivals);

	// Sort by rivalry points (descending)
	AllRivals.Sort([](const FMGRivalProfile& A, const FMGRivalProfile& B)
	{
		return A.RivalryPoints > B.RivalryPoints;
	});

	// Return top N
	if (AllRivals.Num() > Count)
	{
		AllRivals.SetNum(Count);
	}

	return AllRivals;
}

FMGRivalProfile UMGRivalsIntegration::GetRivalProfile(FName RivalID) const
{
	if (const FMGRivalProfile* Profile = RivalProfiles.Find(RivalID))
	{
		return *Profile;
	}
	return FMGRivalProfile();
}

TArray<FMGRivalEncounter> UMGRivalsIntegration::GetEncounterHistory(FName RivalID, int32 Count) const
{
	TArray<FMGRivalEncounter> Result;

	if (const TArray<FMGRivalEncounter>* History = EncounterHistory.Find(RivalID))
	{
		int32 StartIndex = FMath::Max(0, History->Num() - Count);
		for (int32 i = History->Num() - 1; i >= StartIndex; --i)
		{
			Result.Add((*History)[i]);
		}
	}

	return Result;
}

FMGRivalProfile UMGRivalsIntegration::GetNemesis() const
{
	FMGRivalProfile Nemesis;
	int32 HighestPoints = 0;

	for (const auto& Pair : RivalProfiles)
	{
		if (Pair.Value.RivalryPoints > HighestPoints)
		{
			HighestPoints = Pair.Value.RivalryPoints;
			Nemesis = Pair.Value;
		}
	}

	return Nemesis;
}

bool UMGRivalsIntegration::IsRival(FName RacerID) const
{
	return RivalProfiles.Contains(RacerID);
}

// ==========================================
// MATCHMAKING INFLUENCE
// ==========================================

TArray<FName> UMGRivalsIntegration::GetRivalsForMatchmaking(int32 MaxCount) const
{
	TArray<FName> Result;

	// Get top rivals that should appear in matchmaking
	TArray<FMGRivalProfile> TopRivals = GetTopRivals(MaxCount * 2);

	for (const FMGRivalProfile& Rival : TopRivals)
	{
		if (ShouldIncludeRivalInRace(Rival.RivalID))
		{
			Result.Add(Rival.RivalID);
			if (Result.Num() >= MaxCount)
			{
				break;
			}
		}
	}

	return Result;
}

bool UMGRivalsIntegration::ShouldIncludeRivalInRace(FName RivalID) const
{
	if (const FMGRivalProfile* Profile = RivalProfiles.Find(RivalID))
	{
		// Higher intensity = higher chance of appearing
		float BaseChance = 0.0f;
		switch (Profile->Intensity)
		{
		case EMGRivalryIntensity::Acquaintance:
			BaseChance = 0.1f;
			break;
		case EMGRivalryIntensity::Competitor:
			BaseChance = 0.25f;
			break;
		case EMGRivalryIntensity::Rival:
			BaseChance = 0.5f;
			break;
		case EMGRivalryIntensity::Nemesis:
			BaseChance = 0.75f;
			break;
		case EMGRivalryIntensity::ArchNemesis:
			BaseChance = 0.9f;
			break;
		}

		// Boost chance if on a streak
		if (FMath::Abs(Profile->CurrentStreak) >= 2)
		{
			BaseChance += 0.2f;
		}

		// Boost if haven't seen recently
		FTimespan TimeSinceEncounter = FDateTime::UtcNow() - Profile->LastEncounter;
		if (TimeSinceEncounter.GetTotalHours() > 24)
		{
			BaseChance += 0.1f;
		}

		return FMath::FRand() < FMath::Min(BaseChance, 1.0f);
	}

	return false;
}

FName UMGRivalsIntegration::GetSuggestedRivalForEvent(FName EventID) const
{
	// Find a rival appropriate for the career event
	// Consider intensity, last encounter, and event type

	TArray<FMGRivalProfile> Candidates = GetTopRivals(10);

	for (const FMGRivalProfile& Rival : Candidates)
	{
		// Check if rival's preferred race types match event
		// For now, just return the top rival
		return Rival.RivalID;
	}

	return NAME_None;
}

// ==========================================
// NARRATIVE INTEGRATION
// ==========================================

TArray<FName> UMGRivalsIntegration::GetPendingRivalryNarratives() const
{
	return PendingNarratives;
}

void UMGRivalsIntegration::TriggerRivalTaunt(FName RivalID, EMGRivalryEvent Context)
{
	FText Taunt = GenerateTaunt(RivalID, Context);

	if (!Taunt.IsEmpty())
	{
		OnRivalTaunt.Broadcast(RivalID, Taunt);
	}
}

FString UMGRivalsIntegration::GetRivalryStoryState(FName RivalID) const
{
	if (const FMGRivalProfile* Profile = RivalProfiles.Find(RivalID))
	{
		// Return a story state identifier based on rivalry status
		FString State;

		switch (Profile->Intensity)
		{
		case EMGRivalryIntensity::Acquaintance:
			State = TEXT("NewFace");
			break;
		case EMGRivalryIntensity::Competitor:
			State = TEXT("GrowingTension");
			break;
		case EMGRivalryIntensity::Rival:
			State = TEXT("OpenRivalry");
			break;
		case EMGRivalryIntensity::Nemesis:
			State = TEXT("DeadlyEnemy");
			break;
		case EMGRivalryIntensity::ArchNemesis:
			State = TEXT("UltimateShowdown");
			break;
		}

		// Add streak modifier
		if (Profile->CurrentStreak >= 3)
		{
			State += TEXT("_Dominating");
		}
		else if (Profile->CurrentStreak <= -3)
		{
			State += TEXT("_BeingDominated");
		}

		// Add pink slip context
		if (Profile->PinkSlipsWon > 0)
		{
			State += TEXT("_TookTheirRide");
		}
		else if (Profile->PinkSlipsLost > 0)
		{
			State += TEXT("_LostMyRide");
		}

		return State;
	}

	return TEXT("Unknown");
}

// ==========================================
// MILESTONES
// ==========================================

TArray<FMGRivalryMilestone> UMGRivalsIntegration::GetAvailableMilestones() const
{
	TArray<FMGRivalryMilestone> Result;

	// Generate milestones based on current rivalry states
	for (const auto& Pair : RivalProfiles)
	{
		const FMGRivalProfile& Profile = Pair.Value;

		// First win milestone
		if (Profile.WinsAgainst == 0)
		{
			FMGRivalryMilestone Milestone;
			Milestone.MilestoneID = FName(*FString::Printf(TEXT("FirstWin_%s"), *Profile.RivalID.ToString()));
			Milestone.DisplayName = FText::Format(NSLOCTEXT("MG", "FirstWinMilestone", "First Victory vs {0}"),
				Profile.DisplayName);
			Milestone.Description = NSLOCTEXT("MG", "FirstWinDesc", "Beat this rival for the first time");
			Milestone.RivalID = Profile.RivalID;
			Milestone.RewardCredits = 1000;
			Milestone.RewardReputation = 25;
			Milestone.bCompleted = CompletedMilestones.Contains(Milestone.MilestoneID);
			Result.Add(Milestone);
		}

		// Win streak milestone
		if (Profile.BestStreak < 5)
		{
			FMGRivalryMilestone Milestone;
			Milestone.MilestoneID = FName(*FString::Printf(TEXT("WinStreak5_%s"), *Profile.RivalID.ToString()));
			Milestone.DisplayName = FText::Format(NSLOCTEXT("MG", "WinStreakMilestone", "Dominate {0}"),
				Profile.DisplayName);
			Milestone.Description = NSLOCTEXT("MG", "WinStreakDesc", "Win 5 races in a row against this rival");
			Milestone.RivalID = Profile.RivalID;
			Milestone.RewardCredits = 5000;
			Milestone.RewardReputation = 100;
			Milestone.bCompleted = CompletedMilestones.Contains(Milestone.MilestoneID);
			Result.Add(Milestone);
		}

		// Nemesis milestone
		if (Profile.Intensity < EMGRivalryIntensity::Nemesis)
		{
			FMGRivalryMilestone Milestone;
			Milestone.MilestoneID = FName(*FString::Printf(TEXT("Nemesis_%s"), *Profile.RivalID.ToString()));
			Milestone.DisplayName = FText::Format(NSLOCTEXT("MG", "NemesisMilestone", "Nemesis: {0}"),
				Profile.DisplayName);
			Milestone.Description = NSLOCTEXT("MG", "NemesisDesc", "Build rivalry to Nemesis level");
			Milestone.RivalID = Profile.RivalID;
			Milestone.RewardCredits = 10000;
			Milestone.RewardReputation = 200;
			Milestone.bCompleted = CompletedMilestones.Contains(Milestone.MilestoneID);
			Result.Add(Milestone);
		}
	}

	return Result;
}

TArray<FMGRivalryMilestone> UMGRivalsIntegration::GetCompletedMilestones() const
{
	TArray<FMGRivalryMilestone> Result;

	// Would return actual completed milestone data
	// For now, return empty

	return Result;
}

void UMGRivalsIntegration::CheckMilestones(FName RivalID)
{
	if (!RivalProfiles.Contains(RivalID))
	{
		return;
	}

	const FMGRivalProfile& Profile = RivalProfiles[RivalID];

	// Check first win
	FName FirstWinID = FName(*FString::Printf(TEXT("FirstWin_%s"), *RivalID.ToString()));
	if (Profile.WinsAgainst == 1 && !CompletedMilestones.Contains(FirstWinID))
	{
		CompletedMilestones.Add(FirstWinID);

		FMGRivalryMilestone Milestone;
		Milestone.MilestoneID = FirstWinID;
		Milestone.DisplayName = FText::Format(NSLOCTEXT("MG", "FirstWinMilestone", "First Victory vs {0}"),
			Profile.DisplayName);
		Milestone.RivalID = RivalID;
		Milestone.RewardCredits = 1000;
		Milestone.RewardReputation = 25;
		Milestone.bCompleted = true;

		OnRivalryMilestoneComplete.Broadcast(Milestone);
	}

	// Check win streak
	FName WinStreakID = FName(*FString::Printf(TEXT("WinStreak5_%s"), *RivalID.ToString()));
	if (Profile.CurrentStreak >= 5 && !CompletedMilestones.Contains(WinStreakID))
	{
		CompletedMilestones.Add(WinStreakID);

		FMGRivalryMilestone Milestone;
		Milestone.MilestoneID = WinStreakID;
		Milestone.DisplayName = FText::Format(NSLOCTEXT("MG", "WinStreakMilestone", "Dominate {0}"),
			Profile.DisplayName);
		Milestone.RivalID = RivalID;
		Milestone.RewardCredits = 5000;
		Milestone.RewardReputation = 100;
		Milestone.bCompleted = true;

		OnRivalryMilestoneComplete.Broadcast(Milestone);
	}

	// Check nemesis
	FName NemesisID = FName(*FString::Printf(TEXT("Nemesis_%s"), *RivalID.ToString()));
	if (Profile.Intensity >= EMGRivalryIntensity::Nemesis && !CompletedMilestones.Contains(NemesisID))
	{
		CompletedMilestones.Add(NemesisID);

		FMGRivalryMilestone Milestone;
		Milestone.MilestoneID = NemesisID;
		Milestone.DisplayName = FText::Format(NSLOCTEXT("MG", "NemesisMilestone", "Nemesis: {0}"),
			Profile.DisplayName);
		Milestone.RivalID = RivalID;
		Milestone.RewardCredits = 10000;
		Milestone.RewardReputation = 200;
		Milestone.bCompleted = true;

		OnRivalryMilestoneComplete.Broadcast(Milestone);
	}
}

// ==========================================
// INTERNAL
// ==========================================

void UMGRivalsIntegration::UpdateRivalryIntensity(FName RivalID)
{
	if (!RivalProfiles.Contains(RivalID))
	{
		return;
	}

	FMGRivalProfile& Profile = RivalProfiles[RivalID];

	EMGRivalryIntensity NewIntensity = EMGRivalryIntensity::Acquaintance;

	if (Profile.RivalryPoints >= ArchNemesisThreshold)
	{
		NewIntensity = EMGRivalryIntensity::ArchNemesis;
	}
	else if (Profile.RivalryPoints >= NemesisThreshold)
	{
		NewIntensity = EMGRivalryIntensity::Nemesis;
	}
	else if (Profile.RivalryPoints >= RivalThreshold)
	{
		NewIntensity = EMGRivalryIntensity::Rival;
	}
	else if (Profile.RivalryPoints >= CompetitorThreshold)
	{
		NewIntensity = EMGRivalryIntensity::Competitor;
	}

	Profile.Intensity = NewIntensity;
}

int32 UMGRivalsIntegration::CalculateRivalryPoints(EMGRivalryEvent EventType, float TimeGap) const
{
	int32 BasePoints = 0;

	switch (EventType)
	{
	case EMGRivalryEvent::FirstMeet:
		BasePoints = 10;
		break;
	case EMGRivalryEvent::Victory:
		BasePoints = 20;
		break;
	case EMGRivalryEvent::Defeat:
		BasePoints = 25; // Losses build rivalry more
		break;
	case EMGRivalryEvent::PhotoFinish:
		BasePoints = 40; // Close races = intense rivalry
		break;
	case EMGRivalryEvent::Domination:
		BasePoints = 15;
		break;
	case EMGRivalryEvent::Humiliation:
		BasePoints = 50; // Getting destroyed builds resentment
		break;
	case EMGRivalryEvent::PinkSlipVictory:
		BasePoints = 100;
		break;
	case EMGRivalryEvent::PinkSlipLoss:
		BasePoints = 150; // Losing car = maximum rivalry
		break;
	case EMGRivalryEvent::WinStreak:
		BasePoints = 30;
		break;
	case EMGRivalryEvent::LossStreak:
		BasePoints = 40;
		break;
	case EMGRivalryEvent::StreakBreaker:
		BasePoints = 50;
		break;
	case EMGRivalryEvent::LastSecondPass:
		BasePoints = 35;
		break;
	case EMGRivalryEvent::LastSecondLoss:
		BasePoints = 45;
		break;
	case EMGRivalryEvent::MutualDestruction:
		BasePoints = 30;
		break;
	}

	// Modify based on time gap
	if (FMath::Abs(TimeGap) < 1.0f)
	{
		BasePoints = static_cast<int32>(BasePoints * 1.5f);
	}

	return BasePoints;
}

FText UMGRivalsIntegration::GenerateTaunt(FName RivalID, EMGRivalryEvent Context) const
{
	// Dynamic taunt generation based on context
	TArray<FText> Taunts;

	switch (Context)
	{
	case EMGRivalryEvent::Victory:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Loss1", "Lucky win. Won't happen again."));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Loss2", "Enjoy it while it lasts."));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Loss3", "Next time, you're eating my dust."));
		break;

	case EMGRivalryEvent::Defeat:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Win1", "Too easy. Come back when you're ready."));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Win2", "That all you got?"));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Win3", "Maybe try a faster car?"));
		break;

	case EMGRivalryEvent::Domination:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Dom1", "...fluke. Total fluke."));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Dom2", "I wasn't even trying."));
		break;

	case EMGRivalryEvent::Humiliation:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Hum1", "HAHAHAHA! Go home!"));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Hum2", "Pathetic. Absolutely pathetic."));
		break;

	case EMGRivalryEvent::PhotoFinish:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Close1", "That was too close. Next time I'll destroy you."));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Close2", "You got lucky at the line."));
		break;

	case EMGRivalryEvent::PinkSlipVictory:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_PS1", "YOU TOOK MY CAR?! This isn't over!"));
		Taunts.Add(NSLOCTEXT("MG", "Taunt_PS2", "I'm coming for that car. And everything else."));
		break;

	case EMGRivalryEvent::WinStreak:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Streak1", "Stop. Please just stop."));
		break;

	default:
		Taunts.Add(NSLOCTEXT("MG", "Taunt_Default", "See you on the streets."));
		break;
	}

	if (Taunts.Num() > 0)
	{
		return Taunts[FMath::RandRange(0, Taunts.Num() - 1)];
	}

	return FText::GetEmpty();
}

bool UMGRivalsIntegration::ShouldCreateRivalry(FName RacerID, const FMGRaceResults& Results) const
{
	// Create rivalry if:
	// 1. Close finish (within 2 seconds)
	// 2. Pink slip race
	// 3. Multiple encounters in session

	// For now, create rivalry on any competitive finish
	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (FName(*Racer.DisplayName.ToString()) == RacerID)
		{
			// Find player for comparison
			for (const FMGRacerData& Player : Results.RacerResults)
			{
				if (!Player.bIsAI)
				{
					// Close positions = rivalry
					if (FMath::Abs(Racer.Position - Player.Position) <= 2)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

void UMGRivalsIntegration::CreateRival(FName RacerID, const FText& DisplayName)
{
	if (RivalProfiles.Contains(RacerID))
	{
		return;
	}

	FMGRivalProfile Profile;
	Profile.RivalID = RacerID;
	Profile.DisplayName = DisplayName;
	Profile.Intensity = EMGRivalryIntensity::Acquaintance;
	Profile.RivalryPoints = 0;
	Profile.LastEncounter = FDateTime::UtcNow();

	// Generate random catchphrase
	static const TArray<FText> Catchphrases = {
		NSLOCTEXT("MG", "Catch1", "Speed is everything."),
		NSLOCTEXT("MG", "Catch2", "Second place is first loser."),
		NSLOCTEXT("MG", "Catch3", "You can't handle these streets."),
		NSLOCTEXT("MG", "Catch4", "My ride, my rules."),
		NSLOCTEXT("MG", "Catch5", "Catch me if you can."),
		NSLOCTEXT("MG", "Catch6", "Born to race, built to win."),
		NSLOCTEXT("MG", "Catch7", "The night belongs to me."),
		NSLOCTEXT("MG", "Catch8", "Respect the grind.")
	};

	Profile.Catchphrase = Catchphrases[FMath::RandRange(0, Catchphrases.Num() - 1)];

	RivalProfiles.Add(RacerID, Profile);

	OnNewRivalDiscovered.Broadcast(Profile);
}

int32 UMGRivalsIntegration::GetIntensityThreshold(EMGRivalryIntensity Intensity) const
{
	switch (Intensity)
	{
	case EMGRivalryIntensity::Competitor:
		return CompetitorThreshold;
	case EMGRivalryIntensity::Rival:
		return RivalThreshold;
	case EMGRivalryIntensity::Nemesis:
		return NemesisThreshold;
	case EMGRivalryIntensity::ArchNemesis:
		return ArchNemesisThreshold;
	default:
		return 0;
	}
}

void UMGRivalsIntegration::CacheSubsystems()
{
	// Cache references to other subsystems via game instance
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		CareerSubsystem = GameInstance->GetSubsystem<UMGCareerSubsystem>();
		ProgressionSubsystem = GameInstance->GetSubsystem<UMGProgressionSubsystem>();

		if (!CareerSubsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("MGRivalsIntegration: CareerSubsystem not available"));
		}
		if (!ProgressionSubsystem)
		{
			UE_LOG(LogTemp, Warning, TEXT("MGRivalsIntegration: ProgressionSubsystem not available"));
		}
	}
}

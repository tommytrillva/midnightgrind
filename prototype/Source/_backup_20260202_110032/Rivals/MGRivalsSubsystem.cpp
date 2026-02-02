// Copyright Midnight Grind. All Rights Reserved.

#include "Rivals/MGRivalsSubsystem.h"

void UMGRivalsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadRivalData();
}

void UMGRivalsSubsystem::Deinitialize()
{
	SaveRivalData();
	Super::Deinitialize();
}

TArray<FMGRival> UMGRivalsSubsystem::GetActiveRivals() const
{
	TArray<FMGRival> Active;
	for (const FMGRival& Rival : Rivals)
	{
		if (Rival.Status == EMGRivalStatus::Active || Rival.Status == EMGRivalStatus::Dominant)
			Active.Add(Rival);
	}
	return Active;
}

FMGRival UMGRivalsSubsystem::GetRival(const FString& PlayerID) const
{
	const FMGRival* Found = Rivals.FindByPredicate(
		[&PlayerID](const FMGRival& R) { return R.RivalPlayerID == PlayerID; });
	return Found ? *Found : FMGRival();
}

bool UMGRivalsSubsystem::IsRival(const FString& PlayerID) const
{
	return Rivals.ContainsByPredicate(
		[&PlayerID](const FMGRival& R) { return R.RivalPlayerID == PlayerID && R.Intensity >= EMGRivalryIntensity::Competitor; });
}

FMGRival UMGRivalsSubsystem::GetNemesis() const
{
	if (CurrentNemesisID.IsEmpty())
		return FMGRival();

	return GetRival(CurrentNemesisID);
}

bool UMGRivalsSubsystem::HasNemesis() const
{
	return !CurrentNemesisID.IsEmpty();
}

void UMGRivalsSubsystem::RecordEncounter(const FMGRivalEncounter& Encounter)
{
	FMGRival* Rival = FindRival(Encounter.RivalPlayerID);

	if (!Rival)
	{
		// Create new rival entry
		FMGRival NewRival;
		NewRival.RivalPlayerID = Encounter.RivalPlayerID;
		NewRival.FirstEncounter = Encounter.Timestamp;
		NewRival.Intensity = EMGRivalryIntensity::Neutral;
		Rivals.Add(NewRival);
		Rival = &Rivals.Last();
	}

	// Update record
	Rival->Record.TotalRaces++;
	Rival->Record.LastRaceDate = Encounter.Timestamp;

	bool bYouWon = Encounter.YourPosition < Encounter.TheirPosition;

	if (bYouWon)
	{
		Rival->Record.YourWins++;
		Rival->Record.CurrentStreak = FMath::Max(1, Rival->Record.CurrentStreak + 1);
	}
	else
	{
		Rival->Record.TheirWins++;
		Rival->Record.CurrentStreak = FMath::Min(-1, Rival->Record.CurrentStreak - 1);
	}

	// Check for close race (adds intensity)
	if (FMath::Abs(Encounter.FinishTimeDifference) < 1.0f)
	{
		Rival->RivalryScore += 10; // Close races build rivalries
	}

	Rival->RivalryScore += 5; // Any encounter adds some score

	// Update intensity based on new score
	EMGRivalryIntensity OldIntensity = Rival->Intensity;
	UpdateRivalryIntensity(*Rival);

	if (Rival->Intensity > OldIntensity)
	{
		OnRivalryIntensified.Broadcast(*Rival, Rival->Intensity);

		if (OldIntensity < EMGRivalryIntensity::Competitor && Rival->Intensity >= EMGRivalryIntensity::Competitor)
		{
			OnNewRivalDiscovered.Broadcast(*Rival);
		}
	}

	// Broadcast defeat event
	if (bYouWon && Rival->Intensity >= EMGRivalryIntensity::Competitor)
	{
		OnRivalDefeated.Broadcast(*Rival, Encounter.bWasCloseRace);
	}

	// Update status
	if (Rival->Record.CurrentStreak >= 5)
		Rival->Status = EMGRivalStatus::Defeated;
	else if (Rival->Record.CurrentStreak <= -5)
		Rival->Status = EMGRivalStatus::Dominant;
	else
		Rival->Status = EMGRivalStatus::Active;

	CheckForNewNemesis();
	SaveRivalData();
}

void UMGRivalsSubsystem::OnRaceWithPlayer(const FString& PlayerID, const FText& PlayerName, int32 YourPosition, int32 TheirPosition, float TimeDiff)
{
	FMGRivalEncounter Encounter;
	Encounter.RivalPlayerID = PlayerID;
	Encounter.YourPosition = YourPosition;
	Encounter.TheirPosition = TheirPosition;
	Encounter.FinishTimeDifference = TimeDiff;
	Encounter.bWasCloseRace = FMath::Abs(TimeDiff) < 1.0f;
	Encounter.Timestamp = FDateTime::UtcNow();

	// Update name if we have a rival entry
	FMGRival* Rival = FindRival(PlayerID);
	if (Rival)
	{
		Rival->RivalName = PlayerName;
	}

	RecordEncounter(Encounter);
}

TArray<FMGRival> UMGRivalsSubsystem::GetRivalsByIntensity(EMGRivalryIntensity MinIntensity) const
{
	TArray<FMGRival> Result;
	for (const FMGRival& Rival : Rivals)
	{
		if (Rival.Intensity >= MinIntensity)
			Result.Add(Rival);
	}
	return Result;
}

FMGRivalRecord UMGRivalsSubsystem::GetRecordAgainst(const FString& PlayerID) const
{
	FMGRival Rival = GetRival(PlayerID);
	return Rival.Record;
}

int32 UMGRivalsSubsystem::GetTotalRivalryWins() const
{
	int32 Total = 0;
	for (const FMGRival& Rival : Rivals)
	{
		if (Rival.Intensity >= EMGRivalryIntensity::Competitor)
			Total += Rival.Record.YourWins;
	}
	return Total;
}

void UMGRivalsSubsystem::SetNemesis(const FString& PlayerID)
{
	FMGRival* Rival = FindRival(PlayerID);
	if (!Rival)
		return;

	// Clear previous nemesis
	if (!CurrentNemesisID.IsEmpty())
	{
		FMGRival* OldNemesis = FindRival(CurrentNemesisID);
		if (OldNemesis)
			OldNemesis->bIsNemesis = false;
	}

	CurrentNemesisID = PlayerID;
	Rival->bIsNemesis = true;
	Rival->Intensity = EMGRivalryIntensity::Nemesis;

	OnNemesisDesignated.Broadcast(*Rival);
	SaveRivalData();
}

void UMGRivalsSubsystem::ClearNemesis()
{
	if (!CurrentNemesisID.IsEmpty())
	{
		FMGRival* OldNemesis = FindRival(CurrentNemesisID);
		if (OldNemesis)
		{
			OldNemesis->bIsNemesis = false;
			UpdateRivalryIntensity(*OldNemesis);
		}
	}
	CurrentNemesisID.Empty();
	SaveRivalData();
}

void UMGRivalsSubsystem::OnPlayerJoinedSameCrew(const FString& PlayerID)
{
	FMGRival* Rival = FindRival(PlayerID);
	if (Rival && Rival->Intensity >= EMGRivalryIntensity::Competitor)
	{
		Rival->Status = EMGRivalStatus::Ally;

		if (Rival->bIsNemesis)
		{
			ClearNemesis();
		}

		OnRivalBecameAlly.Broadcast(*Rival);
		SaveRivalData();
	}
}

TArray<FString> UMGRivalsSubsystem::GetPreferredOpponents() const
{
	TArray<FString> Preferred;

	// Prioritize active rivals
	for (const FMGRival& Rival : Rivals)
	{
		if (Rival.Status == EMGRivalStatus::Active && Rival.Intensity >= EMGRivalryIntensity::Competitor)
		{
			Preferred.Add(Rival.RivalPlayerID);
		}
	}

	// Add nemesis at the top
	if (!CurrentNemesisID.IsEmpty())
	{
		Preferred.Remove(CurrentNemesisID);
		Preferred.Insert(CurrentNemesisID, 0);
	}

	return Preferred;
}

bool UMGRivalsSubsystem::ShouldPrioritizeRivalMatch() const
{
	// Prioritize rival matches if there are active intense rivalries
	for (const FMGRival& Rival : Rivals)
	{
		if (Rival.Status == EMGRivalStatus::Active && Rival.Intensity >= EMGRivalryIntensity::Rival)
			return true;
	}
	return false;
}

void UMGRivalsSubsystem::LoadRivalData()
{
	// Would load from cloud save
}

void UMGRivalsSubsystem::SaveRivalData()
{
	// Would save to cloud save
}

FMGRival* UMGRivalsSubsystem::FindRival(const FString& PlayerID)
{
	return Rivals.FindByPredicate(
		[&PlayerID](const FMGRival& R) { return R.RivalPlayerID == PlayerID; });
}

void UMGRivalsSubsystem::UpdateRivalryIntensity(FMGRival& Rival)
{
	int32 Score = CalculateRivalryScore(Rival);
	Rival.RivalryScore = Score;

	if (Rival.bIsNemesis)
	{
		Rival.Intensity = EMGRivalryIntensity::Nemesis;
	}
	else if (Score >= 100)
	{
		Rival.Intensity = EMGRivalryIntensity::Rival;
	}
	else if (Score >= 50)
	{
		Rival.Intensity = EMGRivalryIntensity::Competitor;
	}
	else if (Score >= 20)
	{
		Rival.Intensity = EMGRivalryIntensity::Acquaintance;
	}
	else
	{
		Rival.Intensity = EMGRivalryIntensity::Neutral;
	}
}

void UMGRivalsSubsystem::CheckForNewNemesis()
{
	if (HasNemesis())
		return;

	// Auto-designate nemesis if there's a dominant rival
	for (const FMGRival& Rival : Rivals)
	{
		if (Rival.Status == EMGRivalStatus::Dominant && Rival.Intensity >= EMGRivalryIntensity::Rival)
		{
			SetNemesis(Rival.RivalPlayerID);
			break;
		}
	}
}

int32 UMGRivalsSubsystem::CalculateRivalryScore(const FMGRival& Rival) const
{
	int32 Score = 0;

	// Race count contributes
	Score += Rival.Record.TotalRaces * 5;

	// Close records intensify rivalry
	int32 RecordDiff = FMath::Abs(Rival.Record.YourWins - Rival.Record.TheirWins);
	if (RecordDiff <= 2 && Rival.Record.TotalRaces >= 5)
	{
		Score += 30; // Competitive record bonus
	}

	// Recent activity bonus
	FTimespan TimeSinceLastRace = FDateTime::UtcNow() - Rival.Record.LastRaceDate;
	if (TimeSinceLastRace.GetTotalDays() < 7)
	{
		Score += 20;
	}

	// Streak intensity
	Score += FMath::Abs(Rival.Record.CurrentStreak) * 5;

	return Score;
}

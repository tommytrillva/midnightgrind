// Copyright Midnight Grind. All Rights Reserved.

#include "Reputation/MGReputationSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"

void UMGReputationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Initialize all categories
	for (int32 i = 0; i <= (int32)EMGReputationCategory::Competitive; i++)
	{
		FMGReputationLevel Level;
		Level.Category = (EMGReputationCategory)i;
		Level.Tier = EMGReputationTier::Unknown;
		ReputationLevels.Add((EMGReputationCategory)i, Level);
	}

	LoadReputationData();
	InitializeUnlocks();
	InitializeTitles();
}

void UMGReputationSubsystem::Deinitialize()
{
	SaveReputationData();
	Super::Deinitialize();
}

FMGReputationLevel UMGReputationSubsystem::GetReputationLevel(EMGReputationCategory Category) const
{
	const FMGReputationLevel* Level = ReputationLevels.Find(Category);
	return Level ? *Level : FMGReputationLevel();
}

int64 UMGReputationSubsystem::GetReputation(EMGReputationCategory Category) const
{
	FMGReputationLevel Level = GetReputationLevel(Category);
	return Level.CurrentReputation;
}

EMGReputationTier UMGReputationSubsystem::GetTier(EMGReputationCategory Category) const
{
	FMGReputationLevel Level = GetReputationLevel(Category);
	return Level.Tier;
}

FText UMGReputationSubsystem::GetTierName(EMGReputationTier Tier) const
{
	switch (Tier)
	{
	case EMGReputationTier::Unknown: return FText::FromString(TEXT("Unknown"));
	case EMGReputationTier::Rookie: return FText::FromString(TEXT("Rookie"));
	case EMGReputationTier::Regular: return FText::FromString(TEXT("Regular"));
	case EMGReputationTier::Respected: return FText::FromString(TEXT("Respected"));
	case EMGReputationTier::Elite: return FText::FromString(TEXT("Elite"));
	case EMGReputationTier::Legend: return FText::FromString(TEXT("Legend"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

void UMGReputationSubsystem::AddReputation(EMGReputationCategory Category, int64 Amount, const FString& Source)
{
	if (Amount <= 0)
		return;

	FMGReputationLevel* Level = ReputationLevels.Find(Category);
	if (!Level)
		return;

	Level->CurrentReputation += Amount;

	// Also add to overall
	if (Category != EMGReputationCategory::Overall)
	{
		FMGReputationLevel* OverallLevel = ReputationLevels.Find(EMGReputationCategory::Overall);
		if (OverallLevel)
		{
			OverallLevel->CurrentReputation += Amount;
			UpdateTier(EMGReputationCategory::Overall);
		}
	}

	// Record gain
	FMGReputationGain Gain;
	Gain.Category = Category;
	Gain.Amount = Amount;
	Gain.Source = Source;
	Gain.Timestamp = FDateTime::UtcNow();
	GainHistory.Add(Gain);

	while (GainHistory.Num() > MaxGainHistory)
	{
		GainHistory.RemoveAt(0);
	}

	UpdateTier(Category);
	OnReputationGained.Broadcast(Category, Amount);

	CheckUnlocks();
	CheckTitles();
	SaveReputationData();
}

void UMGReputationSubsystem::OnRaceCompleted(int32 Position, int32 TotalRacers, bool bWasCleanRace)
{
	// Racing reputation
	int64 RacingRep = 50; // Base participation

	if (Position == 1)
		RacingRep += 100;
	else if (Position == 2)
		RacingRep += 75;
	else if (Position == 3)
		RacingRep += 50;
	else if (Position <= TotalRacers / 2)
		RacingRep += 25;

	AddReputation(EMGReputationCategory::Racing, RacingRep, TEXT("Race completion"));

	// Technical reputation for clean racing
	if (bWasCleanRace)
	{
		int64 TechRep = 30;
		if (Position <= 3)
			TechRep += 20; // Bonus for clean podium
		AddReputation(EMGReputationCategory::Technical, TechRep, TEXT("Clean race"));
	}
}

void UMGReputationSubsystem::OnTournamentResult(int32 Position, int32 TotalParticipants)
{
	int64 CompRep = 100; // Base tournament participation

	if (Position == 1)
		CompRep += 500;
	else if (Position == 2)
		CompRep += 300;
	else if (Position == 3)
		CompRep += 200;
	else if (Position <= 10)
		CompRep += 100;

	// Scale with tournament size
	float SizeMultiplier = FMath::Clamp((float)TotalParticipants / 32.0f, 1.0f, 3.0f);
	CompRep = FMath::RoundToInt64(CompRep * SizeMultiplier);

	AddReputation(EMGReputationCategory::Competitive, CompRep, TEXT("Tournament"));
}

void UMGReputationSubsystem::OnCrewActivity(const FString& ActivityType)
{
	int64 SocialRep = 25; // Base crew activity

	if (ActivityType == TEXT("CrewRace"))
		SocialRep = 40;
	else if (ActivityType == TEXT("CrewWin"))
		SocialRep = 75;
	else if (ActivityType == TEXT("CrewEvent"))
		SocialRep = 100;

	AddReputation(EMGReputationCategory::Social, SocialRep, FString::Printf(TEXT("Crew: %s"), *ActivityType));
}

void UMGReputationSubsystem::OnCreationShared(bool bIsLivery, int32 Downloads)
{
	int64 CreativeRep = 50; // Base for sharing

	// Bonus based on downloads
	CreativeRep += Downloads * 5;

	// Cap per creation
	CreativeRep = FMath::Min(CreativeRep, (int64)500);

	AddReputation(EMGReputationCategory::Creative, CreativeRep, bIsLivery ? TEXT("Livery shared") : TEXT("Track shared"));
}

TArray<FMGReputationUnlock> UMGReputationSubsystem::GetUnlockedItems() const
{
	TArray<FMGReputationUnlock> Unlocked;
	for (const FMGReputationUnlock& Unlock : Unlocks)
	{
		if (Unlock.bUnlocked)
			Unlocked.Add(Unlock);
	}
	return Unlocked;
}

TArray<FMGReputationUnlock> UMGReputationSubsystem::GetPendingUnlocks() const
{
	TArray<FMGReputationUnlock> Pending;
	for (const FMGReputationUnlock& Unlock : Unlocks)
	{
		if (!Unlock.bUnlocked)
			Pending.Add(Unlock);
	}
	return Pending;
}

bool UMGReputationSubsystem::HasUnlock(FName UnlockID) const
{
	const FMGReputationUnlock* Unlock = Unlocks.FindByPredicate(
		[UnlockID](const FMGReputationUnlock& U) { return U.UnlockID == UnlockID; });
	return Unlock && Unlock->bUnlocked;
}

TArray<FMGReputationTitle> UMGReputationSubsystem::GetUnlockedTitles() const
{
	TArray<FMGReputationTitle> Unlocked;
	for (const FMGReputationTitle& Title : Titles)
	{
		if (Title.bUnlocked)
			Unlocked.Add(Title);
	}
	return Unlocked;
}

void UMGReputationSubsystem::EquipTitle(FName TitleID)
{
	// Unequip current
	for (FMGReputationTitle& Title : Titles)
	{
		Title.bEquipped = false;
	}

	// Equip new
	FMGReputationTitle* NewTitle = Titles.FindByPredicate(
		[TitleID](const FMGReputationTitle& T) { return T.TitleID == TitleID; });

	if (NewTitle && NewTitle->bUnlocked)
	{
		NewTitle->bEquipped = true;
		EquippedTitleID = TitleID;
		SaveReputationData();
	}
}

FMGReputationTitle UMGReputationSubsystem::GetEquippedTitle() const
{
	const FMGReputationTitle* Title = Titles.FindByPredicate(
		[](const FMGReputationTitle& T) { return T.bEquipped; });
	return Title ? *Title : FMGReputationTitle();
}

TArray<FMGReputationGain> UMGReputationSubsystem::GetRecentGains(int32 Count) const
{
	TArray<FMGReputationGain> Recent;
	int32 StartIndex = FMath::Max(0, GainHistory.Num() - Count);

	for (int32 i = GainHistory.Num() - 1; i >= StartIndex; i--)
	{
		Recent.Add(GainHistory[i]);
	}
	return Recent;
}

void UMGReputationSubsystem::LoadReputationData()
{
	// Would load from cloud save
}

void UMGReputationSubsystem::SaveReputationData()
{
	// Would save to cloud save
}

void UMGReputationSubsystem::InitializeUnlocks()
{
	// Respected tier unlocks
	FMGReputationUnlock NeonGarage;
	NeonGarage.UnlockID = FName(TEXT("Unlock_NeonGarage"));
	NeonGarage.UnlockName = FText::FromString(TEXT("Neon Garage"));
	NeonGarage.Description = FText::FromString(TEXT("Exclusive garage customization"));
	NeonGarage.RequiredTier = EMGReputationTier::Respected;
	Unlocks.Add(NeonGarage);

	// Elite tier unlocks
	FMGReputationUnlock EliteMatchmaking;
	EliteMatchmaking.UnlockID = FName(TEXT("Unlock_EliteQueue"));
	EliteMatchmaking.UnlockName = FText::FromString(TEXT("Elite Racing Queue"));
	EliteMatchmaking.Description = FText::FromString(TEXT("Access to elite-only races"));
	EliteMatchmaking.RequiredTier = EMGReputationTier::Elite;
	Unlocks.Add(EliteMatchmaking);

	// Legend tier unlocks
	FMGReputationUnlock LegendLivery;
	LegendLivery.UnlockID = FName(TEXT("Unlock_LegendLivery"));
	LegendLivery.UnlockName = FText::FromString(TEXT("Legend Livery Kit"));
	LegendLivery.Description = FText::FromString(TEXT("Exclusive legendary livery options"));
	LegendLivery.RequiredTier = EMGReputationTier::Legend;
	Unlocks.Add(LegendLivery);

	// Category-specific unlocks
	FMGReputationUnlock CleanRacerBadge;
	CleanRacerBadge.UnlockID = FName(TEXT("Unlock_CleanBadge"));
	CleanRacerBadge.UnlockName = FText::FromString(TEXT("Clean Racer Badge"));
	CleanRacerBadge.Description = FText::FromString(TEXT("Display badge for technical racing"));
	CleanRacerBadge.RequiredTier = EMGReputationTier::Respected;
	CleanRacerBadge.RequiredCategory = EMGReputationCategory::Technical;
	Unlocks.Add(CleanRacerBadge);
}

void UMGReputationSubsystem::InitializeTitles()
{
	// Rookie titles
	FMGReputationTitle RookieTitle;
	RookieTitle.TitleID = FName(TEXT("Title_Rookie"));
	RookieTitle.TitleText = FText::FromString(TEXT("Rookie Racer"));
	RookieTitle.RequiredTier = EMGReputationTier::Rookie;
	RookieTitle.bUnlocked = true; // Default title
	Titles.Add(RookieTitle);

	// Regular titles
	FMGReputationTitle StreetTitle;
	StreetTitle.TitleID = FName(TEXT("Title_Street"));
	StreetTitle.TitleText = FText::FromString(TEXT("Street Regular"));
	StreetTitle.RequiredTier = EMGReputationTier::Regular;
	Titles.Add(StreetTitle);

	// Respected titles
	FMGReputationTitle RespectedTitle;
	RespectedTitle.TitleID = FName(TEXT("Title_Respected"));
	RespectedTitle.TitleText = FText::FromString(TEXT("Respected Racer"));
	RespectedTitle.RequiredTier = EMGReputationTier::Respected;
	Titles.Add(RespectedTitle);

	// Elite titles
	FMGReputationTitle EliteTitle;
	EliteTitle.TitleID = FName(TEXT("Title_Elite"));
	EliteTitle.TitleText = FText::FromString(TEXT("Elite Driver"));
	EliteTitle.RequiredTier = EMGReputationTier::Elite;
	Titles.Add(EliteTitle);

	// Legend titles
	FMGReputationTitle LegendTitle;
	LegendTitle.TitleID = FName(TEXT("Title_Legend"));
	LegendTitle.TitleText = FText::FromString(TEXT("Street Legend"));
	LegendTitle.RequiredTier = EMGReputationTier::Legend;
	Titles.Add(LegendTitle);

	// Category-specific titles
	FMGReputationTitle TechMasterTitle;
	TechMasterTitle.TitleID = FName(TEXT("Title_TechMaster"));
	TechMasterTitle.TitleText = FText::FromString(TEXT("Technical Master"));
	TechMasterTitle.RequiredTier = EMGReputationTier::Elite;
	TechMasterTitle.RequiredCategory = EMGReputationCategory::Technical;
	Titles.Add(TechMasterTitle);

	FMGReputationTitle CrewChampTitle;
	CrewChampTitle.TitleID = FName(TEXT("Title_CrewChamp"));
	CrewChampTitle.TitleText = FText::FromString(TEXT("Crew Champion"));
	CrewChampTitle.RequiredTier = EMGReputationTier::Respected;
	CrewChampTitle.RequiredCategory = EMGReputationCategory::Social;
	Titles.Add(CrewChampTitle);
}

void UMGReputationSubsystem::UpdateTier(EMGReputationCategory Category)
{
	FMGReputationLevel* Level = ReputationLevels.Find(Category);
	if (!Level)
		return;

	EMGReputationTier OldTier = Level->Tier;
	int64 Rep = Level->CurrentReputation;

	if (Rep >= GetReputationForTier(EMGReputationTier::Legend))
		Level->Tier = EMGReputationTier::Legend;
	else if (Rep >= GetReputationForTier(EMGReputationTier::Elite))
		Level->Tier = EMGReputationTier::Elite;
	else if (Rep >= GetReputationForTier(EMGReputationTier::Respected))
		Level->Tier = EMGReputationTier::Respected;
	else if (Rep >= GetReputationForTier(EMGReputationTier::Regular))
		Level->Tier = EMGReputationTier::Regular;
	else if (Rep >= GetReputationForTier(EMGReputationTier::Rookie))
		Level->Tier = EMGReputationTier::Rookie;
	else
		Level->Tier = EMGReputationTier::Unknown;

	// Calculate progress to next tier
	if (Level->Tier != EMGReputationTier::Legend)
	{
		EMGReputationTier NextTier = (EMGReputationTier)((int32)Level->Tier + 1);
		int64 CurrentTierRep = GetReputationForTier(Level->Tier);
		int64 NextTierRep = GetReputationForTier(NextTier);

		Level->ReputationToNextTier = NextTierRep - Rep;
		Level->TierProgressPercent = 100.0f * ((float)(Rep - CurrentTierRep) / (float)(NextTierRep - CurrentTierRep));
	}
	else
	{
		Level->ReputationToNextTier = 0;
		Level->TierProgressPercent = 100.0f;
	}

	if (Level->Tier != OldTier)
	{
		OnTierReached.Broadcast(Category, Level->Tier);
	}
}

void UMGReputationSubsystem::CheckUnlocks()
{
	for (FMGReputationUnlock& Unlock : Unlocks)
	{
		if (Unlock.bUnlocked)
			continue;

		EMGReputationTier CurrentTier = GetTier(Unlock.RequiredCategory);
		if (CurrentTier >= Unlock.RequiredTier)
		{
			Unlock.bUnlocked = true;
			OnUnlockEarned.Broadcast(Unlock);
		}
	}
}

void UMGReputationSubsystem::CheckTitles()
{
	for (FMGReputationTitle& Title : Titles)
	{
		if (Title.bUnlocked)
			continue;

		EMGReputationTier CurrentTier = GetTier(Title.RequiredCategory);
		if (CurrentTier >= Title.RequiredTier)
		{
			Title.bUnlocked = true;
			OnTitleUnlocked.Broadcast(Title);
		}
	}
}

int64 UMGReputationSubsystem::GetReputationForTier(EMGReputationTier Tier) const
{
	switch (Tier)
	{
	case EMGReputationTier::Unknown: return 0;
	case EMGReputationTier::Rookie: return 100;
	case EMGReputationTier::Regular: return 1000;
	case EMGReputationTier::Respected: return 5000;
	case EMGReputationTier::Elite: return 25000;
	case EMGReputationTier::Legend: return 100000;
	default: return 0;
	}
}

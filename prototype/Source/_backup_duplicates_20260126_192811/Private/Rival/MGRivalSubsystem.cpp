// Copyright Midnight Grind. All Rights Reserved.

#include "Rival/MGRivalSubsystem.h"

void UMGRivalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitializeStoryRivals();
}

void UMGRivalSubsystem::Deinitialize()
{
	Rivals.Empty();
	Rivalries.Empty();
	Super::Deinitialize();
}

void UMGRivalSubsystem::InitializeStoryRivals()
{
	// ==================== CHAPTER 1: NEWCOMER ====================

	// "Razor" Ray Martinez - First major rival, tutorial boss
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("Razor");
		Rival.DisplayName = FText::FromString(TEXT("Ray \"Razor\" Martinez"));
		Rival.Nickname = FText::FromString(TEXT("Razor"));
		Rival.Backstory = FText::FromString(TEXT("Local hotshot who thinks he owns the streets. Quick to dismiss newcomers but secretly worried about losing his rep. His Civic is fast, but his ego is faster."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Nuisance;
		Rival.Personality = EMGRivalPersonality::Cocky;
		Rival.SignatureVehicleID = TEXT("HondaCivicEK");
		Rival.VehicleDescription = FText::FromString(TEXT("Modified '99 Civic EK with a built B18C swap"));
		Rival.CorneringSkill = 0.65f;
		Rival.StraightLineSkill = 0.6f;
		Rival.AggressionFactor = 0.4f;
		Rival.DirtyTacticsTendency = 0.2f;
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("Another rookie thinking they can hang. I'll have you crying before the first turn.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("Nice car. It'll look even better in my rearview.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("Hope you didn't waste your whole paycheck on that thing.")));

		Rival.WinLines.Add(FText::FromString(TEXT("And THAT'S why they call me Razor. Clean cut, every time.")));
		Rival.WinLines.Add(FText::FromString(TEXT("Maybe try the go-kart track first, rookie.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("Whatever. My car wasn't running right anyway.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("Lucky break. Won't happen again.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("You cheated. Had to be cheating.")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("Not bad... for a rookie. But I still took it.")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// ==================== CHAPTER 2: RISING ====================

	// "Viper" Vanessa Chen - Female rival, calculated and dangerous
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("Viper");
		Rival.DisplayName = FText::FromString(TEXT("Vanessa \"Viper\" Chen"));
		Rival.Nickname = FText::FromString(TEXT("Viper"));
		Rival.Backstory = FText::FromString(TEXT("Former time attack champion who moved to the streets for bigger money. Cold, calculating, and rarely makes mistakes. Treats racing like chess."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Contender;
		Rival.Personality = EMGRivalPersonality::Calculating;
		Rival.SignatureVehicleID = TEXT("NissanSilvia");
		Rival.VehicleDescription = FText::FromString(TEXT("Pristine S15 Silvia with a built SR20DET"));
		Rival.CorneringSkill = 0.85f;
		Rival.StraightLineSkill = 0.7f;
		Rival.AggressionFactor = 0.3f;
		Rival.DirtyTacticsTendency = 0.1f;
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("I've studied your racing. Three critical weaknesses. This won't take long.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("Interesting. You've improved. But improvement isn't victory.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("Strategy beats raw talent. Every time.")));

		Rival.WinLines.Add(FText::FromString(TEXT("Predictable. You telegraphed every move.")));
		Rival.WinLines.Add(FText::FromString(TEXT("The outcome was never in question.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("...Unexpected. I'll need to recalculate.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("You're more adaptable than my data suggested.")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("Hmm. A variable I didn't account for. Interesting.")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// "Tank" Tommy O'Brien - Aggressive muscle car rival
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("Tank");
		Rival.DisplayName = FText::FromString(TEXT("Tommy \"Tank\" O'Brien"));
		Rival.Nickname = FText::FromString(TEXT("Tank"));
		Rival.Backstory = FText::FromString(TEXT("Third-generation street racer with old-school muscle. Believes in raw power over finesse. Will absolutely put you in the wall if you don't give him room."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Contender;
		Rival.Personality = EMGRivalPersonality::Aggressive;
		Rival.SignatureVehicleID = TEXT("DodgeChallenger");
		Rival.VehicleDescription = FText::FromString(TEXT("Supercharged '70 Challenger with modern internals"));
		Rival.CorneringSkill = 0.55f;
		Rival.StraightLineSkill = 0.9f;
		Rival.AggressionFactor = 0.9f;
		Rival.DirtyTacticsTendency = 0.6f;
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("That little toy car's gonna get crushed. Nothing personal.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("Real racing is done in INCHES. Hope you got insurance.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("My dad raced. His dad raced. You? You're just passing through.")));

		Rival.WinLines.Add(FText::FromString(TEXT("THAT'S how it's done. American muscle, baby!")));
		Rival.WinLines.Add(FText::FromString(TEXT("Next time, bring a real car.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("Bull. You only won 'cause you can't race a straight line.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("*spits* Won't happen again.")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("I'll give you that one. You got guts.")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// ==================== CHAPTER 3: CONTENDER ====================

	// "Ghost" - Silent, legendary underground racer
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("GhostRacer");
		Rival.DisplayName = FText::FromString(TEXT("Ghost"));
		Rival.Nickname = FText::FromString(TEXT("Ghost"));
		Rival.Backstory = FText::FromString(TEXT("Nobody knows their real name or face. Shows up, wins, disappears. Some say Ghost used to be a pro driver. Others say they're running from something. All that matters is they're nearly unbeatable."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Dangerous;
		Rival.Personality = EMGRivalPersonality::Silent;
		Rival.SignatureVehicleID = TEXT("NissanGTR");
		Rival.VehicleDescription = FText::FromString(TEXT("Murdered-out R34 GT-R, modified beyond recognition"));
		Rival.CorneringSkill = 0.9f;
		Rival.StraightLineSkill = 0.88f;
		Rival.AggressionFactor = 0.2f;
		Rival.DirtyTacticsTendency = 0.0f;
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("...")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("*nods*")));

		Rival.WinLines.Add(FText::FromString(TEXT("...")));

		Rival.LoseLines.Add(FText::FromString(TEXT("*a slight nod of respect*")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("*the faintest hint of a smile*")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// "King" Marcus Webb - Crew leader, territorial
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("King");
		Rival.DisplayName = FText::FromString(TEXT("Marcus \"King\" Webb"));
		Rival.Nickname = FText::FromString(TEXT("King"));
		Rival.Backstory = FText::FromString(TEXT("Rules the Southside with an iron fist. Built his crew from nothing and treats any challenge as disrespect. Losing to him means losing access to his territory - and he controls the best racing spots."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Dangerous;
		Rival.Personality = EMGRivalPersonality::Vengeful;
		Rival.SignatureVehicleID = TEXT("DodgeCharger");
		Rival.VehicleDescription = FText::FromString(TEXT("Blacked-out Charger Hellcat with custom everything"));
		Rival.CorneringSkill = 0.75f;
		Rival.StraightLineSkill = 0.85f;
		Rival.AggressionFactor = 0.7f;
		Rival.DirtyTacticsTendency = 0.4f;
		Rival.CrewID = TEXT("SouthsideKings");
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("You're in MY house now. Show some respect.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("I built this scene. You're just visiting.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("After I'm done, you won't show your face in Southside again.")));

		Rival.WinLines.Add(FText::FromString(TEXT("Know your place. I run these streets.")));
		Rival.WinLines.Add(FText::FromString(TEXT("The King stays King. Always.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("This ain't over. Not by a long shot.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("One race doesn't make you anything. Remember that.")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("You got skills. But skill ain't everything out here.")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// ==================== CHAPTER 4: CHAMPION ====================

	// "Empress" Yuki Tanaka - International legend
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("Empress");
		Rival.DisplayName = FText::FromString(TEXT("Yuki \"Empress\" Tanaka"));
		Rival.Nickname = FText::FromString(TEXT("Empress"));
		Rival.Backstory = FText::FromString(TEXT("Daughter of a Japanese racing dynasty. Dominated the Tokyo scene before conquering every major city. Respects pure skill above all else and races with honor - but that honor makes her terrifying."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Nemesis;
		Rival.Personality = EMGRivalPersonality::Respectful;
		Rival.SignatureVehicleID = TEXT("ToyotaSupra");
		Rival.VehicleDescription = FText::FromString(TEXT("Pearl white A80 Supra with a legendary 2JZ build"));
		Rival.CorneringSkill = 0.92f;
		Rival.StraightLineSkill = 0.9f;
		Rival.AggressionFactor = 0.4f;
		Rival.DirtyTacticsTendency = 0.0f;
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("I've heard of your victories. Now show me if they were deserved.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("May this race bring honor to us both.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("No tricks. No games. Just two drivers and the road.")));

		Rival.WinLines.Add(FText::FromString(TEXT("You have potential. Keep training.")));
		Rival.WinLines.Add(FText::FromString(TEXT("A good race. But I expect more next time.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("Impressive. You have earned this victory.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("*bows* You drove with honor. Thank you for this race.")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("THIS is why I race. For moments like this.")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// ==================== CHAPTER 5: LEGEND ====================

	// "Phantom" - The final boss, former champion
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("Phantom");
		Rival.DisplayName = FText::FromString(TEXT("Phantom"));
		Rival.Nickname = FText::FromString(TEXT("Phantom"));
		Rival.Backstory = FText::FromString(TEXT("The undefeated legend who disappeared five years ago at the height of their fame. They've returned to reclaim their throne. No one has ever beaten them. Will you be the first?"));
		Rival.ThreatLevel = EMGRivalThreatLevel::Legend;
		Rival.Personality = EMGRivalPersonality::Silent;
		Rival.SignatureVehicleID = TEXT("MazdaRX7");
		Rival.VehicleDescription = FText::FromString(TEXT("The legendary FD RX-7, rotary screaming, unchanged since their last race"));
		Rival.CorneringSkill = 0.98f;
		Rival.StraightLineSkill = 0.95f;
		Rival.AggressionFactor = 0.5f;
		Rival.DirtyTacticsTendency = 0.0f;
		Rival.bIsStoryRival = true;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("Five years. The streets have changed. The cars have changed. But racing... racing never changes.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("They say you're the new best. Show me.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("I came back for one reason. Let's see if you're it.")));

		Rival.WinLines.Add(FText::FromString(TEXT("Close. But close isn't enough. Not against me.")));
		Rival.WinLines.Add(FText::FromString(TEXT("The throne remains mine. For now.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("...Finally. Someone worthy.")));
		Rival.LoseLines.Add(FText::FromString(TEXT("The streets have a new king. Take care of them.")));

		Rival.CloseRaceLines.Add(FText::FromString(TEXT("I haven't felt this alive in five years.")));

		Rivals.Add(Rival.RivalID, Rival);
		StoryRivalOrder.Add(Rival.RivalID);
	}

	// ==================== NON-STORY RIVALS ====================

	// "Drift King" Danny - Optional drift specialist
	{
		FMGRival Rival;
		Rival.RivalID = TEXT("DriftKing");
		Rival.DisplayName = FText::FromString(TEXT("Danny \"Drift King\" Park"));
		Rival.Nickname = FText::FromString(TEXT("Drift King"));
		Rival.Backstory = FText::FromString(TEXT("Obsessed with style over substance. His drift game is unmatched but he's not the fastest in a straight line. Loves showing off to crowds."));
		Rival.ThreatLevel = EMGRivalThreatLevel::Contender;
		Rival.Personality = EMGRivalPersonality::Showboat;
		Rival.SignatureVehicleID = TEXT("Nissan350Z");
		Rival.VehicleDescription = FText::FromString(TEXT("Widebody 350Z covered in sponsor decals"));
		Rival.CorneringSkill = 0.88f;
		Rival.StraightLineSkill = 0.6f;
		Rival.AggressionFactor = 0.3f;
		Rival.DirtyTacticsTendency = 0.1f;
		Rival.bIsStoryRival = false;

		Rival.PreRaceLines.Add(FText::FromString(TEXT("Hope there's a crowd. I put on a SHOW.")));
		Rival.PreRaceLines.Add(FText::FromString(TEXT("Fastest ain't best. STYLE is best.")));

		Rival.WinLines.Add(FText::FromString(TEXT("Did you SEE that angle? Clean AF.")));

		Rival.LoseLines.Add(FText::FromString(TEXT("Yeah but I LOOKED better doing it.")));

		Rivals.Add(Rival.RivalID, Rival);
	}
}

// ==================== Rival Discovery ====================

TArray<FMGRival> UMGRivalSubsystem::GetAllRivals() const
{
	TArray<FMGRival> Result;
	Rivals.GenerateValueArray(Result);
	return Result;
}

TArray<FMGRival> UMGRivalSubsystem::GetStoryRivals() const
{
	TArray<FMGRival> Result;
	for (const auto& Pair : Rivals)
	{
		if (Pair.Value.bIsStoryRival)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

bool UMGRivalSubsystem::GetRival(FName RivalID, FMGRival& OutRival) const
{
	if (const FMGRival* Found = Rivals.Find(RivalID))
	{
		OutRival = *Found;
		return true;
	}
	return false;
}

TArray<FMGRival> UMGRivalSubsystem::GetRivalsByThreatLevel(EMGRivalThreatLevel ThreatLevel) const
{
	TArray<FMGRival> Result;
	for (const auto& Pair : Rivals)
	{
		if (Pair.Value.ThreatLevel == ThreatLevel)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGRival> UMGRivalSubsystem::GetRivalsInCrew(FName CrewID) const
{
	TArray<FMGRival> Result;
	for (const auto& Pair : Rivals)
	{
		if (Pair.Value.CrewID == CrewID)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// ==================== Rivalry Management ====================

FMGRivalry UMGRivalSubsystem::GetRivalry(FName RivalID) const
{
	if (const FMGRivalry* Found = Rivalries.Find(RivalID))
	{
		return *Found;
	}

	FMGRivalry Default;
	Default.RivalID = RivalID;
	return Default;
}

TArray<FMGRivalry> UMGRivalSubsystem::GetActiveRivalries() const
{
	TArray<FMGRivalry> Result;
	for (const auto& Pair : Rivalries)
	{
		if (Pair.Value.bIsActive && Pair.Value.TotalRaces > 0)
		{
			Result.Add(Pair.Value);
		}
	}

	// Sort by intensity
	Result.Sort([](const FMGRivalry& A, const FMGRivalry& B)
	{
		return static_cast<uint8>(A.Intensity) > static_cast<uint8>(B.Intensity);
	});

	return Result;
}

FName UMGRivalSubsystem::GetCurrentNemesis() const
{
	FName Nemesis = NAME_None;
	EMGRivalryIntensity HighestIntensity = EMGRivalryIntensity::Acquaintance;

	for (const auto& Pair : Rivalries)
	{
		if (Pair.Value.bIsActive && Pair.Value.Intensity > HighestIntensity)
		{
			HighestIntensity = Pair.Value.Intensity;
			Nemesis = Pair.Key;
		}
	}

	return Nemesis;
}

TArray<FName> UMGRivalSubsystem::GetDefeatedRivals() const
{
	TArray<FName> Result;
	for (const auto& Pair : Rivalries)
	{
		if (Pair.Value.bIsDefeated)
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

bool UMGRivalSubsystem::HasFacedRival(FName RivalID) const
{
	if (const FMGRivalry* Found = Rivalries.Find(RivalID))
	{
		return Found->TotalRaces > 0;
	}
	return false;
}

// ==================== Race Integration ====================

void UMGRivalSubsystem::OnRaceCompleted(FName RaceID, FName RivalID, int32 PlayerPosition, int32 RivalPosition, float TimeDifference, bool bWasPinkSlip)
{
	FMGRival Rival;
	if (!GetRival(RivalID, Rival))
	{
		return;
	}

	// Get or create rivalry
	FMGRivalry* Rivalry = Rivalries.Find(RivalID);
	if (!Rivalry)
	{
		FMGRivalry NewRivalry;
		NewRivalry.RivalID = RivalID;
		NewRivalry.FirstEncounter = FDateTime::Now();
		Rivalries.Add(RivalID, NewRivalry);
		Rivalry = Rivalries.Find(RivalID);

		// First encounter event
		RecordRivalryEvent(RivalID, EMGRivalryEventType::FirstEncounter,
			FText::FromString(TEXT("First race against this rival")), PlayerPosition, RivalPosition);
		OnRivalEncountered.Broadcast(RivalID, true);
	}
	else
	{
		OnRivalEncountered.Broadcast(RivalID, false);
	}

	Rivalry->TotalRaces++;

	const bool bPlayerWon = PlayerPosition < RivalPosition;
	const bool bCloseFinish = FMath::Abs(TimeDifference) < 0.5f;

	if (bPlayerWon)
	{
		Rivalry->PlayerWins++;
		Rivalry->CurrentPlayerStreak++;
		Rivalry->CurrentRivalStreak = 0;

		if (Rivalry->CurrentPlayerStreak > Rivalry->BestPlayerStreak)
		{
			Rivalry->BestPlayerStreak = Rivalry->CurrentPlayerStreak;
		}

		RecordRivalryEvent(RivalID, EMGRivalryEventType::PlayerWon,
			FText::FromString(TEXT("Victory against rival")), PlayerPosition, RivalPosition);

		// Check for domination (5+ wins in a row)
		if (Rivalry->CurrentPlayerStreak >= 5 && Rivalry->CurrentPlayerStreak % 5 == 0)
		{
			RecordRivalryEvent(RivalID, EMGRivalryEventType::DominationAchieved,
				FText::Format(FText::FromString(TEXT("{0} consecutive wins")), FText::AsNumber(Rivalry->CurrentPlayerStreak)));
			OnRivalStreakEvent.Broadcast(RivalID, true, Rivalry->CurrentPlayerStreak);
		}

		// Revenge check
		if (Rivalry->CurrentPlayerStreak == 1 && Rivalry->BestRivalStreak >= 3)
		{
			RecordRivalryEvent(RivalID, EMGRivalryEventType::RevengeAchieved,
				FText::FromString(TEXT("Revenge victory after losing streak")));
		}

		// Respect for clean win
		if (!bCloseFinish)
		{
			ModifyRespect(RivalID, 3);
		}

		// Pink slip
		if (bWasPinkSlip)
		{
			Rivalry->PinkSlipsWonFromThem++;
			RecordRivalryEvent(RivalID, EMGRivalryEventType::PinkSlipWon,
				FText::FromString(TEXT("Won their car in a pink slip race!")));
			ModifyRespect(RivalID, -10); // They'll hate you for this
		}
	}
	else
	{
		Rivalry->RivalWins++;
		Rivalry->CurrentRivalStreak++;
		Rivalry->CurrentPlayerStreak = 0;

		if (Rivalry->CurrentRivalStreak > Rivalry->BestRivalStreak)
		{
			Rivalry->BestRivalStreak = Rivalry->CurrentRivalStreak;
		}

		RecordRivalryEvent(RivalID, EMGRivalryEventType::RivalWon,
			FText::FromString(TEXT("Defeated by rival")), PlayerPosition, RivalPosition);

		// Streak notification
		if (Rivalry->CurrentRivalStreak >= 3 && Rivalry->CurrentRivalStreak % 3 == 0)
		{
			OnRivalStreakEvent.Broadcast(RivalID, false, Rivalry->CurrentRivalStreak);
		}

		// Pink slip loss
		if (bWasPinkSlip)
		{
			Rivalry->PinkSlipsLostToThem++;
			RecordRivalryEvent(RivalID, EMGRivalryEventType::PinkSlipLost,
				FText::FromString(TEXT("Lost car in a pink slip race")));
			ModifyRespect(RivalID, 5); // They'll respect you for having guts
		}
	}

	// Close finish
	if (bCloseFinish)
	{
		Rivalry->PhotoFinishes++;
		RecordRivalryEvent(RivalID, EMGRivalryEventType::CloseFinish,
			FText::Format(FText::FromString(TEXT("Photo finish! Gap: {0}s")), FText::AsNumber(FMath::Abs(TimeDifference))));
		ModifyRespect(RivalID, 5); // Close races build mutual respect
	}

	// Update intensity
	UpdateRivalryIntensity(RivalID);

	// Check for defeat (story rivals)
	CheckForDefeat(RivalID);

	// Broadcast update
	OnRivalryUpdated.Broadcast(RivalID, Rivalry->Intensity, Rivalry->RespectLevel);
}

void UMGRivalSubsystem::RecordRaceContact(FName RivalID, bool bPlayerCausedIt)
{
	RecordRivalryEvent(RivalID, EMGRivalryEventType::WreckCaused,
		bPlayerCausedIt ? FText::FromString(TEXT("Caused collision with rival")) : FText::FromString(TEXT("Rival caused collision")));

	// Contact affects respect
	if (bPlayerCausedIt)
	{
		ModifyRespect(RivalID, -5); // They won't like that
	}
	else
	{
		ModifyRespect(RivalID, -3); // You won't like that
	}
}

// ==================== Trash Talk ====================

FText UMGRivalSubsystem::GetTrashTalkLine(FName RivalID, EMGRivalryEventType Context) const
{
	FMGRival Rival;
	if (!GetRival(RivalID, Rival))
	{
		return FText::GetEmpty();
	}

	TArray<FText>* Lines = nullptr;

	switch (Context)
	{
	case EMGRivalryEventType::FirstEncounter:
	case EMGRivalryEventType::TrashTalkReceived:
		Lines = const_cast<TArray<FText>*>(&Rival.PreRaceLines);
		break;

	case EMGRivalryEventType::PlayerWon:
		Lines = const_cast<TArray<FText>*>(&Rival.LoseLines);
		break;

	case EMGRivalryEventType::RivalWon:
		Lines = const_cast<TArray<FText>*>(&Rival.WinLines);
		break;

	case EMGRivalryEventType::CloseFinish:
		Lines = const_cast<TArray<FText>*>(&Rival.CloseRaceLines);
		break;

	default:
		Lines = const_cast<TArray<FText>*>(&Rival.PreRaceLines);
		break;
	}

	if (Lines && Lines->Num() > 0)
	{
		return (*Lines)[FMath::RandRange(0, Lines->Num() - 1)];
	}

	return FText::GetEmpty();
}

void UMGRivalSubsystem::TriggerTrashTalk(FName RivalID, EMGRivalryEventType Context)
{
	FText Line = GetTrashTalkLine(RivalID, Context);
	if (!Line.IsEmpty())
	{
		RecordRivalryEvent(RivalID, EMGRivalryEventType::TrashTalkReceived, Line);
		OnRivalTrashTalk.Broadcast(RivalID, Line);
	}
}

// ==================== Respect/Animosity ====================

int32 UMGRivalSubsystem::GetRespectLevel(FName RivalID) const
{
	if (const FMGRivalry* Found = Rivalries.Find(RivalID))
	{
		return Found->RespectLevel;
	}
	return 0;
}

void UMGRivalSubsystem::ModifyRespect(FName RivalID, int32 Amount)
{
	if (FMGRivalry* Found = Rivalries.Find(RivalID))
	{
		Found->RespectLevel = FMath::Clamp(Found->RespectLevel + Amount, -100, 100);
	}
}

bool UMGRivalSubsystem::IsRivalFriendly(FName RivalID) const
{
	return GetRespectLevel(RivalID) > 25;
}

// ==================== Statistics ====================

float UMGRivalSubsystem::GetWinRatioAgainst(FName RivalID) const
{
	if (const FMGRivalry* Found = Rivalries.Find(RivalID))
	{
		if (Found->TotalRaces == 0) return 0.0f;
		return static_cast<float>(Found->PlayerWins) / static_cast<float>(Found->TotalRaces);
	}
	return 0.0f;
}

int32 UMGRivalSubsystem::GetTotalRivalRaces() const
{
	int32 Total = 0;
	for (const auto& Pair : Rivalries)
	{
		Total += Pair.Value.TotalRaces;
	}
	return Total;
}

FName UMGRivalSubsystem::GetMostFrequentRival() const
{
	FName MostFrequent = NAME_None;
	int32 HighestRaces = 0;

	for (const auto& Pair : Rivalries)
	{
		if (Pair.Value.TotalRaces > HighestRaces)
		{
			HighestRaces = Pair.Value.TotalRaces;
			MostFrequent = Pair.Key;
		}
	}

	return MostFrequent;
}

void UMGRivalSubsystem::GetDominationStreaks(TMap<FName, int32>& OutPlayerStreaks, TMap<FName, int32>& OutRivalStreaks) const
{
	for (const auto& Pair : Rivalries)
	{
		if (Pair.Value.CurrentPlayerStreak >= 3)
		{
			OutPlayerStreaks.Add(Pair.Key, Pair.Value.CurrentPlayerStreak);
		}
		if (Pair.Value.CurrentRivalStreak >= 3)
		{
			OutRivalStreaks.Add(Pair.Key, Pair.Value.CurrentRivalStreak);
		}
	}
}

// ==================== Story Progression ====================

void UMGRivalSubsystem::DefeatStoryRival(FName RivalID)
{
	if (FMGRivalry* Found = Rivalries.Find(RivalID))
	{
		if (!Found->bIsDefeated)
		{
			Found->bIsDefeated = true;

			RecordRivalryEvent(RivalID, EMGRivalryEventType::RivalryEnded,
				FText::FromString(TEXT("Story rival defeated!")));

			OnRivalDefeated.Broadcast(RivalID, Found->PinkSlipsWonFromThem > 0, Found->PlayerWins);

			// Advance story rival index
			for (int32 i = 0; i < StoryRivalOrder.Num(); ++i)
			{
				if (StoryRivalOrder[i] == RivalID && i >= CurrentStoryRivalIndex)
				{
					CurrentStoryRivalIndex = i + 1;
					break;
				}
			}
		}
	}
}

bool UMGRivalSubsystem::CanChallengeStoryRival(FName RivalID) const
{
	// Find the rival's position in story order
	for (int32 i = 0; i < StoryRivalOrder.Num(); ++i)
	{
		if (StoryRivalOrder[i] == RivalID)
		{
			// Can challenge if it's the current or earlier rival
			return i <= CurrentStoryRivalIndex;
		}
	}
	return true; // Non-story rivals can always be challenged
}

FName UMGRivalSubsystem::GetNextStoryRival() const
{
	if (CurrentStoryRivalIndex < StoryRivalOrder.Num())
	{
		return StoryRivalOrder[CurrentStoryRivalIndex];
	}
	return NAME_None; // All story rivals defeated
}

// ==================== Utility ====================

FText UMGRivalSubsystem::GetIntensityDisplayName(EMGRivalryIntensity Intensity)
{
	switch (Intensity)
	{
	case EMGRivalryIntensity::Acquaintance: return FText::FromString(TEXT("Acquaintance"));
	case EMGRivalryIntensity::Competitive:  return FText::FromString(TEXT("Competitive"));
	case EMGRivalryIntensity::Heated:       return FText::FromString(TEXT("Heated"));
	case EMGRivalryIntensity::Bitter:       return FText::FromString(TEXT("Bitter Rivalry"));
	case EMGRivalryIntensity::LifeLong:     return FText::FromString(TEXT("Life-Long Rivalry"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UMGRivalSubsystem::GetThreatDisplayName(EMGRivalThreatLevel ThreatLevel)
{
	switch (ThreatLevel)
	{
	case EMGRivalThreatLevel::Nuisance:   return FText::FromString(TEXT("Nuisance"));
	case EMGRivalThreatLevel::Contender:  return FText::FromString(TEXT("Contender"));
	case EMGRivalThreatLevel::Dangerous:  return FText::FromString(TEXT("Dangerous"));
	case EMGRivalThreatLevel::Nemesis:    return FText::FromString(TEXT("Nemesis"));
	case EMGRivalThreatLevel::Legend:     return FText::FromString(TEXT("Legend"));
	default: return FText::FromString(TEXT("Unknown"));
	}
}

// ==================== Protected Methods ====================

void UMGRivalSubsystem::UpdateRivalryIntensity(FName RivalID)
{
	FMGRivalry* Rivalry = Rivalries.Find(RivalID);
	if (!Rivalry)
	{
		return;
	}

	EMGRivalryIntensity OldIntensity = Rivalry->Intensity;

	// Calculate new intensity based on history
	const int32 TotalRaces = Rivalry->TotalRaces;
	const int32 PhotoFinishes = Rivalry->PhotoFinishes;
	const int32 MaxStreak = FMath::Max(Rivalry->BestPlayerStreak, Rivalry->BestRivalStreak);
	const int32 AbsRespect = FMath::Abs(Rivalry->RespectLevel);

	if (TotalRaces >= 20 || MaxStreak >= 10 || PhotoFinishes >= 5)
	{
		Rivalry->Intensity = EMGRivalryIntensity::LifeLong;
	}
	else if (TotalRaces >= 10 || MaxStreak >= 5 || AbsRespect >= 50)
	{
		Rivalry->Intensity = EMGRivalryIntensity::Bitter;
	}
	else if (TotalRaces >= 5 || MaxStreak >= 3 || PhotoFinishes >= 2)
	{
		Rivalry->Intensity = EMGRivalryIntensity::Heated;
	}
	else if (TotalRaces >= 2)
	{
		Rivalry->Intensity = EMGRivalryIntensity::Competitive;
	}
	else
	{
		Rivalry->Intensity = EMGRivalryIntensity::Acquaintance;
	}

	if (Rivalry->Intensity != OldIntensity)
	{
		RecordRivalryEvent(RivalID, EMGRivalryEventType::RivalryBegan,
			FText::Format(FText::FromString(TEXT("Rivalry intensified to: {0}")), GetIntensityDisplayName(Rivalry->Intensity)));
	}
}

void UMGRivalSubsystem::CheckForDefeat(FName RivalID)
{
	FMGRival Rival;
	if (!GetRival(RivalID, Rival) || !Rival.bIsStoryRival)
	{
		return;
	}

	FMGRivalry* Rivalry = Rivalries.Find(RivalID);
	if (!Rivalry || Rivalry->bIsDefeated)
	{
		return;
	}

	// Story rival is defeated when player has 3+ more wins
	if (Rivalry->PlayerWins >= Rivalry->RivalWins + 3)
	{
		DefeatStoryRival(RivalID);
	}
}

void UMGRivalSubsystem::RecordRivalryEvent(FName RivalID, EMGRivalryEventType EventType, const FText& Description, int32 PlayerPos, int32 RivalPos)
{
	FMGRivalry* Rivalry = Rivalries.Find(RivalID);
	if (!Rivalry)
	{
		return;
	}

	FMGRivalryEvent Event;
	Event.EventType = EventType;
	Event.Timestamp = FDateTime::Now();
	Event.Description = Description;
	Event.PlayerPosition = PlayerPos;
	Event.RivalPosition = RivalPos;

	Rivalry->History.Insert(Event, 0);

	// Keep history reasonable
	if (Rivalry->History.Num() > 50)
	{
		Rivalry->History.SetNum(50);
	}
}

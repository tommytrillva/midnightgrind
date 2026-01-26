// Copyright Midnight Grind. All Rights Reserved.

#include "Social/MGRivalSubsystem.h"

void UMGRivalSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGRivalSubsystem: Initializing rival and nemesis tracking system"));
}

void UMGRivalSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// RIVALRY TRACKING
// ==========================================

void UMGRivalSubsystem::RecordRivalRace(FGuid Player1ID, FGuid Player2ID, FGuid WinnerID, const FMGRivalryRaceOutcome& Outcome)
{
	// Get player names (would come from player data in production)
	FString Player1Name = TEXT("Player1");
	FString Player2Name = TEXT("Player2");

	FMGRivalryData& Rivalry = GetOrCreateRivalry(Player1ID, Player2ID, Player1Name, Player2Name);

	// Update stats based on who won
	bool bPlayer1Won = (WinnerID == Player1ID);

	FMGRivalryRaceOutcome AdjustedOutcome = Outcome;
	AdjustedOutcome.bWon = bPlayer1Won;
	AdjustedOutcome.MarginSeconds = bPlayer1Won ? Outcome.MarginSeconds : -Outcome.MarginSeconds;

	// Update head-to-head stats
	Rivalry.Player1Stats.TotalRaces++;
	if (bPlayer1Won)
	{
		Rivalry.Player1Stats.Wins++;
		Rivalry.Player1Stats.CurrentStreak = FMath::Max(1, Rivalry.Player1Stats.CurrentStreak + 1);
		Rivalry.Player1Stats.LongestWinStreak = FMath::Max(Rivalry.Player1Stats.LongestWinStreak, Rivalry.Player1Stats.CurrentStreak);

		if (Outcome.bCameFromBehind)
		{
			Rivalry.Player1Stats.ComebackWins++;
		}

		Rivalry.Player1Stats.BestWinMargin = FMath::Max(Rivalry.Player1Stats.BestWinMargin, Outcome.MarginSeconds);

		// Track wins by race type
		int32& WinCount = Rivalry.Player1Stats.WinsByRaceType.FindOrAdd(Outcome.RaceType);
		WinCount++;
	}
	else
	{
		Rivalry.Player1Stats.Losses++;
		Rivalry.Player1Stats.CurrentStreak = FMath::Min(-1, Rivalry.Player1Stats.CurrentStreak - 1);
		Rivalry.Player1Stats.LongestLossStreak = FMath::Max(Rivalry.Player1Stats.LongestLossStreak, FMath::Abs(Rivalry.Player1Stats.CurrentStreak));

		if (Outcome.bCameFromBehind)
		{
			Rivalry.Player1Stats.ComebackLosses++;
		}

		// Track losses by race type
		int32& LossCount = Rivalry.Player1Stats.LossesByRaceType.FindOrAdd(Outcome.RaceType);
		LossCount++;
	}

	// Update photo finishes
	if (FMath::Abs(Outcome.MarginSeconds) <= 0.1f)
	{
		Rivalry.Player1Stats.PhotoFinishes++;
	}

	// Update closest race
	if (Rivalry.Player1Stats.TotalRaces == 1 || FMath::Abs(Outcome.MarginSeconds) < Rivalry.Player1Stats.ClosestRace)
	{
		Rivalry.Player1Stats.ClosestRace = FMath::Abs(Outcome.MarginSeconds);
	}

	// Update average margin
	float TotalMargin = Rivalry.Player1Stats.AverageMarginSeconds * (Rivalry.Player1Stats.TotalRaces - 1);
	TotalMargin += AdjustedOutcome.MarginSeconds;
	Rivalry.Player1Stats.AverageMarginSeconds = TotalMargin / Rivalry.Player1Stats.TotalRaces;

	// Add to recent races
	Rivalry.RecentRaces.Insert(AdjustedOutcome, 0);
	if (Rivalry.RecentRaces.Num() > 20)
	{
		Rivalry.RecentRaces.SetNum(20);
	}

	// Update timestamps
	Rivalry.LastRace = Outcome.Timestamp;
	if (!Rivalry.FirstRace.GetTicks())
	{
		Rivalry.FirstRace = Outcome.Timestamp;
	}

	// Update heat level
	UpdateHeatLevel(Rivalry, Outcome);

	// Update rivalry level
	EMGRivalryLevel OldLevel = Rivalry.Level;
	UpdateRivalryLevel(Rivalry);

	// Check milestones
	CheckRivalryMilestones(Rivalry, Player1ID);

	// Fire events
	if (OldLevel != Rivalry.Level)
	{
		OnRivalryLevelChanged.Broadcast(Rivalry.RivalryID, Rivalry.Level);

		// Add notification for level up
		if (Rivalry.Level > OldLevel)
		{
			FString LevelName;
			switch (Rivalry.Level)
			{
			case EMGRivalryLevel::Competitor: LevelName = TEXT("Competitor"); break;
			case EMGRivalryLevel::Rival: LevelName = TEXT("Rival"); break;
			case EMGRivalryLevel::Nemesis: LevelName = TEXT("Nemesis"); break;
			case EMGRivalryLevel::Legend: LevelName = TEXT("Legendary Rival"); break;
			default: LevelName = TEXT("Rival"); break;
			}

			AddRivalNotification(Player1ID, Player2ID, Player2Name,
				FString::Printf(TEXT("Your rivalry with %s has escalated to %s!"), *Player2Name, *LevelName));
			AddRivalNotification(Player2ID, Player1ID, Player1Name,
				FString::Printf(TEXT("Your rivalry with %s has escalated to %s!"), *Player1Name, *LevelName));
		}
	}

	OnRivalRaceComplete.Broadcast(Rivalry.RivalryID, WinnerID, AdjustedOutcome);

	UE_LOG(LogTemp, Log, TEXT("MGRivalSubsystem: Recorded race between %s and %s - Winner: %s, Rivalry Level: %d"),
		*Player1ID.ToString(), *Player2ID.ToString(), *WinnerID.ToString(), static_cast<int32>(Rivalry.Level));
}

bool UMGRivalSubsystem::GetRivalry(FGuid Player1ID, FGuid Player2ID, FMGRivalryData& OutRivalry) const
{
	FString Key = GetRivalryKey(Player1ID, Player2ID);
	const FMGRivalryData* Rivalry = Rivalries.Find(Key);

	if (Rivalry)
	{
		OutRivalry = *Rivalry;

		// Flip stats if player order was reversed
		if (Rivalry->Player1ID != Player1ID)
		{
			// Swap perspective
			OutRivalry.Player1ID = Player2ID;
			OutRivalry.Player2ID = Player1ID;
			OutRivalry.Player1Name = Rivalry->Player2Name;
			OutRivalry.Player2Name = Rivalry->Player1Name;

			// Invert stats
			OutRivalry.Player1Stats.Wins = Rivalry->Player1Stats.Losses;
			OutRivalry.Player1Stats.Losses = Rivalry->Player1Stats.Wins;
			OutRivalry.Player1Stats.CurrentStreak = -Rivalry->Player1Stats.CurrentStreak;
			OutRivalry.Player1Stats.AverageMarginSeconds = -Rivalry->Player1Stats.AverageMarginSeconds;
		}

		return true;
	}

	return false;
}

TArray<FMGRivalryData> UMGRivalSubsystem::GetPlayerRivalries(FGuid PlayerID) const
{
	TArray<FMGRivalryData> Results;

	for (const auto& RivalryPair : Rivalries)
	{
		if (RivalryPair.Value.Player1ID == PlayerID || RivalryPair.Value.Player2ID == PlayerID)
		{
			FMGRivalryData AdjustedRivalry = RivalryPair.Value;

			// Ensure player is always Player1 in returned data
			if (RivalryPair.Value.Player2ID == PlayerID)
			{
				AdjustedRivalry.Player1ID = RivalryPair.Value.Player2ID;
				AdjustedRivalry.Player2ID = RivalryPair.Value.Player1ID;
				AdjustedRivalry.Player1Name = RivalryPair.Value.Player2Name;
				AdjustedRivalry.Player2Name = RivalryPair.Value.Player1Name;
				AdjustedRivalry.Player1Stats.Wins = RivalryPair.Value.Player1Stats.Losses;
				AdjustedRivalry.Player1Stats.Losses = RivalryPair.Value.Player1Stats.Wins;
			}

			Results.Add(AdjustedRivalry);
		}
	}

	// Sort by last race
	Results.Sort([](const FMGRivalryData& A, const FMGRivalryData& B)
	{
		return A.LastRace > B.LastRace;
	});

	return Results;
}

TArray<FMGRivalryData> UMGRivalSubsystem::GetTopRivalries(FGuid PlayerID, int32 MaxCount) const
{
	TArray<FMGRivalryData> AllRivalries = GetPlayerRivalries(PlayerID);

	// Sort by heat level and total races
	AllRivalries.Sort([](const FMGRivalryData& A, const FMGRivalryData& B)
	{
		float ScoreA = A.HeatLevel + A.Player1Stats.TotalRaces * 2.0f;
		float ScoreB = B.HeatLevel + B.Player1Stats.TotalRaces * 2.0f;
		return ScoreA > ScoreB;
	});

	if (AllRivalries.Num() > MaxCount)
	{
		AllRivalries.SetNum(MaxCount);
	}

	return AllRivalries;
}

FMGHeadToHeadStats UMGRivalSubsystem::GetHeadToHeadStats(FGuid PlayerID, FGuid OpponentID) const
{
	FMGRivalryData Rivalry;
	if (GetRivalry(PlayerID, OpponentID, Rivalry))
	{
		return Rivalry.Player1Stats;
	}

	return FMGHeadToHeadStats();
}

// ==========================================
// NEMESIS SYSTEM
// ==========================================

bool UMGRivalSubsystem::DesignateNemesis(FGuid PlayerID, FGuid NemesisID)
{
	if (!PlayerID.IsValid() || !NemesisID.IsValid() || PlayerID == NemesisID)
	{
		return false;
	}

	// Check if there's a rivalry
	FString Key = GetRivalryKey(PlayerID, NemesisID);
	FMGRivalryData* Rivalry = Rivalries.Find(Key);

	if (!Rivalry)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGRivalSubsystem: Cannot designate nemesis without existing rivalry"));
		return false;
	}

	// Set designation
	PlayerNemeses.Add(PlayerID, NemesisID);
	Rivalry->bIsDesignatedNemesis = true;

	// Check if mutual
	FGuid* OtherNemesis = PlayerNemeses.Find(NemesisID);
	if (OtherNemesis && *OtherNemesis == PlayerID)
	{
		Rivalry->bIsMutual = true;
		Rivalry->HeatLevel = FMath::Min(100.0f, Rivalry->HeatLevel + 25.0f);

		// Notify both players
		AddRivalNotification(PlayerID, NemesisID, Rivalry->Player2Name,
			FString::Printf(TEXT("%s has also designated you as their nemesis! This rivalry is now MUTUAL."), *Rivalry->Player2Name));
		AddRivalNotification(NemesisID, PlayerID, Rivalry->Player1Name,
			FString::Printf(TEXT("%s has designated you as their nemesis! This rivalry is now MUTUAL."), *Rivalry->Player1Name));
	}
	else
	{
		// Notify target
		FString PlayerName = (Rivalry->Player1ID == PlayerID) ? Rivalry->Player1Name : Rivalry->Player2Name;
		AddRivalNotification(NemesisID, PlayerID, PlayerName,
			FString::Printf(TEXT("%s has designated you as their NEMESIS!"), *PlayerName));
	}

	OnNemesisDesignated.Broadcast(PlayerID, NemesisID);

	UE_LOG(LogTemp, Log, TEXT("MGRivalSubsystem: %s designated %s as nemesis. Mutual: %s"),
		*PlayerID.ToString(), *NemesisID.ToString(), Rivalry->bIsMutual ? TEXT("Yes") : TEXT("No"));

	return true;
}

bool UMGRivalSubsystem::RemoveNemesis(FGuid PlayerID)
{
	FGuid* CurrentNemesis = PlayerNemeses.Find(PlayerID);
	if (!CurrentNemesis)
	{
		return false;
	}

	FString Key = GetRivalryKey(PlayerID, *CurrentNemesis);
	FMGRivalryData* Rivalry = Rivalries.Find(Key);

	if (Rivalry)
	{
		// Check if other player still has us as nemesis
		FGuid* OtherNemesis = PlayerNemeses.Find(*CurrentNemesis);
		if (!OtherNemesis || *OtherNemesis != PlayerID)
		{
			Rivalry->bIsDesignatedNemesis = false;
		}
		Rivalry->bIsMutual = false;
	}

	PlayerNemeses.Remove(PlayerID);
	return true;
}

FGuid UMGRivalSubsystem::GetDesignatedNemesis(FGuid PlayerID) const
{
	const FGuid* Nemesis = PlayerNemeses.Find(PlayerID);
	return Nemesis ? *Nemesis : FGuid();
}

FGuid UMGRivalSubsystem::GetSuggestedNemesis(FGuid PlayerID) const
{
	TArray<FMGRivalryData> TopRivals = GetTopRivalries(PlayerID, 10);

	FGuid BestNemesis;
	float BestScore = 0.0f;

	for (const FMGRivalryData& Rivalry : TopRivals)
	{
		// Score based on: heat, total races, close win/loss ratio
		float RaceScore = Rivalry.Player1Stats.TotalRaces * 2.0f;
		float HeatScore = Rivalry.HeatLevel;

		// Bonus for close rivalry (win rate near 50%)
		float WinRate = Rivalry.Player1Stats.TotalRaces > 0 ?
			static_cast<float>(Rivalry.Player1Stats.Wins) / Rivalry.Player1Stats.TotalRaces : 0.5f;
		float ClosenessBonus = (1.0f - FMath::Abs(WinRate - 0.5f) * 2.0f) * 30.0f;

		float TotalScore = RaceScore + HeatScore + ClosenessBonus;

		if (TotalScore > BestScore)
		{
			BestScore = TotalScore;
			BestNemesis = Rivalry.Player2ID;
		}
	}

	return BestNemesis;
}

bool UMGRivalSubsystem::AreMutualNemeses(FGuid Player1ID, FGuid Player2ID) const
{
	const FGuid* Nemesis1 = PlayerNemeses.Find(Player1ID);
	const FGuid* Nemesis2 = PlayerNemeses.Find(Player2ID);

	return Nemesis1 && Nemesis2 && *Nemesis1 == Player2ID && *Nemesis2 == Player1ID;
}

// ==========================================
// CALLOUTS
// ==========================================

FGuid UMGRivalSubsystem::IssueCallout(FGuid ChallengerID, FGuid TargetID, EMGCalloutType CalloutType, const FString& Message, int32 CashWager, int32 REPWager)
{
	if (!ChallengerID.IsValid() || !TargetID.IsValid() || ChallengerID == TargetID)
	{
		return FGuid();
	}

	FMGRivalryCallout Callout;
	Callout.CalloutID = FGuid::NewGuid();
	Callout.ChallengerID = ChallengerID;
	Callout.ChallengerName = TEXT("Challenger"); // Would come from player data
	Callout.TargetID = TargetID;
	Callout.TargetName = TEXT("Target"); // Would come from player data
	Callout.CalloutType = CalloutType;
	Callout.Message = Message;
	Callout.Response = EMGCalloutResponse::Pending;
	Callout.CreatedAt = FDateTime::UtcNow();
	Callout.ExpiresAt = FDateTime::UtcNow() + FTimespan::FromHours(24);
	Callout.CashWager = CashWager;
	Callout.REPWager = REPWager;
	Callout.bIsPublic = true;

	ActiveCallouts.Add(Callout.CalloutID, Callout);

	// Increase rivalry heat
	FString Key = GetRivalryKey(ChallengerID, TargetID);
	FMGRivalryData* Rivalry = Rivalries.Find(Key);
	if (Rivalry)
	{
		Rivalry->HeatLevel = FMath::Min(100.0f, Rivalry->HeatLevel + HeatGainFromCallout);
	}

	// Notify target
	AddRivalNotification(TargetID, ChallengerID, Callout.ChallengerName,
		FString::Printf(TEXT("%s has called you out! \"%s\""), *Callout.ChallengerName, *Message));

	OnCalloutReceived.Broadcast(TargetID, Callout);

	UE_LOG(LogTemp, Log, TEXT("MGRivalSubsystem: %s issued callout to %s - Wager: $%d, %d REP"),
		*ChallengerID.ToString(), *TargetID.ToString(), CashWager, REPWager);

	return Callout.CalloutID;
}

FGuid UMGRivalSubsystem::IssuePinkSlipCallout(FGuid ChallengerID, FGuid TargetID, FGuid ChallengerVehicleID, const FString& Message)
{
	FGuid CalloutID = IssueCallout(ChallengerID, TargetID, EMGCalloutType::PinkSlip, Message, 0, 0);

	if (CalloutID.IsValid())
	{
		FMGRivalryCallout* Callout = ActiveCallouts.Find(CalloutID);
		if (Callout)
		{
			Callout->VehicleWager = ChallengerVehicleID;
		}
	}

	return CalloutID;
}

bool UMGRivalSubsystem::RespondToCallout(FGuid CalloutID, FGuid ResponderID, EMGCalloutResponse Response)
{
	FMGRivalryCallout* Callout = ActiveCallouts.Find(CalloutID);
	if (!Callout || Callout->TargetID != ResponderID)
	{
		return false;
	}

	if (Callout->Response != EMGCalloutResponse::Pending)
	{
		return false; // Already responded
	}

	Callout->Response = Response;

	// Notify challenger
	FString ResponseText;
	switch (Response)
	{
	case EMGCalloutResponse::Accepted:
		ResponseText = TEXT("accepted your challenge!");
		break;
	case EMGCalloutResponse::Declined:
		ResponseText = TEXT("declined your challenge.");
		break;
	case EMGCalloutResponse::Ignored:
		ResponseText = TEXT("ignored your challenge.");
		break;
	default:
		ResponseText = TEXT("responded to your challenge.");
		break;
	}

	AddRivalNotification(Callout->ChallengerID, ResponderID, Callout->TargetName,
		FString::Printf(TEXT("%s %s"), *Callout->TargetName, *ResponseText));

	OnCalloutResponded.Broadcast(CalloutID, ResponderID, Response);

	// If declined/ignored, remove from active
	if (Response != EMGCalloutResponse::Accepted)
	{
		// Keep for history but mark as resolved
	}

	UE_LOG(LogTemp, Log, TEXT("MGRivalSubsystem: Callout %s response: %d"),
		*CalloutID.ToString(), static_cast<int32>(Response));

	return true;
}

TArray<FMGRivalryCallout> UMGRivalSubsystem::GetPendingCallouts(FGuid PlayerID) const
{
	TArray<FMGRivalryCallout> Results;

	for (const auto& CalloutPair : ActiveCallouts)
	{
		if (CalloutPair.Value.TargetID == PlayerID &&
			CalloutPair.Value.Response == EMGCalloutResponse::Pending &&
			CalloutPair.Value.ExpiresAt > FDateTime::UtcNow())
		{
			Results.Add(CalloutPair.Value);
		}
	}

	// Sort by creation time
	Results.Sort([](const FMGRivalryCallout& A, const FMGRivalryCallout& B)
	{
		return A.CreatedAt > B.CreatedAt;
	});

	return Results;
}

TArray<FMGRivalryCallout> UMGRivalSubsystem::GetOutgoingCallouts(FGuid PlayerID) const
{
	TArray<FMGRivalryCallout> Results;

	for (const auto& CalloutPair : ActiveCallouts)
	{
		if (CalloutPair.Value.ChallengerID == PlayerID &&
			CalloutPair.Value.ExpiresAt > FDateTime::UtcNow())
		{
			Results.Add(CalloutPair.Value);
		}
	}

	Results.Sort([](const FMGRivalryCallout& A, const FMGRivalryCallout& B)
	{
		return A.CreatedAt > B.CreatedAt;
	});

	return Results;
}

TArray<FMGRivalryCallout> UMGRivalSubsystem::GetRecentPublicCallouts(int32 MaxCount) const
{
	TArray<FMGRivalryCallout> Results;

	for (const auto& CalloutPair : ActiveCallouts)
	{
		if (CalloutPair.Value.bIsPublic &&
			CalloutPair.Value.ExpiresAt > FDateTime::UtcNow())
		{
			Results.Add(CalloutPair.Value);
		}
	}

	// Sort by view count and creation time
	Results.Sort([](const FMGRivalryCallout& A, const FMGRivalryCallout& B)
	{
		if (A.ViewCount != B.ViewCount)
		{
			return A.ViewCount > B.ViewCount;
		}
		return A.CreatedAt > B.CreatedAt;
	});

	if (Results.Num() > MaxCount)
	{
		Results.SetNum(MaxCount);
	}

	return Results;
}

bool UMGRivalSubsystem::SpectateCallout(FGuid CalloutID, FGuid SpectatorID)
{
	FMGRivalryCallout* Callout = ActiveCallouts.Find(CalloutID);
	if (!Callout)
	{
		return false;
	}

	if (!Callout->Spectators.Contains(SpectatorID))
	{
		Callout->Spectators.Add(SpectatorID);
		Callout->ViewCount++;
	}

	return true;
}

// ==========================================
// RIVALRY BONUSES
// ==========================================

float UMGRivalSubsystem::GetRivalREPBonus(FGuid PlayerID, FGuid OpponentID) const
{
	FMGRivalryData Rivalry;
	if (!GetRivalry(PlayerID, OpponentID, Rivalry))
	{
		return 0.0f;
	}

	// Base bonus by rivalry level
	float Bonus = 0.0f;
	switch (Rivalry.Level)
	{
	case EMGRivalryLevel::Noticed: Bonus = 5.0f; break;
	case EMGRivalryLevel::Competitor: Bonus = 10.0f; break;
	case EMGRivalryLevel::Rival: Bonus = 20.0f; break;
	case EMGRivalryLevel::Nemesis: Bonus = 35.0f; break;
	case EMGRivalryLevel::Legend: Bonus = 50.0f; break;
	default: break;
	}

	// Extra bonus for mutual nemesis
	if (Rivalry.bIsMutual)
	{
		Bonus *= 1.5f;
	}

	// Heat bonus
	Bonus += Rivalry.HeatLevel * 0.2f;

	return Bonus;
}

float UMGRivalSubsystem::GetRivalCashBonus(FGuid PlayerID, FGuid OpponentID) const
{
	FMGRivalryData Rivalry;
	if (!GetRivalry(PlayerID, OpponentID, Rivalry))
	{
		return 0.0f;
	}

	// Similar but lower than REP
	float Bonus = 0.0f;
	switch (Rivalry.Level)
	{
	case EMGRivalryLevel::Noticed: Bonus = 2.0f; break;
	case EMGRivalryLevel::Competitor: Bonus = 5.0f; break;
	case EMGRivalryLevel::Rival: Bonus = 10.0f; break;
	case EMGRivalryLevel::Nemesis: Bonus = 20.0f; break;
	case EMGRivalryLevel::Legend: Bonus = 30.0f; break;
	default: break;
	}

	if (Rivalry.bIsMutual)
	{
		Bonus *= 1.25f;
	}

	return Bonus;
}

float UMGRivalSubsystem::GetStreakBonus(FGuid PlayerID, FGuid OpponentID) const
{
	FMGRivalryData Rivalry;
	if (!GetRivalry(PlayerID, OpponentID, Rivalry))
	{
		return 0.0f;
	}

	// On a losing streak? Higher bonus for breaking it
	if (Rivalry.Player1Stats.CurrentStreak < 0)
	{
		return FMath::Abs(Rivalry.Player1Stats.CurrentStreak) * 5.0f;
	}

	// On a win streak? Bonus to extend it
	if (Rivalry.Player1Stats.CurrentStreak > 2)
	{
		return Rivalry.Player1Stats.CurrentStreak * 3.0f;
	}

	return 0.0f;
}

// ==========================================
// LEADERBOARDS
// ==========================================

TArray<FMGRivalryLeaderboardEntry> UMGRivalSubsystem::GetHottestRivalries(int32 MaxEntries)
{
	TArray<FMGRivalryLeaderboardEntry> Results;

	for (const auto& RivalryPair : Rivalries)
	{
		FMGRivalryLeaderboardEntry Entry;
		Entry.Player1Name = RivalryPair.Value.Player1Name;
		Entry.Player2Name = RivalryPair.Value.Player2Name;
		Entry.TotalRaces = RivalryPair.Value.Player1Stats.TotalRaces;
		Entry.Player1Wins = RivalryPair.Value.Player1Stats.Wins;
		Entry.Player2Wins = RivalryPair.Value.Player1Stats.Losses;
		Entry.Level = RivalryPair.Value.Level;
		Entry.HeatLevel = RivalryPair.Value.HeatLevel;

		Results.Add(Entry);
	}

	// Sort by heat level
	Results.Sort([](const FMGRivalryLeaderboardEntry& A, const FMGRivalryLeaderboardEntry& B)
	{
		return A.HeatLevel > B.HeatLevel;
	});

	if (Results.Num() > MaxEntries)
	{
		Results.SetNum(MaxEntries);
	}

	// Assign ranks
	for (int32 i = 0; i < Results.Num(); i++)
	{
		Results[i].Rank = i + 1;
	}

	return Results;
}

TArray<FMGRivalryLeaderboardEntry> UMGRivalSubsystem::GetMostRacesRivalries(int32 MaxEntries)
{
	TArray<FMGRivalryLeaderboardEntry> Results;

	for (const auto& RivalryPair : Rivalries)
	{
		FMGRivalryLeaderboardEntry Entry;
		Entry.Player1Name = RivalryPair.Value.Player1Name;
		Entry.Player2Name = RivalryPair.Value.Player2Name;
		Entry.TotalRaces = RivalryPair.Value.Player1Stats.TotalRaces;
		Entry.Player1Wins = RivalryPair.Value.Player1Stats.Wins;
		Entry.Player2Wins = RivalryPair.Value.Player1Stats.Losses;
		Entry.Level = RivalryPair.Value.Level;
		Entry.HeatLevel = RivalryPair.Value.HeatLevel;

		Results.Add(Entry);
	}

	// Sort by total races
	Results.Sort([](const FMGRivalryLeaderboardEntry& A, const FMGRivalryLeaderboardEntry& B)
	{
		return A.TotalRaces > B.TotalRaces;
	});

	if (Results.Num() > MaxEntries)
	{
		Results.SetNum(MaxEntries);
	}

	for (int32 i = 0; i < Results.Num(); i++)
	{
		Results[i].Rank = i + 1;
	}

	return Results;
}

TArray<FMGRivalryLeaderboardEntry> UMGRivalSubsystem::GetClosestRivalries(int32 MaxEntries)
{
	TArray<FMGRivalryLeaderboardEntry> Results;

	for (const auto& RivalryPair : Rivalries)
	{
		// Minimum 10 races to qualify
		if (RivalryPair.Value.Player1Stats.TotalRaces < 10)
		{
			continue;
		}

		FMGRivalryLeaderboardEntry Entry;
		Entry.Player1Name = RivalryPair.Value.Player1Name;
		Entry.Player2Name = RivalryPair.Value.Player2Name;
		Entry.TotalRaces = RivalryPair.Value.Player1Stats.TotalRaces;
		Entry.Player1Wins = RivalryPair.Value.Player1Stats.Wins;
		Entry.Player2Wins = RivalryPair.Value.Player1Stats.Losses;
		Entry.Level = RivalryPair.Value.Level;
		Entry.HeatLevel = RivalryPair.Value.HeatLevel;

		Results.Add(Entry);
	}

	// Sort by how close the win ratio is to 50%
	Results.Sort([](const FMGRivalryLeaderboardEntry& A, const FMGRivalryLeaderboardEntry& B)
	{
		float RatioA = A.TotalRaces > 0 ? static_cast<float>(A.Player1Wins) / A.TotalRaces : 0.5f;
		float RatioB = B.TotalRaces > 0 ? static_cast<float>(B.Player1Wins) / B.TotalRaces : 0.5f;
		return FMath::Abs(RatioA - 0.5f) < FMath::Abs(RatioB - 0.5f);
	});

	if (Results.Num() > MaxEntries)
	{
		Results.SetNum(MaxEntries);
	}

	for (int32 i = 0; i < Results.Num(); i++)
	{
		Results[i].Rank = i + 1;
	}

	return Results;
}

// ==========================================
// NOTIFICATIONS
// ==========================================

TArray<FMGRivalNotification> UMGRivalSubsystem::GetRivalNotifications(FGuid PlayerID, bool bUnreadOnly) const
{
	TArray<FMGRivalNotification> Results;

	const TArray<FMGRivalNotification>* Notifications = PlayerNotifications.Find(PlayerID);
	if (!Notifications)
	{
		return Results;
	}

	for (const FMGRivalNotification& Notif : *Notifications)
	{
		if (!bUnreadOnly || !Notif.bRead)
		{
			Results.Add(Notif);
		}
	}

	return Results;
}

void UMGRivalSubsystem::MarkNotificationRead(FGuid NotificationID)
{
	for (auto& NotifPair : PlayerNotifications)
	{
		for (FMGRivalNotification& Notif : NotifPair.Value)
		{
			if (Notif.NotificationID == NotificationID)
			{
				Notif.bRead = true;
				return;
			}
		}
	}
}

void UMGRivalSubsystem::MarkAllNotificationsRead(FGuid PlayerID)
{
	TArray<FMGRivalNotification>* Notifications = PlayerNotifications.Find(PlayerID);
	if (Notifications)
	{
		for (FMGRivalNotification& Notif : *Notifications)
		{
			Notif.bRead = true;
		}
	}
}

// ==========================================
// ACHIEVEMENTS
// ==========================================

TArray<FName> UMGRivalSubsystem::CheckRivalryAchievements(FGuid PlayerID) const
{
	TArray<FName> Achievements;

	int32 TotalRaces = 0;
	int32 UniqueRivals = 0;
	int32 NemesisWins = 0;
	int32 CalloutWins = 0;

	GetMilestoneProgress(PlayerID, TotalRaces, UniqueRivals, NemesisWins, CalloutWins);

	// First rival
	if (UniqueRivals >= 1)
	{
		Achievements.Add(FName(TEXT("FirstRival")));
	}

	// 10 unique rivals
	if (UniqueRivals >= 10)
	{
		Achievements.Add(FName(TEXT("TenRivals")));
	}

	// 100 rival races
	if (TotalRaces >= 100)
	{
		Achievements.Add(FName(TEXT("HundredRivalRaces")));
	}

	// Nemesis defeated
	FGuid Nemesis = GetDesignatedNemesis(PlayerID);
	if (Nemesis.IsValid())
	{
		FMGRivalryData Rivalry;
		if (GetRivalry(PlayerID, Nemesis, Rivalry) && Rivalry.Player1Stats.Wins > 0)
		{
			Achievements.Add(FName(TEXT("NemesisDefeated")));
		}

		// Nemesis domination (10 wins vs nemesis)
		if (Rivalry.Player1Stats.Wins >= 10)
		{
			Achievements.Add(FName(TEXT("NemesisDomination")));
		}
	}

	// Mutual nemesis
	for (const auto& RivalryPair : Rivalries)
	{
		if ((RivalryPair.Value.Player1ID == PlayerID || RivalryPair.Value.Player2ID == PlayerID) &&
			RivalryPair.Value.bIsMutual)
		{
			Achievements.Add(FName(TEXT("MutualNemesis")));
			break;
		}
	}

	// Legend rivalry
	for (const auto& RivalryPair : Rivalries)
	{
		if ((RivalryPair.Value.Player1ID == PlayerID || RivalryPair.Value.Player2ID == PlayerID) &&
			RivalryPair.Value.Level == EMGRivalryLevel::Legend)
		{
			Achievements.Add(FName(TEXT("LegendaryRivalry")));
			break;
		}
	}

	return Achievements;
}

void UMGRivalSubsystem::GetMilestoneProgress(FGuid PlayerID, int32& OutTotalRaces, int32& OutUniqueRivals, int32& OutNemesisWins, int32& OutCalloutWins) const
{
	OutTotalRaces = 0;
	OutUniqueRivals = 0;
	OutNemesisWins = 0;
	OutCalloutWins = 0;

	TArray<FMGRivalryData> PlayerRivalries = GetPlayerRivalries(PlayerID);

	OutUniqueRivals = PlayerRivalries.Num();

	for (const FMGRivalryData& Rivalry : PlayerRivalries)
	{
		OutTotalRaces += Rivalry.Player1Stats.TotalRaces;
	}

	// Nemesis wins
	FGuid Nemesis = GetDesignatedNemesis(PlayerID);
	if (Nemesis.IsValid())
	{
		FMGRivalryData NemesisRivalry;
		if (GetRivalry(PlayerID, Nemesis, NemesisRivalry))
		{
			OutNemesisWins = NemesisRivalry.Player1Stats.Wins;
		}
	}

	// Note: Callout wins would need to be tracked separately
}

// ==========================================
// INTERNAL
// ==========================================

FString UMGRivalSubsystem::GetRivalryKey(FGuid Player1ID, FGuid Player2ID) const
{
	// Always use the same order for consistent key
	if (Player1ID.ToString() < Player2ID.ToString())
	{
		return FString::Printf(TEXT("%s_%s"), *Player1ID.ToString(), *Player2ID.ToString());
	}
	return FString::Printf(TEXT("%s_%s"), *Player2ID.ToString(), *Player1ID.ToString());
}

FMGRivalryData& UMGRivalSubsystem::GetOrCreateRivalry(FGuid Player1ID, FGuid Player2ID, const FString& Player1Name, const FString& Player2Name)
{
	FString Key = GetRivalryKey(Player1ID, Player2ID);

	FMGRivalryData* Existing = Rivalries.Find(Key);
	if (Existing)
	{
		return *Existing;
	}

	// Create new rivalry
	FMGRivalryData NewRivalry;
	NewRivalry.RivalryID = FGuid::NewGuid();

	// Ensure consistent ordering
	if (Player1ID.ToString() < Player2ID.ToString())
	{
		NewRivalry.Player1ID = Player1ID;
		NewRivalry.Player2ID = Player2ID;
		NewRivalry.Player1Name = Player1Name;
		NewRivalry.Player2Name = Player2Name;
	}
	else
	{
		NewRivalry.Player1ID = Player2ID;
		NewRivalry.Player2ID = Player1ID;
		NewRivalry.Player1Name = Player2Name;
		NewRivalry.Player2Name = Player1Name;
	}

	NewRivalry.Level = EMGRivalryLevel::None;
	NewRivalry.Disposition = EMGRivalryDisposition::Neutral;

	Rivalries.Add(Key, NewRivalry);

	OnRivalryCreated.Broadcast(Player1ID, NewRivalry);

	return Rivalries[Key];
}

void UMGRivalSubsystem::UpdateRivalryLevel(FMGRivalryData& Rivalry)
{
	int32 TotalRaces = Rivalry.Player1Stats.TotalRaces;
	EMGRivalryLevel OldLevel = Rivalry.Level;

	if (TotalRaces >= LegendThreshold)
	{
		Rivalry.Level = EMGRivalryLevel::Legend;
	}
	else if (TotalRaces >= NemesisThreshold && Rivalry.HeatLevel >= 50.0f)
	{
		Rivalry.Level = EMGRivalryLevel::Nemesis;
	}
	else if (TotalRaces >= RivalThreshold)
	{
		Rivalry.Level = EMGRivalryLevel::Rival;
	}
	else if (TotalRaces >= CompetitorThreshold)
	{
		Rivalry.Level = EMGRivalryLevel::Competitor;
	}
	else if (TotalRaces >= NoticedThreshold)
	{
		Rivalry.Level = EMGRivalryLevel::Noticed;
	}
	else
	{
		Rivalry.Level = EMGRivalryLevel::None;
	}

	// Update disposition based on heat
	if (Rivalry.HeatLevel >= 80.0f)
	{
		Rivalry.Disposition = EMGRivalryDisposition::Hostile;
	}
	else if (Rivalry.HeatLevel >= 50.0f)
	{
		Rivalry.Disposition = EMGRivalryDisposition::Heated;
	}
	else if (Rivalry.HeatLevel >= 20.0f)
	{
		Rivalry.Disposition = EMGRivalryDisposition::Friendly;
	}
	else
	{
		Rivalry.Disposition = EMGRivalryDisposition::Neutral;
	}
}

void UMGRivalSubsystem::UpdateHeatLevel(FMGRivalryData& Rivalry, const FMGRivalryRaceOutcome& Outcome)
{
	// Add heat based on race characteristics
	float HeatGain = HeatGainNormal;

	if (Outcome.bWasClose)
	{
		HeatGain = HeatGainClose;
	}

	if (Outcome.bCameFromBehind)
	{
		HeatGain += HeatGainComeback;
	}

	// Bonus heat for ending a long streak
	if (FMath::Abs(Rivalry.Player1Stats.CurrentStreak) >= 3)
	{
		HeatGain += 5.0f;
	}

	Rivalry.HeatLevel = FMath::Clamp(Rivalry.HeatLevel + HeatGain, 0.0f, 100.0f);
}

void UMGRivalSubsystem::AddRivalNotification(FGuid PlayerID, FGuid RivalID, const FString& RivalName, const FString& Message)
{
	FMGRivalNotification Notif;
	Notif.NotificationID = FGuid::NewGuid();
	Notif.RivalID = RivalID;
	Notif.RivalName = RivalName;
	Notif.Message = Message;
	Notif.Timestamp = FDateTime::UtcNow();
	Notif.bRead = false;

	TArray<FMGRivalNotification>& Notifications = PlayerNotifications.FindOrAdd(PlayerID);
	Notifications.Insert(Notif, 0);

	// Limit notification count
	if (Notifications.Num() > 50)
	{
		Notifications.SetNum(50);
	}
}

void UMGRivalSubsystem::ProcessExpiredCallouts()
{
	FDateTime Now = FDateTime::UtcNow();

	for (auto& CalloutPair : ActiveCallouts)
	{
		if (CalloutPair.Value.Response == EMGCalloutResponse::Pending &&
			CalloutPair.Value.ExpiresAt <= Now)
		{
			CalloutPair.Value.Response = EMGCalloutResponse::Expired;
		}
	}
}

void UMGRivalSubsystem::CheckRivalryMilestones(FMGRivalryData& Rivalry, FGuid PlayerID)
{
	FMGHeadToHeadStats& Stats = Rivalry.Player1Stats;

	// First win milestone
	if (!Rivalry.bFirstWin && Stats.Wins > 0)
	{
		Rivalry.bFirstWin = true;
	}

	// First 3-peat
	if (!Rivalry.bFirst3Peat && Stats.LongestWinStreak >= 3)
	{
		Rivalry.bFirst3Peat = true;
	}

	// 10 races milestone
	if (!Rivalry.bFirst10Races && Stats.TotalRaces >= 10)
	{
		Rivalry.bFirst10Races = true;
	}

	// Photo finish
	if (!Rivalry.bPhotographFinish && Stats.PhotoFinishes > 0)
	{
		Rivalry.bPhotographFinish = true;
	}
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Currency/MGCurrencySubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGCurrencySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadCurrencyData();

	// Update multipliers every minute
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			MultiplierUpdateTimer,
			this,
			&UMGCurrencySubsystem::UpdateMultipliers,
			60.0f,
			true
		);
	}
}

void UMGCurrencySubsystem::Deinitialize()
{
	SaveCurrencyData();
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MultiplierUpdateTimer);
	}
	Super::Deinitialize();
}

int64 UMGCurrencySubsystem::GetCurrencyAmount(EMGCurrencyType Type) const
{
	switch (Type)
	{
	case EMGCurrencyType::GrindCash: return CurrentBalance.GrindCash;
	case EMGCurrencyType::NeonCredits: return CurrentBalance.NeonCredits;
	case EMGCurrencyType::CrewTokens: return CurrentBalance.CrewTokens;
	case EMGCurrencyType::SeasonPoints: return CurrentBalance.SeasonPoints;
	case EMGCurrencyType::LegacyMarks: return CurrentBalance.LegacyMarks;
	default: return 0;
	}
}

bool UMGCurrencySubsystem::CanAfford(EMGCurrencyType Type, int64 Amount) const
{
	return GetCurrencyAmount(Type) >= Amount;
}

bool UMGCurrencySubsystem::EarnCurrency(EMGCurrencyType Type, int64 Amount, EMGEarnSource Source, const FString& Details)
{
	if (Amount <= 0) return false;

	// Apply multipliers
	float Multiplier = GetTotalMultiplier(Type);
	int64 FinalAmount = FMath::RoundToInt64(Amount * Multiplier);

	int64 OldBalance = GetCurrencyAmount(Type);
	int64 NewBalance = OldBalance + FinalAmount;

	switch (Type)
	{
	case EMGCurrencyType::GrindCash:
		CurrentBalance.GrindCash = NewBalance;
		TotalGrindCashEarned += FinalAmount;
		break;
	case EMGCurrencyType::NeonCredits:
		CurrentBalance.NeonCredits = NewBalance;
		TotalNeonCreditsEarned += FinalAmount;
		break;
	case EMGCurrencyType::CrewTokens:
		CurrentBalance.CrewTokens = NewBalance;
		break;
	case EMGCurrencyType::SeasonPoints:
		CurrentBalance.SeasonPoints = NewBalance;
		break;
	case EMGCurrencyType::LegacyMarks:
		CurrentBalance.LegacyMarks = NewBalance;
		break;
	}

	RecordTransaction(Type, FinalAmount, Source, Details);
	OnCurrencyChanged.Broadcast(Type, NewBalance, FinalAmount);
	SaveCurrencyData();

	return true;
}

bool UMGCurrencySubsystem::SpendCurrency(EMGCurrencyType Type, int64 Amount, const FString& PurchaseDetails)
{
	if (Amount <= 0 || !CanAfford(Type, Amount)) return false;

	int64 OldBalance = GetCurrencyAmount(Type);
	int64 NewBalance = OldBalance - Amount;

	switch (Type)
	{
	case EMGCurrencyType::GrindCash:
		CurrentBalance.GrindCash = NewBalance;
		TotalGrindCashSpent += Amount;
		break;
	case EMGCurrencyType::NeonCredits:
		CurrentBalance.NeonCredits = NewBalance;
		TotalNeonCreditsSpent += Amount;
		break;
	case EMGCurrencyType::CrewTokens:
		CurrentBalance.CrewTokens = NewBalance;
		break;
	case EMGCurrencyType::SeasonPoints:
		CurrentBalance.SeasonPoints = NewBalance;
		break;
	case EMGCurrencyType::LegacyMarks:
		CurrentBalance.LegacyMarks = NewBalance;
		break;
	}

	// Record as negative transaction
	FMGCurrencyTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.CurrencyType = Type;
	Transaction.Amount = -Amount;
	Transaction.Source = EMGEarnSource::RaceFinish; // Would have purchase source
	Transaction.SourceDetails = PurchaseDetails;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.BalanceAfter = NewBalance;
	TransactionHistory.Add(Transaction);

	OnCurrencyChanged.Broadcast(Type, NewBalance, -Amount);
	OnTransactionCompleted.Broadcast(Transaction);
	SaveCurrencyData();

	return true;
}

bool UMGCurrencySubsystem::TransferCurrency(EMGCurrencyType Type, int64 Amount, const FString& RecipientID)
{
	// Only allow gifting of GrindCash to prevent premium currency exploits
	if (Type != EMGCurrencyType::GrindCash) return false;
	if (!CanAfford(Type, Amount)) return false;

	// Would validate recipient and execute server-side transfer
	SpendCurrency(Type, Amount, FString::Printf(TEXT("Gift to %s"), *RecipientID));
	return true;
}

FMGRaceEarnings UMGCurrencySubsystem::CalculateRaceEarnings(int32 Position, int32 TotalRacers, FName TrackID, bool bCleanRace, bool bDefeatedRival, bool bFirstWinOnTrack)
{
	FMGRaceEarnings Earnings;

	// Generous base earnings - everyone gets something meaningful
	Earnings.BaseEarnings = 500 + (TotalRacers * 50);

	// Position bonus - rewards skill but doesn't punish lower positions
	if (Position == 1)
		Earnings.PositionBonus = 1000;
	else if (Position == 2)
		Earnings.PositionBonus = 750;
	else if (Position == 3)
		Earnings.PositionBonus = 500;
	else if (Position <= TotalRacers / 2)
		Earnings.PositionBonus = 250;
	else
		Earnings.PositionBonus = 100; // Still get something for participating

	// Clean race bonus (no collisions/penalties)
	if (bCleanRace)
		Earnings.CleanRaceBonus = 300;

	// Rival defeat bonus (story integration)
	if (bDefeatedRival)
		Earnings.RivalBonus = 500;

	// First win on track bonus (exploration reward)
	if (bFirstWinOnTrack && Position == 1)
		Earnings.FirstWinBonus = 1000;

	// Apply multipliers
	float TotalMultiplier = GetTotalMultiplier(EMGCurrencyType::GrindCash);
	int64 Subtotal = Earnings.BaseEarnings + Earnings.PositionBonus + Earnings.CleanRaceBonus + Earnings.RivalBonus + Earnings.FirstWinBonus;

	if (TotalMultiplier > 1.0f)
	{
		Earnings.MultiplierBonus = FMath::RoundToInt64(Subtotal * (TotalMultiplier - 1.0f));
		for (const FMGEarningMultiplier& Mult : ActiveMultipliers)
		{
			if (Mult.AffectedCurrency == EMGCurrencyType::GrindCash)
				Earnings.AppliedMultipliers.Add(Mult);
		}
	}

	Earnings.TotalEarnings = Subtotal + Earnings.MultiplierBonus + Earnings.CrewBonus;
	return Earnings;
}

void UMGCurrencySubsystem::AwardRaceEarnings(const FMGRaceEarnings& Earnings)
{
	EarnCurrency(EMGCurrencyType::GrindCash, Earnings.TotalEarnings, EMGEarnSource::RaceFinish, TEXT("Race completion"));
}

bool UMGCurrencySubsystem::CanClaimDailyBonus() const
{
	if (DailyBonus.bClaimedToday) return false;

	FDateTime Now = FDateTime::UtcNow();
	FDateTime LastClaim = DailyBonus.LastClaimDate;

	// Check if it's a new day
	return Now.GetDay() != LastClaim.GetDay() || Now.GetMonth() != LastClaim.GetMonth() || Now.GetYear() != LastClaim.GetYear();
}

bool UMGCurrencySubsystem::ClaimDailyBonus()
{
	if (!CanClaimDailyBonus()) return false;

	FDateTime Now = FDateTime::UtcNow();
	FDateTime LastClaim = DailyBonus.LastClaimDate;

	// Check if consecutive (within 48 hours to be forgiving)
	FTimespan TimeSinceLastClaim = Now - LastClaim;
	if (TimeSinceLastClaim.GetTotalHours() < 48.0)
	{
		DailyBonus.ConsecutiveDays++;
	}
	else
	{
		// Don't reset to 0, reset to 1 - be forgiving
		DailyBonus.ConsecutiveDays = 1;
	}

	// Cap at 30 days for max bonus
	DailyBonus.ConsecutiveDays = FMath::Min(DailyBonus.ConsecutiveDays, 30);

	// Calculate bonuses
	DailyBonus.TodayGrindCash = GetDailyBonusAmount(DailyBonus.ConsecutiveDays);

	// Every 7 days, award Neon Credits (earnable premium currency!)
	DailyBonus.WeeklyBonusProgress++;
	if (DailyBonus.WeeklyBonusProgress >= 7)
	{
		DailyBonus.TodayNeonCredits = GetWeeklyBonusNeonCredits();
		DailyBonus.WeeklyBonusProgress = 0;
	}
	else
	{
		DailyBonus.TodayNeonCredits = 0;
	}

	DailyBonus.LastClaimDate = Now;
	DailyBonus.bClaimedToday = true;

	// Award the currencies
	EarnCurrency(EMGCurrencyType::GrindCash, DailyBonus.TodayGrindCash, EMGEarnSource::DailyLogin, TEXT("Daily login bonus"));

	if (DailyBonus.TodayNeonCredits > 0)
	{
		EarnCurrency(EMGCurrencyType::NeonCredits, DailyBonus.TodayNeonCredits, EMGEarnSource::DailyLogin, TEXT("Weekly login bonus"));
	}

	OnDailyBonusClaimed.Broadcast(DailyBonus);
	SaveCurrencyData();

	return true;
}

int64 UMGCurrencySubsystem::GetDailyBonusAmount(int32 ConsecutiveDays) const
{
	// Generous scaling - quickly reaches good amounts
	// Day 1: 500, Day 7: 1500, Day 14: 2500, Day 30: 5000
	int64 Base = 500;
	int64 DayBonus = ConsecutiveDays * 150;
	return Base + DayBonus;
}

int64 UMGCurrencySubsystem::GetWeeklyBonusNeonCredits() const
{
	// 100 Neon Credits per week - meaningful amount of premium currency for free
	// This allows players to earn ~400-500 per month just from logging in
	return 100;
}

void UMGCurrencySubsystem::AddMultiplier(const FMGEarningMultiplier& Multiplier)
{
	// Check if already exists
	int32 ExistingIndex = ActiveMultipliers.IndexOfByPredicate(
		[&Multiplier](const FMGEarningMultiplier& M) { return M.MultiplierID == Multiplier.MultiplierID; });

	if (ExistingIndex != INDEX_NONE)
	{
		ActiveMultipliers[ExistingIndex] = Multiplier;
	}
	else
	{
		ActiveMultipliers.Add(Multiplier);
	}

	OnMultiplierActivated.Broadcast(Multiplier);
}

void UMGCurrencySubsystem::RemoveMultiplier(FName MultiplierID)
{
	int32 Index = ActiveMultipliers.IndexOfByPredicate(
		[MultiplierID](const FMGEarningMultiplier& M) { return M.MultiplierID == MultiplierID; });

	if (Index != INDEX_NONE)
	{
		ActiveMultipliers.RemoveAt(Index);
		OnMultiplierExpired.Broadcast(MultiplierID);
	}
}

float UMGCurrencySubsystem::GetTotalMultiplier(EMGCurrencyType Type) const
{
	float Total = 1.0f;
	FDateTime Now = FDateTime::UtcNow();

	for (const FMGEarningMultiplier& Mult : ActiveMultipliers)
	{
		if (Mult.AffectedCurrency == Type)
		{
			if (Mult.bIsPermanent || Mult.ExpiresAt > Now)
			{
				Total += (Mult.Multiplier - 1.0f); // Additive stacking
			}
		}
	}

	return Total;
}

TArray<FMGCurrencyTransaction> UMGCurrencySubsystem::GetRecentTransactions(int32 Count) const
{
	TArray<FMGCurrencyTransaction> Recent;
	int32 StartIndex = FMath::Max(0, TransactionHistory.Num() - Count);

	for (int32 i = TransactionHistory.Num() - 1; i >= StartIndex; i--)
	{
		Recent.Add(TransactionHistory[i]);
	}

	return Recent;
}

int64 UMGCurrencySubsystem::GetTotalEarned(EMGCurrencyType Type) const
{
	switch (Type)
	{
	case EMGCurrencyType::GrindCash: return TotalGrindCashEarned;
	case EMGCurrencyType::NeonCredits: return TotalNeonCreditsEarned;
	default: return 0;
	}
}

int64 UMGCurrencySubsystem::GetTotalSpent(EMGCurrencyType Type) const
{
	switch (Type)
	{
	case EMGCurrencyType::GrindCash: return TotalGrindCashSpent;
	case EMGCurrencyType::NeonCredits: return TotalNeonCreditsSpent;
	default: return 0;
	}
}

void UMGCurrencySubsystem::LoadCurrencyData()
{
	// Would load from cloud save
	// Start players with a welcome bonus
	if (CurrentBalance.GrindCash == 0 && TotalGrindCashEarned == 0)
	{
		CurrentBalance.GrindCash = 5000;  // Generous starting cash
		CurrentBalance.NeonCredits = 200; // Some premium currency to start
	}
}

void UMGCurrencySubsystem::SaveCurrencyData()
{
	// Would save to cloud save
}

void UMGCurrencySubsystem::UpdateMultipliers()
{
	FDateTime Now = FDateTime::UtcNow();

	for (int32 i = ActiveMultipliers.Num() - 1; i >= 0; i--)
	{
		if (!ActiveMultipliers[i].bIsPermanent && ActiveMultipliers[i].ExpiresAt <= Now)
		{
			FName ExpiredID = ActiveMultipliers[i].MultiplierID;
			ActiveMultipliers.RemoveAt(i);
			OnMultiplierExpired.Broadcast(ExpiredID);
		}
	}
}

FString UMGCurrencySubsystem::GenerateTransactionID()
{
	return FGuid::NewGuid().ToString();
}

void UMGCurrencySubsystem::RecordTransaction(EMGCurrencyType Type, int64 Amount, EMGEarnSource Source, const FString& Details)
{
	FMGCurrencyTransaction Transaction;
	Transaction.TransactionID = GenerateTransactionID();
	Transaction.CurrencyType = Type;
	Transaction.Amount = Amount;
	Transaction.Source = Source;
	Transaction.SourceDetails = Details;
	Transaction.Timestamp = FDateTime::UtcNow();
	Transaction.BalanceAfter = GetCurrencyAmount(Type);

	TransactionHistory.Add(Transaction);
	OnTransactionCompleted.Broadcast(Transaction);

	// Keep history reasonable
	if (TransactionHistory.Num() > 1000)
	{
		TransactionHistory.RemoveAt(0, 100);
	}
}

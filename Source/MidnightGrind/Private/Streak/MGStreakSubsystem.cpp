// Copyright Midnight Grind. All Rights Reserved.

#include "Streak/MGStreakSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGStreakSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Register default streak types
	{
		FMGStreakDefinition WinStreak;
		WinStreak.Type = EMGStreakType::Win;
		WinStreak.DisplayName = FText::FromString(TEXT("Win Streak"));
		WinStreak.Description = FText::FromString(TEXT("Consecutive race victories"));
		WinStreak.BronzeThreshold = 3;
		WinStreak.SilverThreshold = 5;
		WinStreak.GoldThreshold = 10;
		WinStreak.PlatinumThreshold = 15;
		WinStreak.DiamondThreshold = 25;
		WinStreak.ChampionThreshold = 50;
		WinStreak.LegendThreshold = 100;
		WinStreak.BaseMultiplier = 1.0f;
		WinStreak.MultiplierPerCount = 0.15f;
		WinStreak.MaxMultiplier = 5.0f;
		WinStreak.BasePointsPerCount = 500;
		RegisterStreakType(WinStreak);
	}
	{
		FMGStreakDefinition PodiumStreak;
		PodiumStreak.Type = EMGStreakType::Podium;
		PodiumStreak.DisplayName = FText::FromString(TEXT("Podium Streak"));
		PodiumStreak.Description = FText::FromString(TEXT("Consecutive top 3 finishes"));
		PodiumStreak.BronzeThreshold = 5;
		PodiumStreak.SilverThreshold = 10;
		PodiumStreak.GoldThreshold = 20;
		PodiumStreak.PlatinumThreshold = 35;
		PodiumStreak.DiamondThreshold = 50;
		PodiumStreak.ChampionThreshold = 75;
		PodiumStreak.LegendThreshold = 150;
		PodiumStreak.BaseMultiplier = 1.0f;
		PodiumStreak.MultiplierPerCount = 0.1f;
		PodiumStreak.MaxMultiplier = 4.0f;
		PodiumStreak.BasePointsPerCount = 200;
		RegisterStreakType(PodiumStreak);
	}
	{
		FMGStreakDefinition PerfectStreak;
		PerfectStreak.Type = EMGStreakType::Perfect;
		PerfectStreak.DisplayName = FText::FromString(TEXT("Perfect Streak"));
		PerfectStreak.Description = FText::FromString(TEXT("Consecutive perfect races (no crashes)"));
		PerfectStreak.BronzeThreshold = 3;
		PerfectStreak.SilverThreshold = 5;
		PerfectStreak.GoldThreshold = 8;
		PerfectStreak.PlatinumThreshold = 12;
		PerfectStreak.DiamondThreshold = 20;
		PerfectStreak.ChampionThreshold = 35;
		PerfectStreak.LegendThreshold = 50;
		PerfectStreak.BaseMultiplier = 1.0f;
		PerfectStreak.MultiplierPerCount = 0.2f;
		PerfectStreak.MaxMultiplier = 6.0f;
		PerfectStreak.BasePointsPerCount = 750;
		RegisterStreakType(PerfectStreak);
	}
	{
		FMGStreakDefinition TakedownStreak;
		TakedownStreak.Type = EMGStreakType::Takedown;
		TakedownStreak.DisplayName = FText::FromString(TEXT("Takedown Streak"));
		TakedownStreak.Description = FText::FromString(TEXT("Consecutive takedowns without being taken down"));
		TakedownStreak.BronzeThreshold = 3;
		TakedownStreak.SilverThreshold = 5;
		TakedownStreak.GoldThreshold = 10;
		TakedownStreak.PlatinumThreshold = 15;
		TakedownStreak.DiamondThreshold = 25;
		TakedownStreak.ChampionThreshold = 40;
		TakedownStreak.LegendThreshold = 75;
		TakedownStreak.BaseMultiplier = 1.0f;
		TakedownStreak.MultiplierPerCount = 0.25f;
		TakedownStreak.MaxMultiplier = 8.0f;
		TakedownStreak.BasePointsPerCount = 300;
		RegisterStreakType(TakedownStreak);
	}
	{
		FMGStreakDefinition DailyStreak;
		DailyStreak.Type = EMGStreakType::Daily;
		DailyStreak.DisplayName = FText::FromString(TEXT("Daily Streak"));
		DailyStreak.Description = FText::FromString(TEXT("Consecutive days of racing"));
		DailyStreak.BronzeThreshold = 3;
		DailyStreak.SilverThreshold = 7;
		DailyStreak.GoldThreshold = 14;
		DailyStreak.PlatinumThreshold = 30;
		DailyStreak.DiamondThreshold = 60;
		DailyStreak.ChampionThreshold = 100;
		DailyStreak.LegendThreshold = 365;
		DailyStreak.BaseMultiplier = 1.0f;
		DailyStreak.MultiplierPerCount = 0.05f;
		DailyStreak.MaxMultiplier = 3.0f;
		DailyStreak.BasePointsPerCount = 100;
		DailyStreak.bHasDailyReset = true;
		DailyStreak.FreezeTokensAllowed = 3;
		RegisterStreakType(DailyStreak);
	}
	{
		FMGStreakDefinition RankedStreak;
		RankedStreak.Type = EMGStreakType::Ranked;
		RankedStreak.DisplayName = FText::FromString(TEXT("Ranked Win Streak"));
		RankedStreak.Description = FText::FromString(TEXT("Consecutive ranked victories"));
		RankedStreak.BronzeThreshold = 3;
		RankedStreak.SilverThreshold = 5;
		RankedStreak.GoldThreshold = 8;
		RankedStreak.PlatinumThreshold = 12;
		RankedStreak.DiamondThreshold = 18;
		RankedStreak.ChampionThreshold = 30;
		RankedStreak.LegendThreshold = 50;
		RankedStreak.BaseMultiplier = 1.0f;
		RankedStreak.MultiplierPerCount = 0.2f;
		RankedStreak.MaxMultiplier = 6.0f;
		RankedStreak.BasePointsPerCount = 1000;
		RegisterStreakType(RankedStreak);
	}
	{
		FMGStreakDefinition CleanStreak;
		CleanStreak.Type = EMGStreakType::Clean;
		CleanStreak.DisplayName = FText::FromString(TEXT("Clean Race Streak"));
		CleanStreak.Description = FText::FromString(TEXT("Consecutive clean races (no penalties)"));
		CleanStreak.BronzeThreshold = 3;
		CleanStreak.SilverThreshold = 6;
		CleanStreak.GoldThreshold = 10;
		CleanStreak.PlatinumThreshold = 15;
		CleanStreak.DiamondThreshold = 25;
		CleanStreak.ChampionThreshold = 40;
		CleanStreak.LegendThreshold = 75;
		CleanStreak.BaseMultiplier = 1.0f;
		CleanStreak.MultiplierPerCount = 0.1f;
		CleanStreak.MaxMultiplier = 3.0f;
		CleanStreak.BasePointsPerCount = 250;
		RegisterStreakType(CleanStreak);
	}

	// Load saved data
	LoadStreakData();

	// Start streak tick
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGStreakSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(StreakTickTimer, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->TickStreaks(0.033f);
			}
		}, 0.033f, true);
	}
}

void UMGStreakSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(StreakTickTimer);
	}

	SaveStreakData();

	StreakDefinitions.Empty();
	PlayerStreaks.Empty();
	ActiveCombos.Empty();
	DailyStreaks.Empty();
	PlayerStats.Empty();
	TierRewards.Empty();
	ClaimedRewards.Empty();

	Super::Deinitialize();
}

// Streak Management
void UMGStreakSubsystem::RegisterStreakType(const FMGStreakDefinition& Definition)
{
	StreakDefinitions.Add(Definition.Type, Definition);
}

FMGStreakDefinition UMGStreakSubsystem::GetStreakDefinition(EMGStreakType Type) const
{
	if (const FMGStreakDefinition* Found = StreakDefinitions.Find(Type))
	{
		return *Found;
	}
	return FMGStreakDefinition();
}

void UMGStreakSubsystem::IncrementStreak(const FString& PlayerId, EMGStreakType Type)
{
	TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId);
	if (!PlayerData)
	{
		TMap<EMGStreakType, FMGActiveStreak> NewData;
		PlayerData = &PlayerStreaks.Add(PlayerId, NewData);
	}

	FMGActiveStreak* Streak = PlayerData->Find(Type);
	if (!Streak)
	{
		FMGActiveStreak NewStreak;
		NewStreak.StreakId = GenerateStreakId();
		NewStreak.PlayerId = PlayerId;
		NewStreak.Type = Type;
		NewStreak.Status = EMGStreakStatus::Active;
		NewStreak.StartTime = FDateTime::Now();
		Streak = &PlayerData->Add(Type, NewStreak);
	}

	// Unfreeze if frozen
	if (Streak->Status == EMGStreakStatus::Frozen)
	{
		Streak->Status = EMGStreakStatus::Active;
	}

	// Increment count
	Streak->CurrentCount++;
	Streak->LastUpdateTime = FDateTime::Now();
	Streak->Status = EMGStreakStatus::Active;

	// Update best if needed
	bool bNewRecord = false;
	if (Streak->CurrentCount > Streak->BestCount)
	{
		Streak->BestCount = Streak->CurrentCount;
		bNewRecord = true;
		OnNewStreakRecord.Broadcast(PlayerId, Type);
	}

	// Calculate new tier and multiplier
	EMGStreakTier OldTier = Streak->CurrentTier;
	Streak->CurrentTier = CalculateTier(Type, Streak->CurrentCount);
	Streak->CurrentMultiplier = CalculateMultiplier(Type, Streak->CurrentCount);
	Streak->NextTierThreshold = GetTierThreshold(Type, static_cast<EMGStreakTier>(static_cast<uint8>(Streak->CurrentTier) + 1));

	// Calculate points
	const FMGStreakDefinition* Def = StreakDefinitions.Find(Type);
	if (Def)
	{
		int32 Points = FMath::RoundToInt(Def->BasePointsPerCount * Streak->CurrentMultiplier);
		Streak->TotalPointsEarned += Points;
	}

	// Check for tier upgrade
	if (Streak->CurrentTier > OldTier)
	{
		OnStreakTierReached.Broadcast(PlayerId, Type, Streak->CurrentTier);
		AwardTierRewards(PlayerId, Type, Streak->CurrentTier);
	}

	// Update stats
	UpdatePlayerStats(PlayerId, Type, Streak->CurrentCount);

	OnStreakUpdated.Broadcast(PlayerId, Type, Streak->CurrentCount, Streak->CurrentMultiplier);
}

void UMGStreakSubsystem::BreakStreak(const FString& PlayerId, EMGStreakType Type)
{
	TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId);
	if (!PlayerData)
	{
		return;
	}

	FMGActiveStreak* Streak = PlayerData->Find(Type);
	if (!Streak || Streak->Status == EMGStreakStatus::Inactive || Streak->Status == EMGStreakStatus::Frozen)
	{
		return;
	}

	int32 FinalCount = Streak->CurrentCount;
	Streak->Status = EMGStreakStatus::Broken;

	OnStreakBroken.Broadcast(PlayerId, Type, FinalCount);

	// Reset streak
	Streak->CurrentCount = 0;
	Streak->CurrentTier = EMGStreakTier::None;
	Streak->CurrentMultiplier = 1.0f;
	Streak->NextTierThreshold = GetTierThreshold(Type, EMGStreakTier::Bronze);
	Streak->Status = EMGStreakStatus::Inactive;
}

void UMGStreakSubsystem::ResetStreak(const FString& PlayerId, EMGStreakType Type)
{
	TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId);
	if (!PlayerData)
	{
		return;
	}

	FMGActiveStreak* Streak = PlayerData->Find(Type);
	if (!Streak)
	{
		return;
	}

	Streak->CurrentCount = 0;
	Streak->CurrentTier = EMGStreakTier::None;
	Streak->CurrentMultiplier = 1.0f;
	Streak->Status = EMGStreakStatus::Inactive;
	Streak->TotalPointsEarned = 0;
	Streak->FreezeTokensUsed = 0;
}

bool UMGStreakSubsystem::FreezeStreak(const FString& PlayerId, EMGStreakType Type)
{
	TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId);
	if (!PlayerData)
	{
		return false;
	}

	FMGActiveStreak* Streak = PlayerData->Find(Type);
	if (!Streak || Streak->Status != EMGStreakStatus::Active)
	{
		return false;
	}

	if (Streak->FreezeTokensUsed >= Streak->MaxFreezeTokens)
	{
		return false;
	}

	Streak->Status = EMGStreakStatus::Frozen;
	Streak->FreezeTokensUsed++;

	OnStreakFrozen.Broadcast(PlayerId, Type);
	return true;
}

void UMGStreakSubsystem::UnfreezeStreak(const FString& PlayerId, EMGStreakType Type)
{
	TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId);
	if (!PlayerData)
	{
		return;
	}

	FMGActiveStreak* Streak = PlayerData->Find(Type);
	if (!Streak || Streak->Status != EMGStreakStatus::Frozen)
	{
		return;
	}

	Streak->Status = EMGStreakStatus::Active;
	Streak->LastUpdateTime = FDateTime::Now();
}

// Streak Queries
FMGActiveStreak UMGStreakSubsystem::GetActiveStreak(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return *Streak;
		}
	}
	return FMGActiveStreak();
}

TArray<FMGActiveStreak> UMGStreakSubsystem::GetAllActiveStreaks(const FString& PlayerId) const
{
	TArray<FMGActiveStreak> Result;

	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		for (const auto& Pair : *PlayerData)
		{
			if (Pair.Value.Status == EMGStreakStatus::Active || Pair.Value.Status == EMGStreakStatus::Frozen)
			{
				Result.Add(Pair.Value);
			}
		}
	}

	return Result;
}

int32 UMGStreakSubsystem::GetCurrentStreakCount(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return Streak->CurrentCount;
		}
	}
	return 0;
}

int32 UMGStreakSubsystem::GetBestStreakCount(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return Streak->BestCount;
		}
	}
	return 0;
}

float UMGStreakSubsystem::GetStreakMultiplier(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return Streak->CurrentMultiplier;
		}
	}
	return 1.0f;
}

EMGStreakTier UMGStreakSubsystem::GetStreakTier(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return Streak->CurrentTier;
		}
	}
	return EMGStreakTier::None;
}

bool UMGStreakSubsystem::IsStreakActive(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return Streak->Status == EMGStreakStatus::Active || Streak->Status == EMGStreakStatus::Frozen;
		}
	}
	return false;
}

bool UMGStreakSubsystem::IsStreakAtRisk(const FString& PlayerId, EMGStreakType Type) const
{
	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		if (const FMGActiveStreak* Streak = PlayerData->Find(Type))
		{
			return Streak->Status == EMGStreakStatus::AtRisk;
		}
	}
	return false;
}

// Combo System
void UMGStreakSubsystem::StartCombo(const FString& PlayerId, EMGComboType Type)
{
	if (ActiveCombos.Contains(PlayerId))
	{
		// Already have an active combo, add to it instead
		AddComboHit(PlayerId, Type, 0);
		return;
	}

	FMGActiveCombo NewCombo;
	NewCombo.ComboId = GenerateComboId();
	NewCombo.PlayerId = PlayerId;
	NewCombo.Type = Type;
	NewCombo.bIsActive = true;
	NewCombo.ComboTimer = NewCombo.MaxComboTime;
	NewCombo.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	NewCombo.ContributingTypes.Add(Type);

	ActiveCombos.Add(PlayerId, NewCombo);
}

void UMGStreakSubsystem::AddComboHit(const FString& PlayerId, EMGComboType Type, int32 Points)
{
	FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId);
	if (!Combo)
	{
		StartCombo(PlayerId, Type);
		Combo = ActiveCombos.Find(PlayerId);
		if (!Combo) return;
	}

	Combo->HitCount++;

	// Add contributing type if new
	if (!Combo->ContributingTypes.Contains(Type))
	{
		Combo->ContributingTypes.Add(Type);
		// Mixed combo bonus
		if (Combo->ContributingTypes.Num() >= 3)
		{
			Combo->Type = EMGComboType::MixedCombo;
		}
	}

	// Calculate multiplier
	Combo->CurrentMultiplier = 1.0f + (Combo->HitCount - 1) * 0.1f;
	Combo->CurrentMultiplier = FMath::Min(Combo->CurrentMultiplier, 10.0f);

	// Add points with multiplier
	int32 MultipliedPoints = FMath::RoundToInt(Points * Combo->CurrentMultiplier);
	Combo->CurrentScore += MultipliedPoints;

	// Reset timer
	Combo->ComboTimer = Combo->MaxComboTime;

	// Check for mega combo
	if (Combo->HitCount >= 10 && Combo->Type != EMGComboType::MegaCombo)
	{
		Combo->Type = EMGComboType::MegaCombo;
		OnMegaCombo.Broadcast(PlayerId, Combo->HitCount, Combo->CurrentScore);
	}

	OnComboHit.Broadcast(PlayerId, Type, Combo->HitCount, MultipliedPoints);
}

void UMGStreakSubsystem::ExtendComboTimer(const FString& PlayerId, float AdditionalTime)
{
	FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId);
	if (!Combo || !Combo->bIsActive)
	{
		return;
	}

	Combo->ComboTimer = FMath::Min(Combo->ComboTimer + AdditionalTime, Combo->MaxComboTime * 2.0f);
}

FMGComboResult UMGStreakSubsystem::EndCombo(const FString& PlayerId)
{
	FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId);
	if (!Combo)
	{
		return FMGComboResult();
	}

	// Calculate final duration
	float EndTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	Combo->TotalDuration = EndTime - Combo->StartTime;

	FMGComboResult Result;
	Result.ResultId = GenerateResultId();
	Result.PlayerId = PlayerId;
	Result.FinalType = Combo->Type;
	Result.TotalHits = Combo->HitCount;
	Result.TotalScore = Combo->CurrentScore;
	Result.FinalMultiplier = Combo->CurrentMultiplier;
	Result.Duration = Combo->TotalDuration;
	Result.TypesUsed = Combo->ContributingTypes;
	Result.Timestamp = FDateTime::Now();

	// Check for personal best
	FMGStreakPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats && Combo->HitCount > Stats->HighestComboHits)
	{
		Result.bIsPersonalBest = true;
	}

	// Update stats
	if (!Stats)
	{
		FMGStreakPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	Stats->TotalCombosCompleted++;
	Stats->TotalComboPoints += Combo->CurrentScore;
	if (Combo->HitCount > Stats->HighestComboHits)
	{
		Stats->HighestComboHits = Combo->HitCount;
	}
	if (Combo->CurrentScore > Stats->HighestComboScore)
	{
		Stats->HighestComboScore = Combo->CurrentScore;
	}
	if (Combo->TotalDuration > Stats->LongestComboDuration)
	{
		Stats->LongestComboDuration = Combo->TotalDuration;
	}
	if (Combo->Type == EMGComboType::MegaCombo)
	{
		Stats->MegaCombosAchieved++;
	}

	OnComboEnded.Broadcast(PlayerId, Result);

	ActiveCombos.Remove(PlayerId);

	return Result;
}

void UMGStreakSubsystem::DropCombo(const FString& PlayerId)
{
	FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId);
	if (!Combo)
	{
		return;
	}

	// End combo with whatever was accumulated
	EndCombo(PlayerId);
}

FMGActiveCombo UMGStreakSubsystem::GetActiveCombo(const FString& PlayerId) const
{
	if (const FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId))
	{
		return *Combo;
	}
	return FMGActiveCombo();
}

bool UMGStreakSubsystem::HasActiveCombo(const FString& PlayerId) const
{
	const FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId);
	return Combo && Combo->bIsActive;
}

float UMGStreakSubsystem::GetComboTimeRemaining(const FString& PlayerId) const
{
	if (const FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId))
	{
		return Combo->ComboTimer;
	}
	return 0.0f;
}

// Daily Streak
void UMGStreakSubsystem::RecordDailyLogin(const FString& PlayerId)
{
	FMGDailyStreakData* DailyData = DailyStreaks.Find(PlayerId);
	if (!DailyData)
	{
		FMGDailyStreakData NewData;
		NewData.PlayerId = PlayerId;
		NewData.StreakStartDate = FDateTime::Now();
		DailyData = &DailyStreaks.Add(PlayerId, NewData);
	}

	FDateTime Now = FDateTime::Now();
	FDateTime Today = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay());
	FDateTime LastLogin = FDateTime(DailyData->LastLoginDate.GetYear(), DailyData->LastLoginDate.GetMonth(), DailyData->LastLoginDate.GetDay());

	// Check if this is a new day
	if (Today != LastLogin)
	{
		FTimespan TimeSinceLastLogin = Today - LastLogin;
		int32 DaysSince = TimeSinceLastLogin.GetDays();

		if (DaysSince == 1)
		{
			// Consecutive day
			DailyData->CurrentDayStreak++;
		}
		else if (DaysSince > 1 && DailyData->FreezeTokensAvailable > 0)
		{
			// Missed days but have freeze token
			DailyData->FreezeTokensAvailable--;
			// Still counts as continuing streak
		}
		else if (DaysSince > 1)
		{
			// Streak broken
			DailyData->CurrentDayStreak = 1;
			DailyData->StreakStartDate = Today;
		}

		DailyData->bPlayedToday = true;
		DailyData->LastLoginDate = Now;
		DailyData->TotalDaysPlayed++;

		// Check for new record
		bool bNewRecord = false;
		if (DailyData->CurrentDayStreak > DailyData->BestDayStreak)
		{
			DailyData->BestDayStreak = DailyData->CurrentDayStreak;
			bNewRecord = true;
		}

		// Update the daily streak type
		IncrementStreak(PlayerId, EMGStreakType::Daily);

		OnDailyStreakUpdated.Broadcast(PlayerId, DailyData->CurrentDayStreak, bNewRecord);
	}
}

void UMGStreakSubsystem::CompleteDailyChallenge(const FString& PlayerId)
{
	FMGDailyStreakData* DailyData = DailyStreaks.Find(PlayerId);
	if (!DailyData)
	{
		return;
	}

	DailyData->bCompletedDailyChallenge = true;
}

FMGDailyStreakData UMGStreakSubsystem::GetDailyStreakData(const FString& PlayerId) const
{
	if (const FMGDailyStreakData* Data = DailyStreaks.Find(PlayerId))
	{
		return *Data;
	}
	return FMGDailyStreakData();
}

int32 UMGStreakSubsystem::GetDailyStreakCount(const FString& PlayerId) const
{
	if (const FMGDailyStreakData* Data = DailyStreaks.Find(PlayerId))
	{
		return Data->CurrentDayStreak;
	}
	return 0;
}

bool UMGStreakSubsystem::UseDailyFreezeToken(const FString& PlayerId)
{
	FMGDailyStreakData* DailyData = DailyStreaks.Find(PlayerId);
	if (!DailyData || DailyData->FreezeTokensAvailable <= 0)
	{
		return false;
	}

	DailyData->FreezeTokensAvailable--;
	return true;
}

int32 UMGStreakSubsystem::GetDailyFreezeTokensAvailable(const FString& PlayerId) const
{
	if (const FMGDailyStreakData* Data = DailyStreaks.Find(PlayerId))
	{
		return Data->FreezeTokensAvailable;
	}
	return 0;
}

// Tier Rewards
void UMGStreakSubsystem::RegisterTierReward(EMGStreakType Type, const FMGStreakTierReward& Reward)
{
	TMap<EMGStreakTier, FMGStreakTierReward>* TypeRewards = TierRewards.Find(Type);
	if (!TypeRewards)
	{
		TMap<EMGStreakTier, FMGStreakTierReward> NewMap;
		TypeRewards = &TierRewards.Add(Type, NewMap);
	}

	TypeRewards->Add(Reward.Tier, Reward);
}

FMGStreakTierReward UMGStreakSubsystem::GetTierReward(EMGStreakType Type, EMGStreakTier Tier) const
{
	if (const TMap<EMGStreakTier, FMGStreakTierReward>* TypeRewards = TierRewards.Find(Type))
	{
		if (const FMGStreakTierReward* Reward = TypeRewards->Find(Tier))
		{
			return *Reward;
		}
	}
	return FMGStreakTierReward();
}

TArray<FMGStreakTierReward> UMGStreakSubsystem::ClaimAvailableRewards(const FString& PlayerId, EMGStreakType Type)
{
	TArray<FMGStreakTierReward> ClaimedRewardsList;

	EMGStreakTier CurrentTier = GetStreakTier(PlayerId, Type);
	if (CurrentTier == EMGStreakTier::None)
	{
		return ClaimedRewardsList;
	}

	TSet<FString>* PlayerClaimed = ClaimedRewards.Find(PlayerId);
	if (!PlayerClaimed)
	{
		TSet<FString> NewSet;
		PlayerClaimed = &ClaimedRewards.Add(PlayerId, NewSet);
	}

	const TMap<EMGStreakTier, FMGStreakTierReward>* TypeRewards = TierRewards.Find(Type);
	if (!TypeRewards)
	{
		return ClaimedRewardsList;
	}

	for (uint8 i = 1; i <= static_cast<uint8>(CurrentTier); i++)
	{
		EMGStreakTier Tier = static_cast<EMGStreakTier>(i);
		FString RewardKey = FString::Printf(TEXT("%d_%d"), static_cast<uint8>(Type), i);

		if (PlayerClaimed->Contains(RewardKey))
		{
			continue;
		}

		if (const FMGStreakTierReward* Reward = TypeRewards->Find(Tier))
		{
			ClaimedRewardsList.Add(*Reward);
			PlayerClaimed->Add(RewardKey);
		}
	}

	return ClaimedRewardsList;
}

// Stats
FMGStreakPlayerStats UMGStreakSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGStreakPlayerStats* Stats = PlayerStats.Find(PlayerId))
	{
		return *Stats;
	}
	return FMGStreakPlayerStats();
}

void UMGStreakSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGStreakPlayerStats NewStats;
	NewStats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, NewStats);
}

// Combined Multiplier
float UMGStreakSubsystem::GetCombinedMultiplier(const FString& PlayerId) const
{
	float Combined = 1.0f;

	if (const TMap<EMGStreakType, FMGActiveStreak>* PlayerData = PlayerStreaks.Find(PlayerId))
	{
		for (const auto& Pair : *PlayerData)
		{
			if (Pair.Value.Status == EMGStreakStatus::Active)
			{
				Combined *= Pair.Value.CurrentMultiplier;
			}
		}
	}

	// Add combo multiplier
	Combined *= GetComboMultiplier(PlayerId);

	return Combined;
}

float UMGStreakSubsystem::GetComboMultiplier(const FString& PlayerId) const
{
	if (const FMGActiveCombo* Combo = ActiveCombos.Find(PlayerId))
	{
		if (Combo->bIsActive)
		{
			return Combo->CurrentMultiplier;
		}
	}
	return 1.0f;
}

// Update
void UMGStreakSubsystem::UpdateStreakSystem(float MGDeltaTime)
{
	TickStreaks(DeltaTime);
}

// Protected
void UMGStreakSubsystem::TickStreaks(float MGDeltaTime)
{
	UpdateCombos(DeltaTime);
	CheckStreakExpirations();
}

void UMGStreakSubsystem::UpdateCombos(float MGDeltaTime)
{
	TArray<FString> EndedCombos;

	for (auto& Pair : ActiveCombos)
	{
		if (!Pair.Value.bIsActive)
		{
			continue;
		}

		Pair.Value.ComboTimer -= DeltaTime;

		if (Pair.Value.ComboTimer <= 0.0f)
		{
			EndedCombos.Add(Pair.Key);
		}
	}

	for (const FString& PlayerId : EndedCombos)
	{
		EndCombo(PlayerId);
	}
}

void UMGStreakSubsystem::CheckStreakExpirations()
{
	FDateTime Now = FDateTime::Now();

	for (auto& PlayerPair : PlayerStreaks)
	{
		for (auto& StreakPair : PlayerPair.Value)
		{
			FMGActiveStreak& Streak = StreakPair.Value;

			if (Streak.Status != EMGStreakStatus::Active || !Streak.bHasExpiration)
			{
				continue;
			}

			if (Now >= Streak.ExpirationTime)
			{
				// Check if at risk first
				FTimespan TimeUntilExpire = Streak.ExpirationTime - Now;
				if (TimeUntilExpire.GetTotalHours() <= 1.0 && Streak.Status != EMGStreakStatus::AtRisk)
				{
					Streak.Status = EMGStreakStatus::AtRisk;
					OnStreakAtRisk.Broadcast(Streak.PlayerId, Streak.Type, TimeUntilExpire.GetTotalSeconds());
				}
				else if (TimeUntilExpire.GetTotalSeconds() <= 0)
				{
					BreakStreak(Streak.PlayerId, Streak.Type);
				}
			}
		}
	}
}

void UMGStreakSubsystem::CheckDailyReset(const FString& PlayerId)
{
	FMGDailyStreakData* DailyData = DailyStreaks.Find(PlayerId);
	if (!DailyData)
	{
		return;
	}

	FDateTime Now = FDateTime::Now();
	FDateTime Today = FDateTime(Now.GetYear(), Now.GetMonth(), Now.GetDay());
	FDateTime LastLogin = FDateTime(DailyData->LastLoginDate.GetYear(), DailyData->LastLoginDate.GetMonth(), DailyData->LastLoginDate.GetDay());

	if (Today != LastLogin)
	{
		DailyData->bPlayedToday = false;
		DailyData->bCompletedDailyChallenge = false;
	}
}

EMGStreakTier UMGStreakSubsystem::CalculateTier(EMGStreakType Type, int32 Count) const
{
	const FMGStreakDefinition* Def = StreakDefinitions.Find(Type);
	if (!Def)
	{
		return EMGStreakTier::None;
	}

	if (Count >= Def->LegendThreshold) return EMGStreakTier::Legend;
	if (Count >= Def->ChampionThreshold) return EMGStreakTier::Champion;
	if (Count >= Def->DiamondThreshold) return EMGStreakTier::Diamond;
	if (Count >= Def->PlatinumThreshold) return EMGStreakTier::Platinum;
	if (Count >= Def->GoldThreshold) return EMGStreakTier::Gold;
	if (Count >= Def->SilverThreshold) return EMGStreakTier::Silver;
	if (Count >= Def->BronzeThreshold) return EMGStreakTier::Bronze;

	return EMGStreakTier::None;
}

float UMGStreakSubsystem::CalculateMultiplier(EMGStreakType Type, int32 Count) const
{
	const FMGStreakDefinition* Def = StreakDefinitions.Find(Type);
	if (!Def)
	{
		return 1.0f;
	}

	float Multiplier = Def->BaseMultiplier + (Count * Def->MultiplierPerCount);
	return FMath::Min(Multiplier, Def->MaxMultiplier);
}

int32 UMGStreakSubsystem::GetTierThreshold(EMGStreakType Type, EMGStreakTier Tier) const
{
	const FMGStreakDefinition* Def = StreakDefinitions.Find(Type);
	if (!Def)
	{
		return 0;
	}

	switch (Tier)
	{
		case EMGStreakTier::Bronze: return Def->BronzeThreshold;
		case EMGStreakTier::Silver: return Def->SilverThreshold;
		case EMGStreakTier::Gold: return Def->GoldThreshold;
		case EMGStreakTier::Platinum: return Def->PlatinumThreshold;
		case EMGStreakTier::Diamond: return Def->DiamondThreshold;
		case EMGStreakTier::Champion: return Def->ChampionThreshold;
		case EMGStreakTier::Legend: return Def->LegendThreshold;
		default: return 0;
	}
}

void UMGStreakSubsystem::AwardTierRewards(const FString& PlayerId, EMGStreakType Type, EMGStreakTier Tier)
{
	// Tier rewards are claimed via ClaimAvailableRewards
	// This function can trigger any immediate effects
}

void UMGStreakSubsystem::UpdatePlayerStats(const FString& PlayerId, EMGStreakType Type, int32 Count)
{
	FMGStreakPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGStreakPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	// Update best streak
	int32* BestStreak = Stats->BestStreaks.Find(Type);
	if (!BestStreak || Count > *BestStreak)
	{
		Stats->BestStreaks.Add(Type, Count);
	}

	// Update total count
	int32& TotalCount = Stats->TotalStreakCounts.FindOrAdd(Type);
	TotalCount++;

	// Update tier achieved
	EMGStreakTier Tier = CalculateTier(Type, Count);
	if (Tier != EMGStreakTier::None)
	{
		int32& TierCount = Stats->TiersAchieved.FindOrAdd(Tier);
		TierCount++;
	}
}

FString UMGStreakSubsystem::GenerateStreakId() const
{
	return FString::Printf(TEXT("STREAK_%d_%lld"), ++const_cast<UMGStreakSubsystem*>(this)->StreakCounter, FDateTime::Now().GetTicks());
}

FString UMGStreakSubsystem::GenerateComboId() const
{
	return FString::Printf(TEXT("COMBO_%d_%lld"), ++const_cast<UMGStreakSubsystem*>(this)->ComboCounter, FDateTime::Now().GetTicks());
}

FString UMGStreakSubsystem::GenerateResultId() const
{
	return FString::Printf(TEXT("COMBORESULT_%d_%lld"), ++const_cast<UMGStreakSubsystem*>(this)->ResultCounter, FDateTime::Now().GetTicks());
}

// Persistence
void UMGStreakSubsystem::SaveStreakData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Streak");
	IFileManager::Get().MakeDirectory(*SaveDir, true);
	FString FilePath = SaveDir / TEXT("streak_data.dat");

	FBufferArchive SaveArchive;

	// Version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save player streaks
	int32 NumPlayers = PlayerStreaks.Num();
	SaveArchive << NumPlayers;

	for (const auto& PlayerPair : PlayerStreaks)
	{
		FString PlayerId = PlayerPair.Key;
		SaveArchive << PlayerId;

		int32 NumStreaks = PlayerPair.Value.Num();
		SaveArchive << NumStreaks;

		for (const auto& StreakPair : PlayerPair.Value)
		{
			int32 TypeInt = static_cast<int32>(StreakPair.Key);
			SaveArchive << TypeInt;

			const FMGActiveStreak& Streak = StreakPair.Value;
			FString StreakId = Streak.StreakId;
			int32 StatusInt = static_cast<int32>(Streak.Status);
			int32 TierInt = static_cast<int32>(Streak.CurrentTier);
			int32 CurrentCount = Streak.CurrentCount;
			int32 BestCount = Streak.BestCount;
			int32 TotalPoints = Streak.TotalPointsEarned;
			int64 StartTimeTicks = Streak.StartTime.GetTicks();
			int64 LastUpdateTicks = Streak.LastUpdateTime.GetTicks();

			SaveArchive << StreakId;
			SaveArchive << StatusInt;
			SaveArchive << TierInt;
			SaveArchive << CurrentCount;
			SaveArchive << BestCount;
			SaveArchive << TotalPoints;
			SaveArchive << StartTimeTicks;
			SaveArchive << LastUpdateTicks;
		}
	}

	// Save player stats
	int32 NumStats = PlayerStats.Num();
	SaveArchive << NumStats;

	for (const auto& StatPair : PlayerStats)
	{
		FString PlayerId = StatPair.Key;
		SaveArchive << PlayerId;

		const FMGStreakPlayerStats& Stats = StatPair.Value;
		int32 TotalCombos = Stats.TotalCombosCompleted;
		int32 HighestCombo = Stats.HighestComboHits;
		int32 HighestComboScore = Stats.HighestComboScore;
		int32 MegaCombos = Stats.MegaCombosAchieved;

		SaveArchive << TotalCombos;
		SaveArchive << HighestCombo;
		SaveArchive << HighestComboScore;
		SaveArchive << MegaCombos;

		// Save best streaks map
		int32 NumBestStreaks = Stats.BestStreaks.Num();
		SaveArchive << NumBestStreaks;
		for (const auto& BestPair : Stats.BestStreaks)
		{
			int32 TypeInt = static_cast<int32>(BestPair.Key);
			int32 Count = BestPair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}
	}

	// Save daily streak data
	int32 NumDaily = DailyStreaks.Num();
	SaveArchive << NumDaily;

	for (const auto& DailyPair : DailyStreaks)
	{
		FString PlayerId = DailyPair.Key;
		SaveArchive << PlayerId;

		const FMGDailyStreakData& Daily = DailyPair.Value;
		int32 CurrentStreak = Daily.CurrentDayStreak;
		int32 BestStreak = Daily.BestDayStreak;
		int64 LastLoginTicks = Daily.LastLoginDate.GetTicks();
		int32 TotalDaysPlayed = Daily.TotalDaysPlayed;

		SaveArchive << CurrentStreak;
		SaveArchive << BestStreak;
		SaveArchive << LastLoginTicks;
		SaveArchive << TotalDaysPlayed;
	}

	// Write to file
	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	UE_LOG(LogTemp, Log, TEXT("MGStreakSubsystem: Saved streak data for %d players"), NumPlayers);
}

void UMGStreakSubsystem::LoadStreakData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Streak") / TEXT("streak_data.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("MGStreakSubsystem: No saved streak data found"));
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGStreakSubsystem: Unknown save version %d"), Version);
		return;
	}

	// Load player streaks
	int32 NumPlayers;
	LoadArchive << NumPlayers;

	for (int32 i = 0; i < NumPlayers; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		int32 NumStreaks;
		LoadArchive << NumStreaks;

		TMap<EMGStreakType, FMGActiveStreak> Streaks;

		for (int32 j = 0; j < NumStreaks; ++j)
		{
			int32 TypeInt;
			LoadArchive << TypeInt;

			FMGActiveStreak Streak;
			Streak.PlayerId = PlayerId;
			Streak.Type = static_cast<EMGStreakType>(TypeInt);

			int32 StatusInt;
			int32 TierInt;
			int64 StartTimeTicks;
			int64 LastUpdateTicks;

			LoadArchive << Streak.StreakId;
			LoadArchive << StatusInt;
			LoadArchive << TierInt;
			LoadArchive << Streak.CurrentCount;
			LoadArchive << Streak.BestCount;
			LoadArchive << Streak.TotalPointsEarned;
			LoadArchive << StartTimeTicks;
			LoadArchive << LastUpdateTicks;

			Streak.Status = static_cast<EMGStreakStatus>(StatusInt);
			Streak.CurrentTier = static_cast<EMGStreakTier>(TierInt);
			Streak.StartTime = FDateTime(StartTimeTicks);
			Streak.LastUpdateTime = FDateTime(LastUpdateTicks);

			Streaks.Add(Streak.Type, Streak);
		}

		PlayerStreaks.Add(PlayerId, Streaks);
	}

	// Load player stats
	int32 NumStats;
	LoadArchive << NumStats;

	for (int32 i = 0; i < NumStats; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGStreakPlayerStats Stats;
		Stats.PlayerId = PlayerId;

		LoadArchive << Stats.TotalCombosCompleted;
		LoadArchive << Stats.HighestComboHits;
		LoadArchive << Stats.HighestComboScore;
		LoadArchive << Stats.MegaCombosAchieved;

		// Load best streaks map
		int32 NumBestStreaks;
		LoadArchive << NumBestStreaks;
		for (int32 j = 0; j < NumBestStreaks; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.BestStreaks.Add(static_cast<EMGStreakType>(TypeInt), Count);
		}

		PlayerStats.Add(PlayerId, Stats);
	}

	// Load daily streak data
	int32 NumDaily;
	LoadArchive << NumDaily;

	for (int32 i = 0; i < NumDaily; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGDailyStreakData Daily;
		Daily.PlayerId = PlayerId;

		int64 LastLoginTicks;

		LoadArchive << Daily.CurrentDayStreak;
		LoadArchive << Daily.BestDayStreak;
		LoadArchive << LastLoginTicks;
		LoadArchive << Daily.TotalDaysPlayed;

		Daily.LastLoginDate = FDateTime(LastLoginTicks);

		DailyStreaks.Add(PlayerId, Daily);
	}

	UE_LOG(LogTemp, Log, TEXT("MGStreakSubsystem: Loaded streak data for %d players"), NumPlayers);
}

// Copyright Midnight Grind. All Rights Reserved.

#include "Bonus/MGBonusSubsystem.h"
#include "TimerManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

void UMGBonusSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ActiveIdCounter = 0;
	EventCounter = 0;

	// Set up default config
	BonusConfig.GlobalMultiplier = 1.0f;
	BonusConfig.RespawnTimeMultiplier = 1.0f;
	BonusConfig.DurationMultiplier = 1.0f;
	BonusConfig.ComboThresholdForBonus = 10.0f;
	BonusConfig.ComboBonusPointsPerLevel = 1000.0f;
	BonusConfig.bEnableSecretBonuses = true;
	BonusConfig.bEnableBonusRounds = true;

	// Rarity spawn weights
	BonusConfig.RaritySpawnWeights.Add(EMGBonusRarity::Common, 50.0f);
	BonusConfig.RaritySpawnWeights.Add(EMGBonusRarity::Uncommon, 30.0f);
	BonusConfig.RaritySpawnWeights.Add(EMGBonusRarity::Rare, 15.0f);
	BonusConfig.RaritySpawnWeights.Add(EMGBonusRarity::Epic, 4.0f);
	BonusConfig.RaritySpawnWeights.Add(EMGBonusRarity::Legendary, 0.9f);
	BonusConfig.RaritySpawnWeights.Add(EMGBonusRarity::Mythic, 0.1f);

	// Register default bonus definitions
	FMGBonusDefinition PointBonus;
	PointBonus.BonusId = TEXT("PointBonus100");
	PointBonus.DisplayName = FText::FromString(TEXT("Point Bonus"));
	PointBonus.Description = FText::FromString(TEXT("Instant point bonus"));
	PointBonus.Type = EMGBonusType::PointBonus;
	PointBonus.Rarity = EMGBonusRarity::Common;
	PointBonus.PointValue = 100;
	RegisterBonusDefinition(PointBonus);

	FMGBonusDefinition DoublePoints;
	DoublePoints.BonusId = TEXT("DoublePoints");
	DoublePoints.DisplayName = FText::FromString(TEXT("Double Points"));
	DoublePoints.Description = FText::FromString(TEXT("Double all points for a short time"));
	DoublePoints.Type = EMGBonusType::DoublePoints;
	DoublePoints.Rarity = EMGBonusRarity::Uncommon;
	DoublePoints.Duration = 15.0f;
	DoublePoints.Multiplier = 2.0f;
	RegisterBonusDefinition(DoublePoints);

	FMGBonusDefinition TriplePoints;
	TriplePoints.BonusId = TEXT("TriplePoints");
	TriplePoints.DisplayName = FText::FromString(TEXT("Triple Points"));
	TriplePoints.Description = FText::FromString(TEXT("Triple all points for a short time"));
	TriplePoints.Type = EMGBonusType::TriplePoints;
	TriplePoints.Rarity = EMGBonusRarity::Rare;
	TriplePoints.Duration = 10.0f;
	TriplePoints.Multiplier = 3.0f;
	RegisterBonusDefinition(TriplePoints);

	FMGBonusDefinition NitroRefill;
	NitroRefill.BonusId = TEXT("NitroRefill");
	NitroRefill.DisplayName = FText::FromString(TEXT("Nitro Refill"));
	NitroRefill.Description = FText::FromString(TEXT("Instantly refill nitro"));
	NitroRefill.Type = EMGBonusType::NitroRefill;
	NitroRefill.Rarity = EMGBonusRarity::Common;
	NitroRefill.Value = 100.0f;
	RegisterBonusDefinition(NitroRefill);

	FMGBonusDefinition SpeedBoost;
	SpeedBoost.BonusId = TEXT("SpeedBoost");
	SpeedBoost.DisplayName = FText::FromString(TEXT("Speed Boost"));
	SpeedBoost.Description = FText::FromString(TEXT("Temporary speed increase"));
	SpeedBoost.Type = EMGBonusType::SpeedBoost;
	SpeedBoost.Rarity = EMGBonusRarity::Uncommon;
	SpeedBoost.Duration = 5.0f;
	SpeedBoost.Value = 20.0f;
	RegisterBonusDefinition(SpeedBoost);

	FMGBonusDefinition Invincibility;
	Invincibility.BonusId = TEXT("Invincibility");
	Invincibility.DisplayName = FText::FromString(TEXT("Invincibility"));
	Invincibility.Description = FText::FromString(TEXT("Temporary invincibility"));
	Invincibility.Type = EMGBonusType::Invincibility;
	Invincibility.Rarity = EMGBonusRarity::Epic;
	Invincibility.Duration = 8.0f;
	RegisterBonusDefinition(Invincibility);

	FMGBonusDefinition ComboExtender;
	ComboExtender.BonusId = TEXT("ComboExtender");
	ComboExtender.DisplayName = FText::FromString(TEXT("Combo Extender"));
	ComboExtender.Description = FText::FromString(TEXT("Extend combo timer"));
	ComboExtender.Type = EMGBonusType::ComboExtender;
	ComboExtender.Rarity = EMGBonusRarity::Uncommon;
	ComboExtender.Value = 5.0f;
	RegisterBonusDefinition(ComboExtender);

	// Start tick timer
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGBonusSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(
			BonusTickTimer,
			[WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->TickBonus(0.033f);
				}
			},
			0.033f,
			true
		);
	}

	LoadBonusData();
}

void UMGBonusSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BonusTickTimer);
	}

	SaveBonusData();
	Super::Deinitialize();
}

// ============================================================================
// Definition Registration
// ============================================================================

void UMGBonusSubsystem::RegisterBonusDefinition(const FMGBonusDefinition& Definition)
{
	if (Definition.BonusId.IsEmpty())
	{
		return;
	}
	Definitions.Add(Definition.BonusId, Definition);
}

FMGBonusDefinition UMGBonusSubsystem::GetBonusDefinition(const FString& BonusId) const
{
	if (const FMGBonusDefinition* Found = Definitions.Find(BonusId))
	{
		return *Found;
	}
	return FMGBonusDefinition();
}

TArray<FMGBonusDefinition> UMGBonusSubsystem::GetAllDefinitions() const
{
	TArray<FMGBonusDefinition> Result;
	for (const auto& Pair : Definitions)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

TArray<FMGBonusDefinition> UMGBonusSubsystem::GetDefinitionsByType(EMGBonusType Type) const
{
	TArray<FMGBonusDefinition> Result;
	for (const auto& Pair : Definitions)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

TArray<FMGBonusDefinition> UMGBonusSubsystem::GetDefinitionsByRarity(EMGBonusRarity Rarity) const
{
	TArray<FMGBonusDefinition> Result;
	for (const auto& Pair : Definitions)
	{
		if (Pair.Value.Rarity == Rarity)
		{
			Result.Add(Pair.Value);
		}
	}
	return Result;
}

// ============================================================================
// Spawn Points
// ============================================================================

void UMGBonusSubsystem::RegisterSpawnPoint(const FMGBonusSpawnPoint& SpawnPoint)
{
	if (SpawnPoint.SpawnId.IsEmpty())
	{
		return;
	}
	SpawnPoints.Add(SpawnPoint.SpawnId, SpawnPoint);
}

void UMGBonusSubsystem::UnregisterSpawnPoint(const FString& SpawnId)
{
	SpawnPoints.Remove(SpawnId);
}

FMGBonusSpawnPoint UMGBonusSubsystem::GetSpawnPoint(const FString& SpawnId) const
{
	if (const FMGBonusSpawnPoint* Found = SpawnPoints.Find(SpawnId))
	{
		return *Found;
	}
	return FMGBonusSpawnPoint();
}

TArray<FMGBonusSpawnPoint> UMGBonusSubsystem::GetAllSpawnPoints() const
{
	TArray<FMGBonusSpawnPoint> Result;
	for (const auto& Pair : SpawnPoints)
	{
		Result.Add(Pair.Value);
	}
	return Result;
}

void UMGBonusSubsystem::SpawnBonus(const FString& SpawnId)
{
	FMGBonusSpawnPoint* SpawnPoint = SpawnPoints.Find(SpawnId);
	if (!SpawnPoint)
	{
		return;
	}

	// Select bonus to spawn
	FString BonusId;
	if (SpawnPoint->PossibleBonusIds.Num() > 0)
	{
		BonusId = SelectRandomBonus(SpawnPoint->PossibleBonusIds);
	}
	else if (!SpawnPoint->AssignedBonusId.IsEmpty())
	{
		BonusId = SpawnPoint->AssignedBonusId;
	}

	if (BonusId.IsEmpty())
	{
		return;
	}

	SpawnPoint->AssignedBonusId = BonusId;
	SpawnPoint->bIsActive = true;
	SpawnPoint->bIsCollected = false;

	OnBonusSpawned.Broadcast(SpawnId, BonusId);
}

void UMGBonusSubsystem::SpawnAllBonuses()
{
	for (auto& Pair : SpawnPoints)
	{
		if (!Pair.Value.bIsActive && !Pair.Value.bIsCollected)
		{
			SpawnBonus(Pair.Key);
		}
	}
}

void UMGBonusSubsystem::RespawnBonus(const FString& SpawnId)
{
	FMGBonusSpawnPoint* SpawnPoint = SpawnPoints.Find(SpawnId);
	if (!SpawnPoint)
	{
		return;
	}

	const FMGBonusDefinition* Def = Definitions.Find(SpawnPoint->AssignedBonusId);
	float RespawnTime = Def ? Def->RespawnTime : 30.0f;
	RespawnTime *= BonusConfig.RespawnTimeMultiplier;

	SpawnPoint->RespawnTimer = RespawnTime;
	SpawnPoint->bIsCollected = false;
	SpawnPoint->bIsActive = false;

	OnBonusRespawned.Broadcast(SpawnId, RespawnTime);
}

// ============================================================================
// Collection
// ============================================================================

FMGActiveBonus UMGBonusSubsystem::CollectBonus(const FString& PlayerId, const FString& SpawnId)
{
	FMGActiveBonus ActiveBonus;

	FMGBonusSpawnPoint* SpawnPoint = SpawnPoints.Find(SpawnId);
	if (!SpawnPoint || !SpawnPoint->bIsActive || SpawnPoint->bIsCollected)
	{
		return ActiveBonus;
	}

	const FMGBonusDefinition* Def = Definitions.Find(SpawnPoint->AssignedBonusId);
	if (!Def)
	{
		return ActiveBonus;
	}

	// Mark as collected
	SpawnPoint->bIsCollected = true;
	SpawnPoint->bIsActive = false;

	// Calculate points
	int32 PointsAwarded = Def->PointValue;
	PointsAwarded = FMath::RoundToInt(PointsAwarded * BonusConfig.GlobalMultiplier);

	// Create active bonus
	ActiveBonus.ActiveId = GenerateActiveId();
	ActiveBonus.BonusId = Def->BonusId;
	ActiveBonus.PlayerId = PlayerId;
	ActiveBonus.Type = Def->Type;
	ActiveBonus.Value = Def->Value;
	ActiveBonus.Multiplier = Def->Multiplier;
	ActiveBonus.TimeRemaining = Def->Duration * BonusConfig.DurationMultiplier;
	ActiveBonus.TotalDuration = ActiveBonus.TimeRemaining;
	ActiveBonus.ActivatedAt = FDateTime::Now();

	// Apply bonus effect
	ApplyBonusEffect(PlayerId, *Def);

	// Add to active bonuses if duration > 0
	if (ActiveBonus.TimeRemaining > 0.0f)
	{
		TArray<FMGActiveBonus>& PlayerBonuses = PlayerActiveBonuses.FindOrAdd(PlayerId);
		PlayerBonuses.Add(ActiveBonus);

		OnBonusActivated.Broadcast(PlayerId, ActiveBonus, ActiveBonus.TimeRemaining);
	}

	// Update stats
	UpdatePlayerStats(PlayerId, *Def, PointsAwarded);

	// Schedule respawn
	RespawnBonus(SpawnId);

	OnBonusCollected.Broadcast(PlayerId, *Def, PointsAwarded);

	return ActiveBonus;
}

void UMGBonusSubsystem::GrantBonus(const FString& PlayerId, const FString& BonusId)
{
	const FMGBonusDefinition* Def = Definitions.Find(BonusId);
	if (!Def)
	{
		return;
	}

	int32 PointsAwarded = Def->PointValue;

	FMGActiveBonus ActiveBonus;
	ActiveBonus.ActiveId = GenerateActiveId();
	ActiveBonus.BonusId = BonusId;
	ActiveBonus.PlayerId = PlayerId;
	ActiveBonus.Type = Def->Type;
	ActiveBonus.Value = Def->Value;
	ActiveBonus.Multiplier = Def->Multiplier;
	ActiveBonus.TimeRemaining = Def->Duration * BonusConfig.DurationMultiplier;
	ActiveBonus.TotalDuration = ActiveBonus.TimeRemaining;
	ActiveBonus.ActivatedAt = FDateTime::Now();

	ApplyBonusEffect(PlayerId, *Def);

	if (ActiveBonus.TimeRemaining > 0.0f)
	{
		TArray<FMGActiveBonus>& PlayerBonuses = PlayerActiveBonuses.FindOrAdd(PlayerId);

		// Check for stacking
		if (Def->bStackable)
		{
			bool bFound = false;
			for (FMGActiveBonus& Existing : PlayerBonuses)
			{
				if (Existing.BonusId == BonusId)
				{
					Existing.CurrentStacks = FMath::Min(Existing.CurrentStacks + 1, Def->MaxStacks);
					Existing.TimeRemaining = ActiveBonus.TimeRemaining;
					bFound = true;
					OnBonusStacked.Broadcast(PlayerId, BonusId, Existing.CurrentStacks);
					break;
				}
			}
			if (!bFound)
			{
				PlayerBonuses.Add(ActiveBonus);
			}
		}
		else
		{
			PlayerBonuses.Add(ActiveBonus);
		}

		OnBonusActivated.Broadcast(PlayerId, ActiveBonus, ActiveBonus.TimeRemaining);
	}

	UpdatePlayerStats(PlayerId, *Def, PointsAwarded);
	OnBonusCollected.Broadcast(PlayerId, *Def, PointsAwarded);
}

bool UMGBonusSubsystem::TryCollectAtLocation(const FString& PlayerId, FVector Location)
{
	FString NearestId = GetNearestBonusSpawnId(Location, 500.0f);
	if (NearestId.IsEmpty())
	{
		return false;
	}

	FMGBonusSpawnPoint* SpawnPoint = SpawnPoints.Find(NearestId);
	if (!SpawnPoint || !SpawnPoint->bIsActive)
	{
		return false;
	}

	float Distance = FVector::Dist(Location, SpawnPoint->Location);
	if (Distance <= SpawnPoint->CollectionRadius)
	{
		CollectBonus(PlayerId, NearestId);
		return true;
	}

	return false;
}

FString UMGBonusSubsystem::GetNearestBonusSpawnId(FVector Location, float MaxDistance) const
{
	FString NearestId;
	float NearestDist = MaxDistance;

	for (const auto& Pair : SpawnPoints)
	{
		if (!Pair.Value.bIsActive)
		{
			continue;
		}

		float Dist = FVector::Dist(Location, Pair.Value.Location);
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			NearestId = Pair.Key;
		}
	}

	return NearestId;
}

// ============================================================================
// Active Bonuses
// ============================================================================

TArray<FMGActiveBonus> UMGBonusSubsystem::GetActiveBonuses(const FString& PlayerId) const
{
	if (const TArray<FMGActiveBonus>* Found = PlayerActiveBonuses.Find(PlayerId))
	{
		return *Found;
	}
	return TArray<FMGActiveBonus>();
}

FMGActiveBonus UMGBonusSubsystem::GetActiveBonus(const FString& PlayerId, const FString& BonusId) const
{
	if (const TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (const FMGActiveBonus& Bonus : *Bonuses)
		{
			if (Bonus.BonusId == BonusId)
			{
				return Bonus;
			}
		}
	}
	return FMGActiveBonus();
}

bool UMGBonusSubsystem::HasActiveBonus(const FString& PlayerId, EMGBonusType Type) const
{
	if (const TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (const FMGActiveBonus& Bonus : *Bonuses)
		{
			if (Bonus.Type == Type && Bonus.TimeRemaining > 0.0f && !Bonus.bIsPaused)
			{
				return true;
			}
		}
	}
	return false;
}

void UMGBonusSubsystem::PauseBonus(const FString& PlayerId, const FString& ActiveId)
{
	if (TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (FMGActiveBonus& Bonus : *Bonuses)
		{
			if (Bonus.ActiveId == ActiveId)
			{
				Bonus.bIsPaused = true;
				break;
			}
		}
	}
}

void UMGBonusSubsystem::ResumeBonus(const FString& PlayerId, const FString& ActiveId)
{
	if (TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (FMGActiveBonus& Bonus : *Bonuses)
		{
			if (Bonus.ActiveId == ActiveId)
			{
				Bonus.bIsPaused = false;
				break;
			}
		}
	}
}

void UMGBonusSubsystem::CancelBonus(const FString& PlayerId, const FString& ActiveId)
{
	if (TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (int32 i = Bonuses->Num() - 1; i >= 0; i--)
		{
			if ((*Bonuses)[i].ActiveId == ActiveId)
			{
				RemoveBonusEffect(PlayerId, (*Bonuses)[i]);
				OnBonusExpired.Broadcast(PlayerId, (*Bonuses)[i].BonusId, (*Bonuses)[i].Value);
				Bonuses->RemoveAt(i);
				break;
			}
		}
	}
}

void UMGBonusSubsystem::ClearAllBonuses(const FString& PlayerId)
{
	if (TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (const FMGActiveBonus& Bonus : *Bonuses)
		{
			RemoveBonusEffect(PlayerId, Bonus);
			OnBonusExpired.Broadcast(PlayerId, Bonus.BonusId, Bonus.Value);
		}
		Bonuses->Empty();
	}
}

// ============================================================================
// Multipliers
// ============================================================================

float UMGBonusSubsystem::GetTotalMultiplier(const FString& PlayerId) const
{
	float Multiplier = BonusConfig.GlobalMultiplier;

	if (const TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (const FMGActiveBonus& Bonus : *Bonuses)
		{
			if (!Bonus.bIsPaused && Bonus.TimeRemaining > 0.0f)
			{
				if (Bonus.Type == EMGBonusType::ScoreMultiplier ||
				    Bonus.Type == EMGBonusType::DoublePoints ||
				    Bonus.Type == EMGBonusType::TriplePoints)
				{
					Multiplier *= Bonus.Multiplier;
				}
			}
		}
	}

	return Multiplier;
}

float UMGBonusSubsystem::GetScoreMultiplier(const FString& PlayerId) const
{
	return GetTotalMultiplier(PlayerId);
}

float UMGBonusSubsystem::GetXPMultiplier(const FString& PlayerId) const
{
	float Multiplier = 1.0f;

	if (const TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (const FMGActiveBonus& Bonus : *Bonuses)
		{
			if (!Bonus.bIsPaused && Bonus.TimeRemaining > 0.0f && Bonus.Type == EMGBonusType::XPBonus)
			{
				Multiplier *= Bonus.Multiplier;
			}
		}
	}

	return Multiplier;
}

float UMGBonusSubsystem::GetCashMultiplier(const FString& PlayerId) const
{
	float Multiplier = 1.0f;

	if (const TArray<FMGActiveBonus>* Bonuses = PlayerActiveBonuses.Find(PlayerId))
	{
		for (const FMGActiveBonus& Bonus : *Bonuses)
		{
			if (!Bonus.bIsPaused && Bonus.TimeRemaining > 0.0f && Bonus.Type == EMGBonusType::CashBonus)
			{
				Multiplier *= Bonus.Multiplier;
			}
		}
	}

	return Multiplier;
}

// ============================================================================
// Bonus Rounds
// ============================================================================

void UMGBonusSubsystem::RegisterBonusRound(const FMGBonusRound& Round)
{
	if (Round.RoundId.IsEmpty())
	{
		return;
	}
	BonusRounds.Add(Round.RoundId, Round);
}

FMGBonusRound UMGBonusSubsystem::GetBonusRound(const FString& RoundId) const
{
	if (const FMGBonusRound* Found = BonusRounds.Find(RoundId))
	{
		return *Found;
	}
	return FMGBonusRound();
}

void UMGBonusSubsystem::StartBonusRound(const FString& PlayerId, const FString& RoundId)
{
	if (!BonusConfig.bEnableBonusRounds)
	{
		return;
	}

	const FMGBonusRound* Round = BonusRounds.Find(RoundId);
	if (!Round)
	{
		return;
	}

	FMGActiveBonusRound ActiveRound;
	ActiveRound.RoundId = RoundId;
	ActiveRound.PlayerId = PlayerId;
	ActiveRound.Type = Round->Type;
	ActiveRound.bIsActive = true;
	ActiveRound.TimeRemaining = Round->Duration;
	ActiveRound.TargetScore = Round->TargetScore;
	ActiveRound.TotalItems = Round->SpawnPointIds.Num();

	ActiveBonusRounds.Add(PlayerId, ActiveRound);

	OnBonusRoundStart.Broadcast(PlayerId, *Round);
}

void UMGBonusSubsystem::UpdateBonusRound(const FString& PlayerId, int32 ScoreGained)
{
	FMGActiveBonusRound* Active = ActiveBonusRounds.Find(PlayerId);
	if (!Active || !Active->bIsActive)
	{
		return;
	}

	const FMGBonusRound* Round = BonusRounds.Find(Active->RoundId);
	if (!Round)
	{
		return;
	}

	Active->CurrentScore += FMath::RoundToInt(ScoreGained * Round->PointMultiplier);

	if (Active->CurrentScore >= Active->TargetScore)
	{
		Active->bCompleted = true;
		EndBonusRound(PlayerId);
	}
}

void UMGBonusSubsystem::EndBonusRound(const FString& PlayerId)
{
	FMGActiveBonusRound* Active = ActiveBonusRounds.Find(PlayerId);
	if (!Active)
	{
		return;
	}

	Active->bIsActive = false;

	const FMGBonusRound* Round = BonusRounds.Find(Active->RoundId);

	if (Active->bCompleted)
	{
		int32 FinalScore = Active->CurrentScore;
		if (Round)
		{
			FinalScore += Round->CompletionBonus;
		}

		FMGBonusPlayerStats& Stats = PlayerStats.FindOrAdd(PlayerId);
		Stats.BonusRoundsCompleted++;

		if (Round && Active->CurrentScore >= Round->GoldThreshold)
		{
			Stats.BonusRoundsGold++;
		}

		OnBonusRoundComplete.Broadcast(PlayerId, Active->RoundId, FinalScore);
	}
	else if (Active->bFailed || Active->TimeRemaining <= 0.0f)
	{
		Active->bFailed = true;
		OnBonusRoundFailed.Broadcast(PlayerId, Active->RoundId);
	}

	ActiveBonusRounds.Remove(PlayerId);
}

FMGActiveBonusRound UMGBonusSubsystem::GetActiveBonusRound(const FString& PlayerId) const
{
	if (const FMGActiveBonusRound* Found = ActiveBonusRounds.Find(PlayerId))
	{
		return *Found;
	}
	return FMGActiveBonusRound();
}

bool UMGBonusSubsystem::IsInBonusRound(const FString& PlayerId) const
{
	if (const FMGActiveBonusRound* Found = ActiveBonusRounds.Find(PlayerId))
	{
		return Found->bIsActive;
	}
	return false;
}

// ============================================================================
// Combo Bonuses
// ============================================================================

void UMGBonusSubsystem::ProcessComboBonus(const FString& PlayerId, int32 ComboCount, float ComboMultiplier)
{
	int32 ComboLevel = ComboCount / static_cast<int32>(BonusConfig.ComboThresholdForBonus);

	if (ComboLevel > 0 && ComboCount % static_cast<int32>(BonusConfig.ComboThresholdForBonus) == 0)
	{
		int32 BonusPoints = FMath::RoundToInt(ComboLevel * BonusConfig.ComboBonusPointsPerLevel * ComboMultiplier);

		FMGBonusPlayerStats& Stats = PlayerStats.FindOrAdd(PlayerId);
		Stats.TotalPointsFromBonuses += BonusPoints;

		if (ComboLevel > Stats.MaxComboBonus)
		{
			Stats.MaxComboBonus = ComboLevel;
		}

		OnComboBonusTriggered.Broadcast(PlayerId, ComboLevel, BonusPoints);
	}
}

int32 UMGBonusSubsystem::GetComboBonusPoints(int32 ComboLevel) const
{
	return FMath::RoundToInt(ComboLevel * BonusConfig.ComboBonusPointsPerLevel);
}

// ============================================================================
// Secret Bonuses
// ============================================================================

void UMGBonusSubsystem::RegisterSecretBonus(const FString& SecretId, const FString& BonusId, FVector Location)
{
	if (!BonusConfig.bEnableSecretBonuses)
	{
		return;
	}

	SecretBonuses.Add(SecretId, BonusId);
	SecretLocations.Add(SecretId, Location);
}

bool UMGBonusSubsystem::TryDiscoverSecret(const FString& PlayerId, FVector Location)
{
	if (!BonusConfig.bEnableSecretBonuses)
	{
		return false;
	}

	for (const auto& Pair : SecretLocations)
	{
		if (DiscoveredSecrets.Contains(Pair.Key))
		{
			continue;
		}

		float Distance = FVector::Dist(Location, Pair.Value);
		if (Distance < 200.0f)
		{
			DiscoveredSecrets.Add(Pair.Key);

			const FString* BonusId = SecretBonuses.Find(Pair.Key);
			if (BonusId)
			{
				GrantBonus(PlayerId, *BonusId);
			}

			FMGBonusPlayerStats& Stats = PlayerStats.FindOrAdd(PlayerId);
			Stats.SecretBonusesFound++;

			OnSecretBonusFound.Broadcast(PlayerId, Pair.Key);
			return true;
		}
	}

	return false;
}

bool UMGBonusSubsystem::IsSecretDiscovered(const FString& SecretId) const
{
	return DiscoveredSecrets.Contains(SecretId);
}

TArray<FString> UMGBonusSubsystem::GetDiscoveredSecrets(const FString& PlayerId) const
{
	return DiscoveredSecrets;
}

// ============================================================================
// Stats
// ============================================================================

FMGBonusPlayerStats UMGBonusSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGBonusPlayerStats* Found = PlayerStats.Find(PlayerId))
	{
		return *Found;
	}
	return FMGBonusPlayerStats();
}

void UMGBonusSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGBonusPlayerStats Stats;
	Stats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, Stats);
}

// ============================================================================
// Configuration
// ============================================================================

void UMGBonusSubsystem::SetBonusConfig(const FMGBonusConfig& Config)
{
	BonusConfig = Config;
}

FMGBonusConfig UMGBonusSubsystem::GetBonusConfig() const
{
	return BonusConfig;
}

// ============================================================================
// Update
// ============================================================================

void UMGBonusSubsystem::UpdateBonusSystem(float DeltaTime)
{
	UpdateActiveBonuses(DeltaTime);
	UpdateSpawnRespawns(DeltaTime);
	UpdateBonusRounds(DeltaTime);
}

// ============================================================================
// Save/Load
// ============================================================================

void UMGBonusSubsystem::SaveBonusData()
{
	FString DataDir = FPaths::ProjectSavedDir() / TEXT("Bonus");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*DataDir))
	{
		PlatformFile.CreateDirectory(*DataDir);
	}

	FString FilePath = DataDir / TEXT("bonus_stats.dat");

	FBufferArchive Archive;

	// Write version
	int32 Version = 1;
	Archive << Version;

	// Write player stats
	int32 PlayerCount = PlayerStats.Num();
	Archive << PlayerCount;

	for (const auto& Pair : PlayerStats)
	{
		FString PlayerId = Pair.Key;
		const FMGBonusPlayerStats& Stats = Pair.Value;

		Archive << PlayerId;
		Archive << Stats.TotalBonusesCollected;
		Archive << Stats.TotalPointsFromBonuses;
		Archive << Stats.BestMultiplier;
		Archive << Stats.LongestChain;
		Archive << Stats.SecretBonusesFound;
		Archive << Stats.TotalBonusRoundsCompleted;
		Archive << Stats.BestBonusRoundScore;

		// Write rarity counts
		int32 RarityCount = Stats.RaritiesCollected.Num();
		Archive << RarityCount;
		for (const auto& RarityPair : Stats.RaritiesCollected)
		{
			int32 RarityInt = static_cast<int32>(RarityPair.Key);
			int32 Count = RarityPair.Value;
			Archive << RarityInt;
			Archive << Count;
		}

		// Write type counts
		int32 TypeCount = Stats.TypesCollected.Num();
		Archive << TypeCount;
		for (const auto& TypePair : Stats.TypesCollected)
		{
			int32 TypeInt = static_cast<int32>(TypePair.Key);
			int32 Count = TypePair.Value;
			Archive << TypeInt;
			Archive << Count;
		}
	}

	FFileHelper::SaveArrayToFile(Archive, *FilePath);
	Archive.FlushCache();
	Archive.Empty();

	UE_LOG(LogTemp, Log, TEXT("MGBonus: Saved bonus stats for %d players"), PlayerCount);
}

void UMGBonusSubsystem::LoadBonusData()
{
	FString DataDir = FPaths::ProjectSavedDir() / TEXT("Bonus");
	FString FilePath = DataDir / TEXT("bonus_stats.dat");

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return;
	}

	FMemoryReader Archive(FileData, true);

	// Read version
	int32 Version;
	Archive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGBonus: Unknown save version %d"), Version);
		return;
	}

	// Read player stats
	int32 PlayerCount;
	Archive << PlayerCount;

	for (int32 i = 0; i < PlayerCount; i++)
	{
		FString PlayerId;
		FMGBonusPlayerStats Stats;

		Archive << PlayerId;
		Archive << Stats.TotalBonusesCollected;
		Archive << Stats.TotalPointsFromBonuses;
		Archive << Stats.BestMultiplier;
		Archive << Stats.LongestChain;
		Archive << Stats.SecretBonusesFound;
		Archive << Stats.TotalBonusRoundsCompleted;
		Archive << Stats.BestBonusRoundScore;

		// Read rarity counts
		int32 RarityCount;
		Archive << RarityCount;
		for (int32 j = 0; j < RarityCount; j++)
		{
			int32 RarityInt;
			int32 Count;
			Archive << RarityInt;
			Archive << Count;
			Stats.RaritiesCollected.Add(static_cast<EMGBonusRarity>(RarityInt), Count);
		}

		// Read type counts
		int32 TypeCount;
		Archive << TypeCount;
		for (int32 j = 0; j < TypeCount; j++)
		{
			int32 TypeInt;
			int32 Count;
			Archive << TypeInt;
			Archive << Count;
			Stats.TypesCollected.Add(static_cast<EMGBonusType>(TypeInt), Count);
		}

		Stats.PlayerId = PlayerId;
		PlayerStats.Add(PlayerId, Stats);
	}

	UE_LOG(LogTemp, Log, TEXT("MGBonus: Loaded bonus stats for %d players"), PlayerCount);
}

// ============================================================================
// Protected Methods
// ============================================================================

void UMGBonusSubsystem::TickBonus(float DeltaTime)
{
	UpdateBonusSystem(DeltaTime);
}

void UMGBonusSubsystem::UpdateActiveBonuses(float DeltaTime)
{
	for (auto& PlayerPair : PlayerActiveBonuses)
	{
		TArray<int32> ExpiredIndices;

		for (int32 i = 0; i < PlayerPair.Value.Num(); i++)
		{
			FMGActiveBonus& Bonus = PlayerPair.Value[i];

			if (!Bonus.bIsPaused && Bonus.TimeRemaining > 0.0f)
			{
				Bonus.TimeRemaining -= DeltaTime;

				if (Bonus.TimeRemaining <= 0.0f)
				{
					ExpiredIndices.Add(i);
				}
			}
		}

		// Remove expired bonuses (reverse order to preserve indices)
		for (int32 i = ExpiredIndices.Num() - 1; i >= 0; i--)
		{
			int32 Index = ExpiredIndices[i];
			FMGActiveBonus& Bonus = PlayerPair.Value[Index];

			RemoveBonusEffect(PlayerPair.Key, Bonus);
			OnBonusExpired.Broadcast(PlayerPair.Key, Bonus.BonusId, Bonus.Value);

			PlayerPair.Value.RemoveAt(Index);
		}
	}
}

void UMGBonusSubsystem::UpdateSpawnRespawns(float DeltaTime)
{
	for (auto& Pair : SpawnPoints)
	{
		FMGBonusSpawnPoint& SpawnPoint = Pair.Value;

		if (SpawnPoint.bIsCollected && SpawnPoint.RespawnTimer > 0.0f)
		{
			SpawnPoint.RespawnTimer -= DeltaTime;

			if (SpawnPoint.RespawnTimer <= 0.0f)
			{
				SpawnBonus(Pair.Key);
			}
		}
	}
}

void UMGBonusSubsystem::UpdateBonusRounds(float DeltaTime)
{
	TArray<FString> FailedRounds;

	for (auto& Pair : ActiveBonusRounds)
	{
		FMGActiveBonusRound& Round = Pair.Value;

		if (Round.bIsActive)
		{
			Round.TimeRemaining -= DeltaTime;

			if (Round.TimeRemaining <= 0.0f && !Round.bCompleted)
			{
				Round.bFailed = true;
				FailedRounds.Add(Pair.Key);
			}
		}
	}

	for (const FString& PlayerId : FailedRounds)
	{
		EndBonusRound(PlayerId);
	}
}

void UMGBonusSubsystem::ApplyBonusEffect(const FString& PlayerId, const FMGBonusDefinition& Bonus)
{
	// Effects are applied through other game systems
	// This is a central place to trigger them
	switch (Bonus.Type)
	{
		case EMGBonusType::DoublePoints:
		case EMGBonusType::TriplePoints:
		case EMGBonusType::ScoreMultiplier:
			OnMultiplierChanged.Broadcast(PlayerId, GetTotalMultiplier(PlayerId));
			break;
		default:
			break;
	}
}

void UMGBonusSubsystem::RemoveBonusEffect(const FString& PlayerId, const FMGActiveBonus& Bonus)
{
	switch (Bonus.Type)
	{
		case EMGBonusType::DoublePoints:
		case EMGBonusType::TriplePoints:
		case EMGBonusType::ScoreMultiplier:
			OnMultiplierChanged.Broadcast(PlayerId, GetTotalMultiplier(PlayerId));
			break;
		default:
			break;
	}
}

void UMGBonusSubsystem::UpdatePlayerStats(const FString& PlayerId, const FMGBonusDefinition& Bonus, int32 Points)
{
	FMGBonusPlayerStats& Stats = PlayerStats.FindOrAdd(PlayerId);
	Stats.PlayerId = PlayerId;
	Stats.TotalBonusesCollected++;
	Stats.TotalPointsFromBonuses += Points;

	int32& TypeCount = Stats.BonusesCollected.FindOrAdd(Bonus.Type);
	TypeCount++;

	int32& RarityCount = Stats.RaritiesCollected.FindOrAdd(Bonus.Rarity);
	RarityCount++;

	if (Bonus.Duration > Stats.LongestMultiplierDuration)
	{
		Stats.LongestMultiplierDuration = Bonus.Duration;
	}

	if (Bonus.Multiplier > Stats.HighestMultiplierValue)
	{
		Stats.HighestMultiplierValue = Bonus.Multiplier;
	}
}

FString UMGBonusSubsystem::SelectRandomBonus(const TArray<FString>& PossibleIds) const
{
	if (PossibleIds.Num() == 0)
	{
		return FString();
	}

	// Calculate total weight based on rarity
	float TotalWeight = 0.0f;
	TArray<float> Weights;

	for (const FString& Id : PossibleIds)
	{
		const FMGBonusDefinition* Def = Definitions.Find(Id);
		float Weight = 1.0f;

		if (Def)
		{
			if (const float* RarityWeight = BonusConfig.RaritySpawnWeights.Find(Def->Rarity))
			{
				Weight = *RarityWeight;
			}
			Weight *= Def->SpawnChance;
		}

		Weights.Add(Weight);
		TotalWeight += Weight;
	}

	// Select based on weight
	float Random = FMath::FRand() * TotalWeight;
	float Accumulated = 0.0f;

	for (int32 i = 0; i < PossibleIds.Num(); i++)
	{
		Accumulated += Weights[i];
		if (Random <= Accumulated)
		{
			return PossibleIds[i];
		}
	}

	return PossibleIds.Last();
}

FString UMGBonusSubsystem::GenerateActiveId() const
{
	return FString::Printf(TEXT("ACT_%d_%lld"), ++const_cast<UMGBonusSubsystem*>(this)->ActiveIdCounter,
	                       FDateTime::Now().GetTicks());
}

FString UMGBonusSubsystem::GenerateEventId() const
{
	return FString::Printf(TEXT("BEVT_%d_%lld"), ++const_cast<UMGBonusSubsystem*>(this)->EventCounter,
	                       FDateTime::Now().GetTicks());
}

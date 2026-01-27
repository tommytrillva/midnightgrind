// Copyright Epic Games, Inc. All Rights Reserved.

#include "Showdown/MGShowdownSubsystem.h"
#include "Career/MGCareerSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void UMGShowdownSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Register default bosses
	{
		FMGBossEncounter Shadow;
		Shadow.BossId = TEXT("BOSS_SHADOW_KING");
		Shadow.BossName = FText::FromString(TEXT("Shadow King"));
		Shadow.Title = FText::FromString(TEXT("Ruler of the Night"));
		Shadow.Backstory = FText::FromString(TEXT("The original midnight racer. No one has ever seen his face. They say he sold his soul for speed."));
		Shadow.VehicleId = TEXT("VEHICLE_SHADOW_PHANTOM");
		Shadow.BaseSkillLevel = 95;
		Shadow.AggressionFactor = 0.8f;
		Shadow.SpecialAbilities.Add(TEXT("NITRO_BURST"));
		Shadow.SpecialAbilities.Add(TEXT("SHADOW_CLONE"));
		Shadow.Weaknesses.Add(TEXT("VULNERABLE_IN_LIGHT"));
		Shadow.PhaseDialogue.Add(TEXT("INTRO"), FText::FromString(TEXT("You dare challenge me at my hour?")));
		Shadow.PhaseDialogue.Add(TEXT("PHASE2"), FText::FromString(TEXT("You're faster than I expected...")));
		Shadow.PhaseDialogue.Add(TEXT("FINALE"), FText::FromString(TEXT("This ends now!")));
		RegisterBoss(Shadow);
	}
	{
		FMGBossEncounter Storm;
		Storm.BossId = TEXT("BOSS_STORM_RIDER");
		Storm.BossName = FText::FromString(TEXT("Storm Rider"));
		Storm.Title = FText::FromString(TEXT("Master of the Highway"));
		Storm.Backstory = FText::FromString(TEXT("Conquered every highway in the city. Known for brutal takedowns in the rain."));
		Storm.VehicleId = TEXT("VEHICLE_STORM_CRUSHER");
		Storm.BaseSkillLevel = 88;
		Storm.AggressionFactor = 0.9f;
		Storm.SpecialAbilities.Add(TEXT("LIGHTNING_BOOST"));
		Storm.SpecialAbilities.Add(TEXT("RAIN_MASTERY"));
		Storm.Weaknesses.Add(TEXT("CLEAR_WEATHER"));
		RegisterBoss(Storm);
	}
	{
		FMGBossEncounter Phoenix;
		Phoenix.BossId = TEXT("BOSS_PHOENIX_LEGEND");
		Phoenix.BossName = FText::FromString(TEXT("Phoenix"));
		Phoenix.Title = FText::FromString(TEXT("The Undying Legend"));
		Phoenix.Backstory = FText::FromString(TEXT("Has never lost a race. Ever. They say defeating Phoenix is impossible."));
		Phoenix.VehicleId = TEXT("VEHICLE_PHOENIX_FIRE");
		Phoenix.BaseSkillLevel = 99;
		Phoenix.AggressionFactor = 0.75f;
		Phoenix.SpecialAbilities.Add(TEXT("RESURRECTION"));
		Phoenix.SpecialAbilities.Add(TEXT("FLAME_TRAIL"));
		Phoenix.SpecialAbilities.Add(TEXT("SPEED_OF_LIGHT"));
		RegisterBoss(Phoenix);
	}

	// Register default phases
	{
		FMGBossPhaseDefinition IntroPhase;
		IntroPhase.PhaseId = TEXT("PHASE_INTRO");
		IntroPhase.PhaseName = FText::FromString(TEXT("The Challenge"));
		IntroPhase.Type = EMGBossPhaseType::Introduction;
		IntroPhase.PhaseNumber = 1;
		IntroPhase.Objective = FText::FromString(TEXT("Catch up to the boss"));
		IntroPhase.Duration = 30.0f;
		IntroPhase.BossSpeedMultiplier = 0.8f;
		IntroPhase.BossAggressionLevel = 0.3f;
		RegisterPhase(IntroPhase);
	}
	{
		FMGBossPhaseDefinition ChasePhase;
		ChasePhase.PhaseId = TEXT("PHASE_CHASE");
		ChasePhase.PhaseName = FText::FromString(TEXT("The Chase"));
		ChasePhase.Type = EMGBossPhaseType::ChasePhase;
		ChasePhase.PhaseNumber = 2;
		ChasePhase.Objective = FText::FromString(TEXT("Stay close to the boss"));
		ChasePhase.BossSpeedMultiplier = 1.0f;
		ChasePhase.BossAggressionLevel = 0.5f;
		ChasePhase.CheckpointIndex = 1;
		RegisterPhase(ChasePhase);
	}
	{
		FMGBossPhaseDefinition RacePhase;
		RacePhase.PhaseId = TEXT("PHASE_RACE");
		RacePhase.PhaseName = FText::FromString(TEXT("The Race"));
		RacePhase.Type = EMGBossPhaseType::RacePhase;
		RacePhase.PhaseNumber = 3;
		RacePhase.Objective = FText::FromString(TEXT("Beat the boss to the finish"));
		RacePhase.BossSpeedMultiplier = 1.1f;
		RacePhase.BossAggressionLevel = 0.7f;
		RacePhase.CheckpointIndex = 2;
		RegisterPhase(RacePhase);
	}
	{
		FMGBossPhaseDefinition BattlePhase;
		BattlePhase.PhaseId = TEXT("PHASE_BATTLE");
		BattlePhase.PhaseName = FText::FromString(TEXT("The Battle"));
		BattlePhase.Type = EMGBossPhaseType::BattlePhase;
		BattlePhase.PhaseNumber = 4;
		BattlePhase.Objective = FText::FromString(TEXT("Take down the boss"));
		BattlePhase.BossHealthPercent = 100;
		BattlePhase.BossSpeedMultiplier = 1.0f;
		BattlePhase.BossAggressionLevel = 0.9f;
		BattlePhase.CheckpointIndex = 3;
		RegisterPhase(BattlePhase);
	}
	{
		FMGBossPhaseDefinition FinalPhase;
		FinalPhase.PhaseId = TEXT("PHASE_FINALE");
		FinalPhase.PhaseName = FText::FromString(TEXT("The Finale"));
		FinalPhase.Type = EMGBossPhaseType::FinalPhase;
		FinalPhase.PhaseNumber = 5;
		FinalPhase.Objective = FText::FromString(TEXT("Defeat the boss once and for all"));
		FinalPhase.BossHealthPercent = 50;
		FinalPhase.BossSpeedMultiplier = 1.2f;
		FinalPhase.BossAggressionLevel = 1.0f;
		FinalPhase.PhaseModifiers.Add(EMGShowdownModifier::AggresiveAI);
		RegisterPhase(FinalPhase);
	}

	// Register default showdowns
	{
		FMGShowdownDefinition ShadowShowdown;
		ShadowShowdown.ShowdownId = TEXT("SHOWDOWN_SHADOW_KING");
		ShadowShowdown.DisplayName = FText::FromString(TEXT("Shadow King's Challenge"));
		ShadowShowdown.Description = FText::FromString(TEXT("Face the legendary Shadow King in the ultimate midnight showdown"));
		ShadowShowdown.IntroDialogue = FText::FromString(TEXT("At midnight, we race. Winner takes all."));
		ShadowShowdown.VictoryDialogue = FText::FromString(TEXT("Impossible... you've broken my curse."));
		ShadowShowdown.DefeatDialogue = FText::FromString(TEXT("You're not ready. Come back when you've truly mastered the night."));
		ShadowShowdown.Type = EMGShowdownType::BossRace;
		ShadowShowdown.Difficulty = EMGShowdownDifficulty::Hard;
		ShadowShowdown.BossId = TEXT("BOSS_SHADOW_KING");
		ShadowShowdown.BossName = FText::FromString(TEXT("Shadow King"));
		ShadowShowdown.BossSkillLevel = 95;
		ShadowShowdown.PhaseIds.Add(TEXT("PHASE_INTRO"));
		ShadowShowdown.PhaseIds.Add(TEXT("PHASE_CHASE"));
		ShadowShowdown.PhaseIds.Add(TEXT("PHASE_RACE"));
		ShadowShowdown.PhaseIds.Add(TEXT("PHASE_FINALE"));
		ShadowShowdown.Modifiers.Add(EMGShowdownModifier::NightOnly);
		ShadowShowdown.TrackId = TEXT("TRACK_MIDNIGHT_CANYON");
		ShadowShowdown.RequiredLevel = 50;
		ShadowShowdown.RewardCurrency = 250000;
		ShadowShowdown.RewardExperience = 10000;
		ShadowShowdown.RewardReputation = 5000;
		ShadowShowdown.RewardVehicleId = TEXT("VEHICLE_SHADOW_PHANTOM");
		ShadowShowdown.RewardTitleId = TEXT("TITLE_SHADOW_SLAYER");
		ShadowShowdown.bIsRepeatable = true;
		RegisterShowdown(ShadowShowdown);
	}
	{
		FMGShowdownDefinition PhoenixShowdown;
		PhoenixShowdown.ShowdownId = TEXT("SHOWDOWN_PHOENIX_LEGEND");
		PhoenixShowdown.DisplayName = FText::FromString(TEXT("Legend's Final Stand"));
		PhoenixShowdown.Description = FText::FromString(TEXT("Challenge the undefeated Phoenix in the ultimate test of skill"));
		PhoenixShowdown.IntroDialogue = FText::FromString(TEXT("You've come far, but this is where legends end their journey."));
		PhoenixShowdown.VictoryDialogue = FText::FromString(TEXT("At last... a worthy successor. Take my flame."));
		PhoenixShowdown.DefeatDialogue = FText::FromString(TEXT("The legend continues. Perhaps another lifetime."));
		PhoenixShowdown.Type = EMGShowdownType::LegendChallenge;
		PhoenixShowdown.Difficulty = EMGShowdownDifficulty::Nightmare;
		PhoenixShowdown.BossId = TEXT("BOSS_PHOENIX_LEGEND");
		PhoenixShowdown.BossName = FText::FromString(TEXT("Phoenix"));
		PhoenixShowdown.BossSkillLevel = 99;
		PhoenixShowdown.PhaseIds.Add(TEXT("PHASE_INTRO"));
		PhoenixShowdown.PhaseIds.Add(TEXT("PHASE_CHASE"));
		PhoenixShowdown.PhaseIds.Add(TEXT("PHASE_BATTLE"));
		PhoenixShowdown.PhaseIds.Add(TEXT("PHASE_RACE"));
		PhoenixShowdown.PhaseIds.Add(TEXT("PHASE_FINALE"));
		PhoenixShowdown.Modifiers.Add(EMGShowdownModifier::AggresiveAI);
		PhoenixShowdown.Modifiers.Add(EMGShowdownModifier::NoDamage);
		PhoenixShowdown.RequiredLevel = 80;
		PhoenixShowdown.RequiredCompletedShowdowns.Add(TEXT("SHOWDOWN_SHADOW_KING"));
		PhoenixShowdown.RewardCurrency = 500000;
		PhoenixShowdown.RewardExperience = 25000;
		PhoenixShowdown.RewardReputation = 10000;
		PhoenixShowdown.RewardVehicleId = TEXT("VEHICLE_PHOENIX_FIRE");
		PhoenixShowdown.RewardTitleId = TEXT("TITLE_LEGEND_SLAYER");
		RegisterShowdown(PhoenixShowdown);
	}
	{
		FMGShowdownDefinition StormShowdown;
		StormShowdown.ShowdownId = TEXT("SHOWDOWN_STORM_RIDER");
		StormShowdown.DisplayName = FText::FromString(TEXT("Storm's Fury"));
		StormShowdown.Description = FText::FromString(TEXT("Race the Storm Rider through treacherous weather"));
		StormShowdown.Type = EMGShowdownType::BossRace;
		StormShowdown.Difficulty = EMGShowdownDifficulty::Extreme;
		StormShowdown.BossId = TEXT("BOSS_STORM_RIDER");
		StormShowdown.BossName = FText::FromString(TEXT("Storm Rider"));
		StormShowdown.BossSkillLevel = 88;
		StormShowdown.PhaseIds.Add(TEXT("PHASE_INTRO"));
		StormShowdown.PhaseIds.Add(TEXT("PHASE_CHASE"));
		StormShowdown.PhaseIds.Add(TEXT("PHASE_RACE"));
		StormShowdown.Modifiers.Add(EMGShowdownModifier::WeatherHazard);
		StormShowdown.TrackId = TEXT("TRACK_STORM_HIGHWAY");
		StormShowdown.RequiredLevel = 35;
		StormShowdown.RewardCurrency = 150000;
		StormShowdown.RewardExperience = 7500;
		StormShowdown.RewardReputation = 3000;
		StormShowdown.bIsRepeatable = true;
		RegisterShowdown(StormShowdown);
	}

	// Start showdown tick
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGShowdownSubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(ShowdownTickTimer, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->TickShowdowns(0.1f);
			}
		}, 0.1f, true);
	}
}

void UMGShowdownSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ShowdownTickTimer);
	}

	SaveShowdownData();

	ShowdownDefinitions.Empty();
	PhaseDefinitions.Empty();
	BossEncounters.Empty();
	ActiveShowdowns.Empty();
	UnlockedShowdowns.Empty();
	CompletedShowdowns.Empty();
	PlayerRecords.Empty();
	PlayerStats.Empty();
	WorldRecords.Empty();
	WorldRecordHolders.Empty();

	Super::Deinitialize();
}

// Registration
void UMGShowdownSubsystem::RegisterShowdown(const FMGShowdownDefinition& Showdown)
{
	if (!Showdown.ShowdownId.IsEmpty())
	{
		ShowdownDefinitions.Add(Showdown.ShowdownId, Showdown);
	}
}

void UMGShowdownSubsystem::RegisterPhase(const FMGBossPhaseDefinition& Phase)
{
	if (!Phase.PhaseId.IsEmpty())
	{
		PhaseDefinitions.Add(Phase.PhaseId, Phase);
	}
}

void UMGShowdownSubsystem::RegisterBoss(const FMGBossEncounter& Boss)
{
	if (!Boss.BossId.IsEmpty())
	{
		BossEncounters.Add(Boss.BossId, Boss);
	}
}

// Showdown Actions
bool UMGShowdownSubsystem::StartShowdown(const FString& PlayerId, const FString& ShowdownId)
{
	if (!IsShowdownUnlocked(PlayerId, ShowdownId))
	{
		return false;
	}

	if (HasActiveShowdown(PlayerId))
	{
		return false;
	}

	const FMGShowdownDefinition* Definition = ShowdownDefinitions.Find(ShowdownId);
	if (!Definition)
	{
		return false;
	}

	if (!CheckShowdownRequirements(PlayerId, *Definition))
	{
		return false;
	}

	// Create active showdown
	FMGActiveShowdown Active;
	Active.InstanceId = GenerateInstanceId();
	Active.ShowdownId = ShowdownId;
	Active.PlayerId = PlayerId;
	Active.Status = EMGShowdownStatus::InProgress;
	Active.StartTime = FDateTime::Now();
	Active.BossHealthRemaining = 100;
	Active.bIsFirstAttempt = true;

	if (Definition->ExpirationHours > 0.0f)
	{
		Active.ExpirationTime = Active.StartTime + FTimespan::FromHours(Definition->ExpirationHours);
	}

	// Initialize phase completions
	for (const FString& PhaseId : Definition->PhaseIds)
	{
		Active.PhaseCompletions.Add(PhaseId, false);
	}

	// Set first phase
	if (Definition->PhaseIds.Num() > 0)
	{
		Active.CurrentPhaseId = Definition->PhaseIds[0];
		Active.CurrentPhaseIndex = 0;
	}

	ActiveShowdowns.Add(PlayerId, Active);

	// Update stats
	FMGShowdownPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGShowdownPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}
	Stats->TotalShowdownsAttempted++;

	OnShowdownStarted.Broadcast(PlayerId, ShowdownId, Definition->Type);
	OnPhaseStarted.Broadcast(PlayerId, ShowdownId, 0, Active.CurrentPhaseId);

	return true;
}

void UMGShowdownSubsystem::AbandonShowdown(const FString& PlayerId, const FString& ShowdownId)
{
	FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return;
	}

	Active->Status = EMGShowdownStatus::Failed;
	OnShowdownFailed.Broadcast(PlayerId, ShowdownId, Active->CurrentPhaseIndex);

	ActiveShowdowns.Remove(PlayerId);
}

FMGShowdownResult UMGShowdownSubsystem::CompleteShowdown(const FString& PlayerId, const FString& ShowdownId)
{
	FMGShowdownResult Result;
	Result.ShowdownId = ShowdownId;
	Result.PlayerId = PlayerId;
	Result.Timestamp = FDateTime::Now();

	FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return Result;
	}

	const FMGShowdownDefinition* Definition = ShowdownDefinitions.Find(ShowdownId);
	if (!Definition)
	{
		return Result;
	}

	Result.bVictory = true;
	Active->Status = EMGShowdownStatus::Completed;

	// Calculate results
	Result.TotalTime = Active->TotalTime;
	Result.AttemptsUsed = Active->AttemptsUsed;
	Result.FinalScore = Active->PlayerScore;
	Result.TotalPhases = Definition->PhaseIds.Num();

	// Count completed phases
	for (const auto& Pair : Active->PhaseCompletions)
	{
		if (Pair.Value)
		{
			Result.PhasesCompleted++;
		}
	}

	// Check for perfect run
	Result.bPerfectRun = (Result.PhasesCompleted == Result.TotalPhases && Active->AttemptsUsed == 1);

	// Check for first completion
	TSet<FString>* Completed = CompletedShowdowns.Find(PlayerId);
	if (!Completed)
	{
		TSet<FString> NewSet;
		Completed = &CompletedShowdowns.Add(PlayerId, NewSet);
	}
	Result.bFirstCompletion = !Completed->Contains(ShowdownId);
	Completed->Add(ShowdownId);

	// Calculate rewards
	float Multiplier = GetDifficultyMultiplier(Definition->Difficulty);
	if (Result.bPerfectRun)
	{
		Multiplier *= 2.0f;
	}
	if (Result.bFirstCompletion)
	{
		Multiplier *= 1.5f;
	}

	Result.CurrencyEarned = FMath::RoundToInt(Definition->RewardCurrency * Multiplier);
	Result.ExperienceEarned = FMath::RoundToInt(Definition->RewardExperience * Multiplier);
	Result.ReputationEarned = FMath::RoundToInt(Definition->RewardReputation * Multiplier);

	if (Result.bFirstCompletion)
	{
		Result.VehicleRewardId = Definition->RewardVehicleId;
		Result.PartRewardId = Definition->RewardPartId;
		Result.TitleRewardId = Definition->RewardTitleId;
	}

	// Update records
	UpdateRecords(PlayerId, ShowdownId, Result);

	// Update player stats
	UpdatePlayerStats(PlayerId, Result);

	// Boss defeated
	if (!Definition->BossId.IsEmpty())
	{
		OnBossDefeated.Broadcast(PlayerId, Definition->BossId, Result.TotalTime);
	}

	OnShowdownCompleted.Broadcast(PlayerId, Result);

	ActiveShowdowns.Remove(PlayerId);

	return Result;
}

void UMGShowdownSubsystem::FailShowdown(const FString& PlayerId, const FString& ShowdownId)
{
	FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return;
	}

	Active->Status = EMGShowdownStatus::Failed;

	// Update stats
	FMGShowdownPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		Stats->TotalShowdownsFailed++;
	}

	OnShowdownFailed.Broadcast(PlayerId, ShowdownId, Active->CurrentPhaseIndex);

	// Don't remove yet - allow retry
}

bool UMGShowdownSubsystem::RetryShowdown(const FString& PlayerId, const FString& ShowdownId, bool bFromCheckpoint)
{
	FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return false;
	}

	const FMGShowdownDefinition* Definition = ShowdownDefinitions.Find(ShowdownId);
	if (!Definition)
	{
		return false;
	}

	// Check max attempts
	if (Definition->MaxAttempts > 0 && Active->AttemptsUsed >= Definition->MaxAttempts)
	{
		ActiveShowdowns.Remove(PlayerId);
		return false;
	}

	Active->AttemptsUsed++;
	Active->bIsFirstAttempt = false;
	Active->Status = EMGShowdownStatus::InProgress;

	// Update stats
	FMGShowdownPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		Stats->TotalRetries++;
	}

	if (bFromCheckpoint && Active->LastCheckpoint >= 0)
	{
		// Resume from checkpoint
		Active->CurrentPhaseIndex = Active->LastCheckpoint;
		if (Active->CurrentPhaseIndex < Definition->PhaseIds.Num())
		{
			Active->CurrentPhaseId = Definition->PhaseIds[Active->CurrentPhaseIndex];
		}
	}
	else
	{
		// Full restart
		Active->CurrentPhaseIndex = 0;
		if (Definition->PhaseIds.Num() > 0)
		{
			Active->CurrentPhaseId = Definition->PhaseIds[0];
		}
		Active->PhaseTime = 0.0f;
		Active->TotalTime = 0.0f;
		Active->BossHealthRemaining = 100;
		Active->PlayerScore = 0;

		for (auto& Pair : Active->PhaseCompletions)
		{
			Pair.Value = false;
		}
	}

	OnPhaseStarted.Broadcast(PlayerId, ShowdownId, Active->CurrentPhaseIndex, Active->CurrentPhaseId);
	return true;
}

// Phase Management
void UMGShowdownSubsystem::AdvancePhase(const FString& PlayerId, const FString& ShowdownId)
{
	FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return;
	}

	const FMGShowdownDefinition* Definition = ShowdownDefinitions.Find(ShowdownId);
	if (!Definition)
	{
		return;
	}

	// Mark current phase complete
	bool* PhaseComplete = Active->PhaseCompletions.Find(Active->CurrentPhaseId);
	if (PhaseComplete)
	{
		*PhaseComplete = true;
	}

	OnPhaseCompleted.Broadcast(PlayerId, ShowdownId, Active->CurrentPhaseIndex, Active->PhaseTime);

	// Check for checkpoint
	const FMGBossPhaseDefinition* PhaseDef = PhaseDefinitions.Find(Active->CurrentPhaseId);
	if (PhaseDef && PhaseDef->CheckpointIndex >= 0)
	{
		SetCheckpoint(PlayerId, ShowdownId, PhaseDef->CheckpointIndex);
	}

	// Advance to next phase
	Active->CurrentPhaseIndex++;
	Active->PhaseTime = 0.0f;

	if (Active->CurrentPhaseIndex >= Definition->PhaseIds.Num())
	{
		// All phases complete - showdown victory
		CompleteShowdown(PlayerId, ShowdownId);
	}
	else
	{
		Active->CurrentPhaseId = Definition->PhaseIds[Active->CurrentPhaseIndex];
		OnPhaseStarted.Broadcast(PlayerId, ShowdownId, Active->CurrentPhaseIndex, Active->CurrentPhaseId);
	}
}

void UMGShowdownSubsystem::CompleteCurrentPhase(const FString& PlayerId, const FString& ShowdownId)
{
	AdvancePhase(PlayerId, ShowdownId);
}

void UMGShowdownSubsystem::SetCheckpoint(const FString& PlayerId, const FString& ShowdownId, int32 CheckpointIndex)
{
	FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return;
	}

	if (CheckpointIndex > Active->LastCheckpoint)
	{
		Active->LastCheckpoint = CheckpointIndex;
		OnCheckpointReached.Broadcast(PlayerId, ShowdownId, CheckpointIndex);
	}
}

FMGBossPhaseDefinition UMGShowdownSubsystem::GetCurrentPhase(const FString& PlayerId, const FString& ShowdownId) const
{
	const FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return FMGBossPhaseDefinition();
	}

	if (const FMGBossPhaseDefinition* Phase = PhaseDefinitions.Find(Active->CurrentPhaseId))
	{
		return *Phase;
	}
	return FMGBossPhaseDefinition();
}

int32 UMGShowdownSubsystem::GetCurrentPhaseIndex(const FString& PlayerId, const FString& ShowdownId) const
{
	const FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	if (!Active || Active->ShowdownId != ShowdownId)
	{
		return -1;
	}
	return Active->CurrentPhaseIndex;
}

// Boss Interactions
void UMGShowdownSubsystem::DamageBoss(const FString& ShowdownId, int32 Damage)
{
	for (auto& Pair : ActiveShowdowns)
	{
		if (Pair.Value.ShowdownId == ShowdownId)
		{
			Pair.Value.BossHealthRemaining = FMath::Max(0, Pair.Value.BossHealthRemaining - Damage);
			OnBossHealthChanged.Broadcast(ShowdownId, Pair.Value.BossHealthRemaining, 100);

			if (Pair.Value.BossHealthRemaining <= 0)
			{
				// Boss defeated, advance phase or complete
				AdvancePhase(Pair.Key, ShowdownId);
			}
			break;
		}
	}
}

int32 UMGShowdownSubsystem::GetBossHealth(const FString& ShowdownId) const
{
	for (const auto& Pair : ActiveShowdowns)
	{
		if (Pair.Value.ShowdownId == ShowdownId)
		{
			return Pair.Value.BossHealthRemaining;
		}
	}
	return 0;
}

FMGBossEncounter UMGShowdownSubsystem::GetBoss(const FString& BossId) const
{
	if (const FMGBossEncounter* Boss = BossEncounters.Find(BossId))
	{
		return *Boss;
	}
	return FMGBossEncounter();
}

float UMGShowdownSubsystem::GetBossAggressionLevel(const FString& ShowdownId) const
{
	for (const auto& Pair : ActiveShowdowns)
	{
		if (Pair.Value.ShowdownId == ShowdownId)
		{
			const FMGBossPhaseDefinition* Phase = PhaseDefinitions.Find(Pair.Value.CurrentPhaseId);
			if (Phase)
			{
				return Phase->BossAggressionLevel;
			}
			break;
		}
	}
	return 0.5f;
}

// Queries
FMGShowdownDefinition UMGShowdownSubsystem::GetShowdownDefinition(const FString& ShowdownId) const
{
	if (const FMGShowdownDefinition* Def = ShowdownDefinitions.Find(ShowdownId))
	{
		return *Def;
	}
	return FMGShowdownDefinition();
}

FMGActiveShowdown UMGShowdownSubsystem::GetActiveShowdown(const FString& PlayerId, const FString& ShowdownId) const
{
	if (const FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId))
	{
		if (Active->ShowdownId == ShowdownId)
		{
			return *Active;
		}
	}
	return FMGActiveShowdown();
}

TArray<FMGShowdownDefinition> UMGShowdownSubsystem::GetAvailableShowdowns(const FString& PlayerId) const
{
	TArray<FMGShowdownDefinition> Result;

	for (const auto& Pair : ShowdownDefinitions)
	{
		if (IsShowdownUnlocked(PlayerId, Pair.Key))
		{
			if (!IsShowdownCompleted(PlayerId, Pair.Key) || Pair.Value.bIsRepeatable)
			{
				Result.Add(Pair.Value);
			}
		}
	}

	return Result;
}

TArray<FMGShowdownDefinition> UMGShowdownSubsystem::GetShowdownsByType(EMGShowdownType Type) const
{
	TArray<FMGShowdownDefinition> Result;

	for (const auto& Pair : ShowdownDefinitions)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

bool UMGShowdownSubsystem::IsShowdownUnlocked(const FString& PlayerId, const FString& ShowdownId) const
{
	// Check if explicitly unlocked
	const TSet<FString>* Unlocked = UnlockedShowdowns.Find(PlayerId);
	if (Unlocked && Unlocked->Contains(ShowdownId))
	{
		return true;
	}

	// Check if requirements are met
	const FMGShowdownDefinition* Def = ShowdownDefinitions.Find(ShowdownId);
	if (Def)
	{
		return CheckShowdownRequirements(PlayerId, *Def);
	}

	return false;
}

bool UMGShowdownSubsystem::IsShowdownCompleted(const FString& PlayerId, const FString& ShowdownId) const
{
	const TSet<FString>* Completed = CompletedShowdowns.Find(PlayerId);
	return Completed && Completed->Contains(ShowdownId);
}

bool UMGShowdownSubsystem::HasActiveShowdown(const FString& PlayerId) const
{
	const FMGActiveShowdown* Active = ActiveShowdowns.Find(PlayerId);
	return Active && Active->Status == EMGShowdownStatus::InProgress;
}

// Records
FMGShowdownRecord UMGShowdownSubsystem::GetShowdownRecord(const FString& PlayerId, const FString& ShowdownId) const
{
	const TMap<FString, FMGShowdownRecord>* Records = PlayerRecords.Find(PlayerId);
	if (Records)
	{
		if (const FMGShowdownRecord* Record = Records->Find(ShowdownId))
		{
			return *Record;
		}
	}
	return FMGShowdownRecord();
}

float UMGShowdownSubsystem::GetPersonalBestTime(const FString& PlayerId, const FString& ShowdownId) const
{
	const TMap<FString, FMGShowdownRecord>* Records = PlayerRecords.Find(PlayerId);
	if (Records)
	{
		if (const FMGShowdownRecord* Record = Records->Find(ShowdownId))
		{
			return Record->PersonalBestTime;
		}
	}
	return 0.0f;
}

float UMGShowdownSubsystem::GetWorldRecordTime(const FString& ShowdownId) const
{
	const float* Record = WorldRecords.Find(ShowdownId);
	return Record ? *Record : 0.0f;
}

void UMGShowdownSubsystem::SetWorldRecord(const FString& ShowdownId, float Time, const FString& PlayerName)
{
	WorldRecords.Add(ShowdownId, Time);
	WorldRecordHolders.Add(ShowdownId, PlayerName);
}

// Stats
FMGShowdownPlayerStats UMGShowdownSubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGShowdownPlayerStats* Stats = PlayerStats.Find(PlayerId))
	{
		return *Stats;
	}
	return FMGShowdownPlayerStats();
}

void UMGShowdownSubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGShowdownPlayerStats NewStats;
	NewStats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, NewStats);
}

// Unlocks
void UMGShowdownSubsystem::UnlockShowdown(const FString& PlayerId, const FString& ShowdownId)
{
	TSet<FString>* Unlocked = UnlockedShowdowns.Find(PlayerId);
	if (!Unlocked)
	{
		TSet<FString> NewSet;
		Unlocked = &UnlockedShowdowns.Add(PlayerId, NewSet);
	}

	if (!Unlocked->Contains(ShowdownId))
	{
		Unlocked->Add(ShowdownId);
		OnShowdownUnlocked.Broadcast(PlayerId, ShowdownId);
	}
}

void UMGShowdownSubsystem::CheckUnlockRequirements(const FString& PlayerId)
{
	for (const auto& Pair : ShowdownDefinitions)
	{
		if (!IsShowdownUnlocked(PlayerId, Pair.Key))
		{
			if (CheckShowdownRequirements(PlayerId, Pair.Value))
			{
				UnlockShowdown(PlayerId, Pair.Key);
			}
		}
	}
}

// Update
void UMGShowdownSubsystem::UpdateShowdownSystem(float DeltaTime)
{
	TickShowdowns(DeltaTime);
}

// Protected
void UMGShowdownSubsystem::TickShowdowns(float DeltaTime)
{
	UpdateActiveShowdowns(DeltaTime);
}

void UMGShowdownSubsystem::UpdateActiveShowdowns(float DeltaTime)
{
	FDateTime Now = FDateTime::Now();

	for (auto& Pair : ActiveShowdowns)
	{
		FMGActiveShowdown& Active = Pair.Value;

		if (Active.Status != EMGShowdownStatus::InProgress)
		{
			continue;
		}

		// Update times
		Active.PhaseTime += DeltaTime;
		Active.TotalTime += DeltaTime;

		// Check expiration
		const FMGShowdownDefinition* Def = ShowdownDefinitions.Find(Active.ShowdownId);
		if (Def && Def->ExpirationHours > 0.0f)
		{
			if (Now >= Active.ExpirationTime)
			{
				Active.Status = EMGShowdownStatus::Expired;
				OnShowdownFailed.Broadcast(Pair.Key, Active.ShowdownId, Active.CurrentPhaseIndex);
			}
		}

		// Check phase time limit
		const FMGBossPhaseDefinition* Phase = PhaseDefinitions.Find(Active.CurrentPhaseId);
		if (Phase && Phase->Duration > 0.0f && Active.PhaseTime >= Phase->Duration)
		{
			FailShowdown(Pair.Key, Active.ShowdownId);
		}
	}
}

void UMGShowdownSubsystem::UpdatePlayerStats(const FString& PlayerId, const FMGShowdownResult& Result)
{
	FMGShowdownPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGShowdownPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	if (Result.bVictory)
	{
		Stats->TotalShowdownsCompleted++;
		Stats->TotalCurrencyEarned += Result.CurrencyEarned;
		Stats->BossesDefeated++;

		if (Result.bPerfectRun)
		{
			Stats->PerfectRuns++;
		}

		if (Stats->FastestBossDefeat == 0.0f || Result.TotalTime < Stats->FastestBossDefeat)
		{
			Stats->FastestBossDefeat = Result.TotalTime;
		}

		const FMGShowdownDefinition* Def = ShowdownDefinitions.Find(Result.ShowdownId);
		if (Def)
		{
			int32& TypeCount = Stats->CompletionsByType.FindOrAdd(Def->Type);
			TypeCount++;

			int32& DiffCount = Stats->CompletionsByDifficulty.FindOrAdd(Def->Difficulty);
			DiffCount++;

			if (!Def->BossId.IsEmpty() && !Stats->UnlockedBosses.Contains(Def->BossId))
			{
				Stats->UnlockedBosses.Add(Def->BossId);
			}

			float* BestTime = Stats->BestTimesByShowdown.Find(Result.ShowdownId);
			if (!BestTime || Result.TotalTime < *BestTime)
			{
				Stats->BestTimesByShowdown.Add(Result.ShowdownId, Result.TotalTime);
			}
		}
	}
}

void UMGShowdownSubsystem::UpdateRecords(const FString& PlayerId, const FString& ShowdownId, const FMGShowdownResult& Result)
{
	TMap<FString, FMGShowdownRecord>* Records = PlayerRecords.Find(PlayerId);
	if (!Records)
	{
		TMap<FString, FMGShowdownRecord> NewMap;
		Records = &PlayerRecords.Add(PlayerId, NewMap);
	}

	FMGShowdownRecord* Record = Records->Find(ShowdownId);
	if (!Record)
	{
		FMGShowdownRecord NewRecord;
		NewRecord.ShowdownId = ShowdownId;
		NewRecord.FirstCompletionDate = FDateTime::Now();
		Record = &Records->Add(ShowdownId, NewRecord);
	}

	Record->TotalAttempts += Result.AttemptsUsed;
	if (Result.bVictory)
	{
		Record->TimesCompleted++;

		if (Record->PersonalBestTime == 0.0f || Result.TotalTime < Record->PersonalBestTime)
		{
			Record->PersonalBestTime = Result.TotalTime;
			Record->BestTimeDate = FDateTime::Now();
			OnNewShowdownRecord.Broadcast(PlayerId, ShowdownId, Result.TotalTime);
		}

		if (Result.FinalScore > Record->PersonalBestScore)
		{
			Record->PersonalBestScore = Result.FinalScore;
		}

		if (Result.bPerfectRun)
		{
			Record->bPerfectRunAchieved = true;
		}

		// Check world record
		float* WorldRecord = WorldRecords.Find(ShowdownId);
		if (!WorldRecord || Result.TotalTime < *WorldRecord)
		{
			SetWorldRecord(ShowdownId, Result.TotalTime, PlayerId);
		}
	}
}

bool UMGShowdownSubsystem::CheckShowdownRequirements(const FString& PlayerId, const FMGShowdownDefinition& Showdown) const
{
	// Check required completed showdowns
	for (const FString& RequiredId : Showdown.RequiredCompletedShowdowns)
	{
		if (!IsShowdownCompleted(PlayerId, RequiredId))
		{
			return false;
		}
	}

	// Check level requirements via Career subsystem
	if (Showdown.RequiredLevel > 1)
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
		{
			if (UMGCareerSubsystem* CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>())
			{
				// Map career chapter to level (1-5 chapters = levels 1-50 in increments)
				// Newcomer=1-10, Rising=11-20, Contender=21-30, Champion=31-40, Legend=41-50
				const EMGCareerChapter CurrentChapter = CareerSubsystem->GetCurrentChapter();
				const float ChapterProgress = CareerSubsystem->GetChapterProgressPercent();

				int32 PlayerLevel = 1;
				switch (CurrentChapter)
				{
					case EMGCareerChapter::Newcomer:
						PlayerLevel = 1 + FMath::FloorToInt(ChapterProgress * 0.1f);
						break;
					case EMGCareerChapter::Rising:
						PlayerLevel = 11 + FMath::FloorToInt(ChapterProgress * 0.1f);
						break;
					case EMGCareerChapter::Contender:
						PlayerLevel = 21 + FMath::FloorToInt(ChapterProgress * 0.1f);
						break;
					case EMGCareerChapter::Champion:
						PlayerLevel = 31 + FMath::FloorToInt(ChapterProgress * 0.1f);
						break;
					case EMGCareerChapter::Legend:
						PlayerLevel = 41 + FMath::FloorToInt(ChapterProgress * 0.1f);
						break;
				}

				if (PlayerLevel < Showdown.RequiredLevel)
				{
					return false;
				}
			}
		}
	}

	// Check story progress requirements
	if (!Showdown.RequiredStoryProgress.IsEmpty())
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
		{
			if (UMGCareerSubsystem* CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>())
			{
				// Check if the required story milestone/objective has been completed
				// RequiredStoryProgress could be a milestone name like "DefeatedRival"
				// or a specific objective ID

				// Check milestones first
				if (Showdown.RequiredStoryProgress.StartsWith(TEXT("MILESTONE_")))
				{
					FString MilestoneName = Showdown.RequiredStoryProgress.RightChop(10);

					// Map string to milestone enum
					EMGCareerMilestone RequiredMilestone = EMGCareerMilestone::FirstRace;
					if (MilestoneName == TEXT("FirstWin")) RequiredMilestone = EMGCareerMilestone::FirstWin;
					else if (MilestoneName == TEXT("FirstPodium")) RequiredMilestone = EMGCareerMilestone::FirstPodium;
					else if (MilestoneName == TEXT("JoinedCrew")) RequiredMilestone = EMGCareerMilestone::JoinedCrew;
					else if (MilestoneName == TEXT("DefeatedRival")) RequiredMilestone = EMGCareerMilestone::DefeatedRival;
					else if (MilestoneName == TEXT("WonTournament")) RequiredMilestone = EMGCareerMilestone::WonTournament;
					else if (MilestoneName == TEXT("ReachedContender")) RequiredMilestone = EMGCareerMilestone::ReachedContender;
					else if (MilestoneName == TEXT("BecameChampion")) RequiredMilestone = EMGCareerMilestone::BecameChampion;
					else if (MilestoneName == TEXT("EarnedLegendStatus")) RequiredMilestone = EMGCareerMilestone::EarnedLegendStatus;

					if (!CareerSubsystem->HasCompletedMilestone(RequiredMilestone))
					{
						return false;
					}
				}
				else if (Showdown.RequiredStoryProgress.StartsWith(TEXT("CHAPTER_")))
				{
					// Check chapter requirement
					FString ChapterName = Showdown.RequiredStoryProgress.RightChop(8);
					EMGCareerChapter RequiredChapter = EMGCareerChapter::Newcomer;

					if (ChapterName == TEXT("Rising")) RequiredChapter = EMGCareerChapter::Rising;
					else if (ChapterName == TEXT("Contender")) RequiredChapter = EMGCareerChapter::Contender;
					else if (ChapterName == TEXT("Champion")) RequiredChapter = EMGCareerChapter::Champion;
					else if (ChapterName == TEXT("Legend")) RequiredChapter = EMGCareerChapter::Legend;

					if (static_cast<int32>(CareerSubsystem->GetCurrentChapter()) < static_cast<int32>(RequiredChapter))
					{
						return false;
					}
				}
			}
		}
	}

	return true;
}

float UMGShowdownSubsystem::GetDifficultyMultiplier(EMGShowdownDifficulty Difficulty) const
{
	switch (Difficulty)
	{
		case EMGShowdownDifficulty::Normal: return 1.0f;
		case EMGShowdownDifficulty::Hard: return 1.5f;
		case EMGShowdownDifficulty::Extreme: return 2.0f;
		case EMGShowdownDifficulty::Nightmare: return 3.0f;
		case EMGShowdownDifficulty::Impossible: return 5.0f;
		default: return 1.0f;
	}
}

FString UMGShowdownSubsystem::GenerateInstanceId() const
{
	return FString::Printf(TEXT("SHOWDOWN_%d_%lld"), ++const_cast<UMGShowdownSubsystem*>(this)->InstanceCounter, FDateTime::Now().GetTicks());
}

// Persistence
void UMGShowdownSubsystem::SaveShowdownData()
{
	// Save implementation would persist showdown progress, records, and stats
}

void UMGShowdownSubsystem::LoadShowdownData()
{
	// Load implementation would restore showdown progress, records, and stats
}

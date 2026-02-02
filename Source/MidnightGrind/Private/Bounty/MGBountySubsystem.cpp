// Copyright Midnight Grind. All Rights Reserved.

#include "Bounty/MGBountySubsystem.h"
#include "Career/MGCareerSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"

void UMGBountySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Register some default bounty targets
	{
		FMGBountyTarget Boss1;
		Boss1.TargetId = TEXT("BOSS_SHADOW");
		Boss1.DisplayName = FText::FromString(TEXT("Shadow"));
		Boss1.Title = FText::FromString(TEXT("The Phantom Racer"));
		Boss1.Biography = FText::FromString(TEXT("A legendary street racer who appears only at midnight. No one has ever seen their face."));
		Boss1.Type = EMGBountyTargetType::BossRacer;
		Boss1.SkillLevel = 85;
		Boss1.HomeDistrict = TEXT("Downtown");
		Boss1.TotalBountyValue = 50000;
		RegisterTarget(Boss1);
	}
	{
		FMGBountyTarget Boss2;
		Boss2.TargetId = TEXT("BOSS_THUNDER");
		Boss2.DisplayName = FText::FromString(TEXT("Thunder"));
		Boss2.Title = FText::FromString(TEXT("King of the Highway"));
		Boss2.Biography = FText::FromString(TEXT("Dominates the highway battles. Known for aggressive takedowns."));
		Boss2.Type = EMGBountyTargetType::BossRacer;
		Boss2.SkillLevel = 90;
		Boss2.HomeDistrict = TEXT("Highway");
		Boss2.TotalBountyValue = 75000;
		RegisterTarget(Boss2);
	}
	{
		FMGBountyTarget Legend;
		Legend.TargetId = TEXT("LEGEND_PHOENIX");
		Legend.DisplayName = FText::FromString(TEXT("Phoenix"));
		Legend.Title = FText::FromString(TEXT("The Undying Champion"));
		Legend.Biography = FText::FromString(TEXT("Has never lost a race. They say defeating them is impossible."));
		Legend.Type = EMGBountyTargetType::LegendaryRacer;
		Legend.SkillLevel = 99;
		Legend.HomeDistrict = TEXT("Elite Circuit");
		Legend.TotalBountyValue = 150000;
		RegisterTarget(Legend);
	}

	// Register default bounties
	{
		FMGBountyDefinition ShadowBounty;
		ShadowBounty.BountyId = TEXT("BOUNTY_SHADOW_HUNT");
		ShadowBounty.DisplayName = FText::FromString(TEXT("Hunt the Shadow"));
		ShadowBounty.Description = FText::FromString(TEXT("Challenge and defeat Shadow in a midnight race"));
		ShadowBounty.FlavorText = FText::FromString(TEXT("They say if you flash your lights three times at midnight, Shadow will appear..."));
		ShadowBounty.Type = EMGBountyType::BossChallenge;
		ShadowBounty.Difficulty = EMGBountyDifficulty::Hard;
		ShadowBounty.TargetType = EMGBountyTargetType::BossRacer;
		ShadowBounty.TargetId = TEXT("BOSS_SHADOW");
		ShadowBounty.TargetName = FText::FromString(TEXT("Shadow"));
		ShadowBounty.RewardCurrency = 50000;
		ShadowBounty.RewardExperience = 2500;
		ShadowBounty.RewardReputation = 500;
		ShadowBounty.TimeLimit = 300.0f;
		ShadowBounty.ExpirationHours = 48.0f;
		ShadowBounty.RequiredPlayerLevel = 30;
		ShadowBounty.RequiredObjectives.Add(TEXT("LOCATE_TARGET"));
		ShadowBounty.RequiredObjectives.Add(TEXT("WIN_RACE"));
		ShadowBounty.OptionalObjectives.Add(TEXT("NO_REWINDS"));
		ShadowBounty.OptionalObjectives.Add(TEXT("WIN_BY_5_SECONDS"));
		ShadowBounty.BonusMultiplierPerOptional = 0.25f;
		RegisterBounty(ShadowBounty);
	}
	{
		FMGBountyDefinition ThunderBounty;
		ThunderBounty.BountyId = TEXT("BOUNTY_THUNDER_TAKEDOWN");
		ThunderBounty.DisplayName = FText::FromString(TEXT("Thunder's Reign"));
		ThunderBounty.Description = FText::FromString(TEXT("End Thunder's dominance on the highway"));
		ThunderBounty.FlavorText = FText::FromString(TEXT("The highway belongs to Thunder. Time to change that."));
		ThunderBounty.Type = EMGBountyType::BossChallenge;
		ThunderBounty.Difficulty = EMGBountyDifficulty::Expert;
		ThunderBounty.TargetType = EMGBountyTargetType::BossRacer;
		ThunderBounty.TargetId = TEXT("BOSS_THUNDER");
		ThunderBounty.TargetName = FText::FromString(TEXT("Thunder"));
		ThunderBounty.RewardCurrency = 75000;
		ThunderBounty.RewardExperience = 3500;
		ThunderBounty.RewardReputation = 750;
		ThunderBounty.ExpirationHours = 72.0f;
		ThunderBounty.RequiredPlayerLevel = 50;
		ThunderBounty.RequiredObjectives.Add(TEXT("REACH_HIGHWAY"));
		ThunderBounty.RequiredObjectives.Add(TEXT("CHALLENGE_THUNDER"));
		ThunderBounty.RequiredObjectives.Add(TEXT("TAKEDOWN_THUNDER"));
		ThunderBounty.OptionalObjectives.Add(TEXT("NO_DAMAGE"));
		ThunderBounty.OptionalObjectives.Add(TEXT("PERFORM_TAKEDOWN"));
		RegisterBounty(ThunderBounty);
	}
	{
		FMGBountyDefinition DailyRace;
		DailyRace.BountyId = TEXT("BOUNTY_DAILY_RACE");
		DailyRace.DisplayName = FText::FromString(TEXT("Daily Street Race"));
		DailyRace.Description = FText::FromString(TEXT("Win any street race"));
		DailyRace.Type = EMGBountyType::RaceWin;
		DailyRace.Difficulty = EMGBountyDifficulty::Easy;
		DailyRace.TargetType = EMGBountyTargetType::AIDriver;
		DailyRace.RewardCurrency = 5000;
		DailyRace.RewardExperience = 500;
		DailyRace.RewardReputation = 50;
		DailyRace.ExpirationHours = 24.0f;
		DailyRace.bIsRepeatable = true;
		DailyRace.RepeatCooldownHours = 24.0f;
		DailyRace.RequiredObjectives.Add(TEXT("WIN_RACE"));
		RegisterBounty(DailyRace);
	}
	{
		FMGBountyDefinition TakedownBounty;
		TakedownBounty.BountyId = TEXT("BOUNTY_TAKEDOWN_HUNTER");
		TakedownBounty.DisplayName = FText::FromString(TEXT("Takedown Hunter"));
		TakedownBounty.Description = FText::FromString(TEXT("Perform 5 takedowns in a single race"));
		TakedownBounty.Type = EMGBountyType::Takedown;
		TakedownBounty.Difficulty = EMGBountyDifficulty::Medium;
		TakedownBounty.RewardCurrency = 15000;
		TakedownBounty.RewardExperience = 1000;
		TakedownBounty.RewardReputation = 150;
		TakedownBounty.ExpirationHours = 48.0f;
		TakedownBounty.bIsRepeatable = true;
		TakedownBounty.RepeatCooldownHours = 12.0f;
		TakedownBounty.RequiredObjectives.Add(TEXT("TAKEDOWN_1"));
		TakedownBounty.RequiredObjectives.Add(TEXT("TAKEDOWN_2"));
		TakedownBounty.RequiredObjectives.Add(TEXT("TAKEDOWN_3"));
		TakedownBounty.RequiredObjectives.Add(TEXT("TAKEDOWN_4"));
		TakedownBounty.RequiredObjectives.Add(TEXT("TAKEDOWN_5"));
		TakedownBounty.OptionalObjectives.Add(TEXT("CHAIN_TAKEDOWN"));
		RegisterBounty(TakedownBounty);
	}
	{
		FMGBountyDefinition PursuitBounty;
		PursuitBounty.BountyId = TEXT("BOUNTY_HEAT_SURVIVOR");
		PursuitBounty.DisplayName = FText::FromString(TEXT("Heat Survivor"));
		PursuitBounty.Description = FText::FromString(TEXT("Escape a Heat Level 5 pursuit"));
		PursuitBounty.Type = EMGBountyType::PolicePursuit;
		PursuitBounty.Difficulty = EMGBountyDifficulty::Hard;
		PursuitBounty.RewardCurrency = 30000;
		PursuitBounty.RewardExperience = 2000;
		PursuitBounty.RewardReputation = 400;
		PursuitBounty.ExpirationHours = 72.0f;
		PursuitBounty.RequiredPlayerLevel = 20;
		PursuitBounty.RequiredObjectives.Add(TEXT("REACH_HEAT_5"));
		PursuitBounty.RequiredObjectives.Add(TEXT("ESCAPE_PURSUIT"));
		PursuitBounty.OptionalObjectives.Add(TEXT("DESTROY_COP_CARS"));
		PursuitBounty.OptionalObjectives.Add(TEXT("EVADE_HELICOPTER"));
		RegisterBounty(PursuitBounty);
	}

	// Start bounty tick
	if (UWorld* World = GetWorld())
	{
		TWeakObjectPtr<UMGBountySubsystem> WeakThis(this);
		World->GetTimerManager().SetTimer(BountyTickTimer, [WeakThis]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->TickBounties(1.0f);
			}
		}, 1.0f, true);
	}

	// Load saved data
	LoadBountyData();
}

void UMGBountySubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BountyTickTimer);
	}

	SaveBountyData();

	BountyDefinitions.Empty();
	Targets.Empty();
	PlayerActiveBounties.Empty();
	PlayerBountyBoards.Empty();
	PlayerStats.Empty();
	CompletedBounties.Empty();
	BountyCooldowns.Empty();
	CommunityBounties.Empty();

	Super::Deinitialize();
}

// Registration
void UMGBountySubsystem::RegisterBounty(const FMGBountyDefinition& Bounty)
{
	if (!Bounty.BountyId.IsEmpty())
	{
		BountyDefinitions.Add(Bounty.BountyId, Bounty);
	}
}

void UMGBountySubsystem::UnregisterBounty(const FString& BountyId)
{
	BountyDefinitions.Remove(BountyId);
}

void UMGBountySubsystem::RegisterTarget(const FMGBountyTarget& Target)
{
	if (!Target.TargetId.IsEmpty())
	{
		Targets.Add(Target.TargetId, Target);
	}
}

// Bounty Actions
bool UMGBountySubsystem::AcceptBounty(const FString& PlayerId, const FString& BountyId)
{
	if (!CanAcceptBounty(PlayerId, BountyId))
	{
		return false;
	}

	const FMGBountyDefinition* Definition = BountyDefinitions.Find(BountyId);
	if (!Definition)
	{
		return false;
	}

	// Get or create player active bounties
	TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		TMap<FString, FMGActiveBounty> NewMap;
		ActiveBounties = &PlayerActiveBounties.Add(PlayerId, NewMap);
	}

	// Create active bounty
	FMGActiveBounty NewBounty;
	NewBounty.InstanceId = GenerateInstanceId();
	NewBounty.BountyId = BountyId;
	NewBounty.PlayerId = PlayerId;
	NewBounty.Status = EMGBountyStatus::Accepted;
	NewBounty.AcceptedTime = FDateTime::Now();
	NewBounty.ExpirationTime = NewBounty.AcceptedTime + FTimespan::FromHours(Definition->ExpirationHours);
	NewBounty.TimeRemaining = Definition->TimeLimit;
	NewBounty.TotalObjectives = Definition->RequiredObjectives.Num();
	NewBounty.CurrentRewardMultiplier = GetDifficultyMultiplier(Definition->Difficulty);

	// Initialize objectives
	for (const FString& Objective : Definition->RequiredObjectives)
	{
		NewBounty.ObjectiveProgress.Add(Objective, false);
	}
	for (const FString& Objective : Definition->OptionalObjectives)
	{
		NewBounty.ObjectiveProgress.Add(Objective, false);
	}

	ActiveBounties->Add(BountyId, NewBounty);

	// Update bounty board
	FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (Board)
	{
		Board->ActiveBountyIds.AddUnique(BountyId);
		Board->AvailableBountyIds.Remove(BountyId);
	}

	OnBountyAccepted.Broadcast(PlayerId, BountyId);
	return true;
}

bool UMGBountySubsystem::AbandonBounty(const FString& PlayerId, const FString& BountyId)
{
	TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return false;
	}

	FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty)
	{
		return false;
	}

	Bounty->Status = EMGBountyStatus::Abandoned;

	// Update stats
	FMGBountyPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (Stats)
	{
		Stats->TotalBountiesAbandoned++;
	}

	// Break streak
	UpdateStreak(PlayerId, false);

	OnBountyAbandoned.Broadcast(PlayerId, BountyId);

	// Remove from active
	ActiveBounties->Remove(BountyId);

	// Update board
	FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (Board)
	{
		Board->ActiveBountyIds.Remove(BountyId);
	}

	return true;
}

FMGBountyCompletionResult UMGBountySubsystem::CompleteBounty(const FString& PlayerId, const FString& BountyId)
{
	FMGBountyCompletionResult Result;
	Result.BountyId = BountyId;
	Result.PlayerId = PlayerId;
	Result.Timestamp = FDateTime::Now();

	TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return Result;
	}

	FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty)
	{
		return Result;
	}

	const FMGBountyDefinition* Definition = BountyDefinitions.Find(BountyId);
	if (!Definition)
	{
		return Result;
	}

	// Check if all required objectives are complete
	if (!AreAllRequiredObjectivesComplete(PlayerId, BountyId))
	{
		FailBounty(PlayerId, BountyId, TEXT("Required objectives not completed"));
		return Result;
	}

	Result.bSuccess = true;
	Bounty->Status = EMGBountyStatus::Completed;

	// Calculate completion time
	FTimespan Duration = Result.Timestamp - Bounty->AcceptedTime;
	Result.CompletionTime = Duration.GetTotalSeconds();

	// Count completed objectives
	Result.ObjectivesCompleted = Bounty->CompletedObjectives;

	// Count optional objectives
	for (const FString& Optional : Definition->OptionalObjectives)
	{
		bool* Complete = Bounty->ObjectiveProgress.Find(Optional);
		if (Complete && *Complete)
		{
			Result.OptionalObjectivesCompleted++;
		}
	}

	// Calculate multiplier
	Result.FinalMultiplier = Bounty->CurrentRewardMultiplier;
	Result.FinalMultiplier += Result.OptionalObjectivesCompleted * Definition->BonusMultiplierPerOptional;

	// Check for perfect completion
	Result.bPerfectCompletion = (Result.OptionalObjectivesCompleted == Definition->OptionalObjectives.Num());
	if (Result.bPerfectCompletion)
	{
		Result.FinalMultiplier *= 1.5f;
	}

	// Check for first completion
	TSet<FString>* Completed = CompletedBounties.Find(PlayerId);
	if (!Completed)
	{
		TSet<FString> NewSet;
		Completed = &CompletedBounties.Add(PlayerId, NewSet);
	}
	Result.bFirstCompletion = !Completed->Contains(BountyId);
	if (Result.bFirstCompletion)
	{
		Result.FinalMultiplier *= 1.25f;
		Completed->Add(BountyId);
	}

	// Calculate rewards
	Result.CurrencyEarned = CalculateRewardCurrency(BountyId, Result.FinalMultiplier);
	Result.ExperienceEarned = CalculateRewardExperience(BountyId, Result.FinalMultiplier);
	Result.ReputationEarned = FMath::RoundToInt(Definition->RewardReputation * Result.FinalMultiplier);
	Result.SpecialRewardId = Definition->SpecialRewardId;

	// Update player stats
	UpdatePlayerStats(PlayerId, Result);

	// Update streak
	UpdateStreak(PlayerId, true);

	// Set cooldown if repeatable
	if (Definition->bIsRepeatable)
	{
		TMap<FString, FDateTime>* Cooldowns = BountyCooldowns.Find(PlayerId);
		if (!Cooldowns)
		{
			TMap<FString, FDateTime> NewCooldowns;
			Cooldowns = &BountyCooldowns.Add(PlayerId, NewCooldowns);
		}
		Cooldowns->Add(BountyId, FDateTime::Now() + FTimespan::FromHours(Definition->RepeatCooldownHours));
	}

	// Update target if applicable
	if (!Definition->TargetId.IsEmpty())
	{
		FMGBountyTarget* Target = Targets.Find(Definition->TargetId);
		if (Target)
		{
			Target->TimesCaptured++;
		}
	}

	OnBountyCompleted.Broadcast(PlayerId, Result);

	// Remove from active
	ActiveBounties->Remove(BountyId);

	// Update board
	FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (Board)
	{
		Board->ActiveBountyIds.Remove(BountyId);
	}

	return Result;
}

void UMGBountySubsystem::FailBounty(const FString& PlayerId, const FString& BountyId, const FString& Reason)
{
	TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return;
	}

	FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty)
	{
		return;
	}

	Bounty->Status = EMGBountyStatus::Failed;
	Bounty->AttemptCount++;

	// Update stats
	FMGBountyPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGBountyPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}
	Stats->TotalBountiesFailed++;

	// Break streak
	UpdateStreak(PlayerId, false);

	OnBountyFailed.Broadcast(PlayerId, BountyId, Reason);

	// Remove from active
	ActiveBounties->Remove(BountyId);

	// Update board
	FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (Board)
	{
		Board->ActiveBountyIds.Remove(BountyId);
	}
}

// Objective Tracking
void UMGBountySubsystem::CompleteObjective(const FString& PlayerId, const FString& BountyId, const FString& ObjectiveId)
{
	TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return;
	}

	FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty)
	{
		return;
	}

	bool* Progress = Bounty->ObjectiveProgress.Find(ObjectiveId);
	if (!Progress || *Progress)
	{
		return; // Already complete or doesn't exist
	}

	*Progress = true;
	Bounty->CompletedObjectives++;
	Bounty->Status = EMGBountyStatus::InProgress;

	OnBountyObjectiveCompleted.Broadcast(PlayerId, BountyId, ObjectiveId);

	// Calculate and broadcast progress
	float TotalProgress = GetBountyProgress(PlayerId, BountyId);
	OnBountyProgress.Broadcast(PlayerId, BountyId, TotalProgress);

	// Check if all required are complete
	if (AreAllRequiredObjectivesComplete(PlayerId, BountyId))
	{
		// Bounty can now be completed
		// Could auto-complete here or wait for explicit call
	}
}

bool UMGBountySubsystem::IsObjectiveComplete(const FString& PlayerId, const FString& BountyId, const FString& ObjectiveId) const
{
	const TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return false;
	}

	const FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty)
	{
		return false;
	}

	const bool* Progress = Bounty->ObjectiveProgress.Find(ObjectiveId);
	return Progress && *Progress;
}

float UMGBountySubsystem::GetBountyProgress(const FString& PlayerId, const FString& BountyId) const
{
	const TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return 0.0f;
	}

	const FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty || Bounty->TotalObjectives == 0)
	{
		return 0.0f;
	}

	return static_cast<float>(Bounty->CompletedObjectives) / static_cast<float>(Bounty->TotalObjectives);
}

bool UMGBountySubsystem::AreAllRequiredObjectivesComplete(const FString& PlayerId, const FString& BountyId) const
{
	const TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return false;
	}

	const FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId);
	if (!Bounty)
	{
		return false;
	}

	const FMGBountyDefinition* Definition = BountyDefinitions.Find(BountyId);
	if (!Definition)
	{
		return false;
	}

	for (const FString& Required : Definition->RequiredObjectives)
	{
		const bool* Complete = Bounty->ObjectiveProgress.Find(Required);
		if (!Complete || !*Complete)
		{
			return false;
		}
	}

	return true;
}

// Bounty Board
FMGPlayerBountyBoard UMGBountySubsystem::GetBountyBoard(const FString& PlayerId) const
{
	if (const FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId))
	{
		return *Board;
	}
	return FMGPlayerBountyBoard();
}

bool UMGBountySubsystem::RefreshBountyBoard(const FString& PlayerId, bool bForce)
{
	FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (!Board)
	{
		FMGPlayerBountyBoard NewBoard;
		NewBoard.PlayerId = PlayerId;
		NewBoard.MaxActiveBounties = 3;
		NewBoard.RefreshesRemaining = 3;
		NewBoard.RefreshCost = 1000;
		Board = &PlayerBountyBoards.Add(PlayerId, NewBoard);
	}

	if (!bForce)
	{
		if (Board->RefreshesRemaining <= 0)
		{
			return false;
		}
		Board->RefreshesRemaining--;
	}

	// Generate new bounties
	GenerateBountiesForPlayer(PlayerId);

	Board->LastRefreshTime = FDateTime::Now();

	OnBountyBoardRefreshed.Broadcast(PlayerId, Board->AvailableBountyIds.Num());
	return true;
}

TArray<FMGBountyDefinition> UMGBountySubsystem::GetAvailableBounties(const FString& PlayerId) const
{
	TArray<FMGBountyDefinition> Result;

	const FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (!Board)
	{
		return Result;
	}

	for (const FString& BountyId : Board->AvailableBountyIds)
	{
		if (const FMGBountyDefinition* Def = BountyDefinitions.Find(BountyId))
		{
			Result.Add(*Def);
		}
	}

	return Result;
}

TArray<FMGActiveBounty> UMGBountySubsystem::GetActiveBounties(const FString& PlayerId) const
{
	TArray<FMGActiveBounty> Result;

	const TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (!ActiveBounties)
	{
		return Result;
	}

	for (const auto& Pair : *ActiveBounties)
	{
		Result.Add(Pair.Value);
	}

	return Result;
}

bool UMGBountySubsystem::CanAcceptBounty(const FString& PlayerId, const FString& BountyId) const
{
	const FMGBountyDefinition* Definition = BountyDefinitions.Find(BountyId);
	if (!Definition)
	{
		return false;
	}

	// Check max active bounties
	const FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (Board && Board->ActiveBountyIds.Num() >= Board->MaxActiveBounties)
	{
		return false;
	}

	// Check if already active
	const TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId);
	if (ActiveBounties && ActiveBounties->Contains(BountyId))
	{
		return false;
	}

	// Check cooldown for repeatable bounties
	if (Definition->bIsRepeatable)
	{
		const TMap<FString, FDateTime>* Cooldowns = BountyCooldowns.Find(PlayerId);
		if (Cooldowns)
		{
			const FDateTime* CooldownEnd = Cooldowns->Find(BountyId);
			if (CooldownEnd && *CooldownEnd > FDateTime::Now())
			{
				return false;
			}
		}
	}
	else
	{
		// Non-repeatable, check if completed
		const TSet<FString>* Completed = CompletedBounties.Find(PlayerId);
		if (Completed && Completed->Contains(BountyId))
		{
			return false;
		}
	}

	// Check player level requirement via Career subsystem
	if (Definition->RequiredPlayerLevel > 1)
	{
		if (UGameInstance* GI = UGameplayStatics::GetGameInstance(GetWorld()))
		{
			if (UMGCareerSubsystem* CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>())
			{
				// Calculate player level from career chapter and progress
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

				if (PlayerLevel < Definition->RequiredPlayerLevel)
				{
					return false;
				}
			}
		}
	}

	return true;
}

// Queries
FMGBountyDefinition UMGBountySubsystem::GetBountyDefinition(const FString& BountyId) const
{
	if (const FMGBountyDefinition* Def = BountyDefinitions.Find(BountyId))
	{
		return *Def;
	}
	return FMGBountyDefinition();
}

FMGActiveBounty UMGBountySubsystem::GetActiveBounty(const FString& PlayerId, const FString& BountyId) const
{
	if (const TMap<FString, FMGActiveBounty>* ActiveBounties = PlayerActiveBounties.Find(PlayerId))
	{
		if (const FMGActiveBounty* Bounty = ActiveBounties->Find(BountyId))
		{
			return *Bounty;
		}
	}
	return FMGActiveBounty();
}

FMGBountyTarget UMGBountySubsystem::GetTarget(const FString& TargetId) const
{
	if (const FMGBountyTarget* Target = Targets.Find(TargetId))
	{
		return *Target;
	}
	return FMGBountyTarget();
}

TArray<FMGBountyDefinition> UMGBountySubsystem::GetBountiesForTarget(const FString& TargetId) const
{
	TArray<FMGBountyDefinition> Result;

	for (const auto& Pair : BountyDefinitions)
	{
		if (Pair.Value.TargetId == TargetId)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGBountyDefinition> UMGBountySubsystem::GetBountiesByType(EMGBountyType Type) const
{
	TArray<FMGBountyDefinition> Result;

	for (const auto& Pair : BountyDefinitions)
	{
		if (Pair.Value.Type == Type)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

TArray<FMGBountyDefinition> UMGBountySubsystem::GetBountiesByDifficulty(EMGBountyDifficulty Difficulty) const
{
	TArray<FMGBountyDefinition> Result;

	for (const auto& Pair : BountyDefinitions)
	{
		if (Pair.Value.Difficulty == Difficulty)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

// Community Bounties
void UMGBountySubsystem::RegisterCommunityBounty(const FMGCommunityBounty& CommunityBounty)
{
	if (!CommunityBounty.CommunityBountyId.IsEmpty())
	{
		CommunityBounties.Add(CommunityBounty.CommunityBountyId, CommunityBounty);
	}
}

TArray<FMGCommunityBounty> UMGBountySubsystem::GetActiveCommunityBounties() const
{
	TArray<FMGCommunityBounty> Result;
	FDateTime Now = FDateTime::Now();

	for (const auto& Pair : CommunityBounties)
	{
		if (Now >= Pair.Value.StartTime && Now <= Pair.Value.EndTime && !Pair.Value.bCompleted)
		{
			Result.Add(Pair.Value);
		}
	}

	return Result;
}

void UMGBountySubsystem::ContributeToCommunityBounty(const FString& CommunityBountyId)
{
	FMGCommunityBounty* Bounty = CommunityBounties.Find(CommunityBountyId);
	if (!Bounty || Bounty->bCompleted)
	{
		return;
	}

	Bounty->TotalContributors++;
	Bounty->TotalCompletions++;

	OnCommunityBountyProgress.Broadcast(CommunityBountyId, Bounty->TotalCompletions, Bounty->TargetCompletions);

	// Check milestones
	for (int32 i = 0; i < Bounty->MilestoneThresholds.Num(); i++)
	{
		if (Bounty->TotalCompletions >= Bounty->MilestoneThresholds[i] && Bounty->BonusRewardTier < i + 1)
		{
			Bounty->BonusRewardTier = i + 1;
			OnCommunityBountyMilestone.Broadcast(CommunityBountyId, Bounty->BonusRewardTier);
		}
	}

	// Check completion
	if (Bounty->TotalCompletions >= Bounty->TargetCompletions)
	{
		Bounty->bCompleted = true;
	}
}

FMGCommunityBounty UMGBountySubsystem::GetCommunityBounty(const FString& CommunityBountyId) const
{
	if (const FMGCommunityBounty* Bounty = CommunityBounties.Find(CommunityBountyId))
	{
		return *Bounty;
	}
	return FMGCommunityBounty();
}

// Stats
FMGBountyPlayerStats UMGBountySubsystem::GetPlayerStats(const FString& PlayerId) const
{
	if (const FMGBountyPlayerStats* Stats = PlayerStats.Find(PlayerId))
	{
		return *Stats;
	}
	return FMGBountyPlayerStats();
}

void UMGBountySubsystem::ResetPlayerStats(const FString& PlayerId)
{
	FMGBountyPlayerStats NewStats;
	NewStats.PlayerId = PlayerId;
	PlayerStats.Add(PlayerId, NewStats);
}

// Rewards
int32 UMGBountySubsystem::CalculateRewardCurrency(const FString& BountyId, float Multiplier) const
{
	const FMGBountyDefinition* Definition = BountyDefinitions.Find(BountyId);
	if (!Definition)
	{
		return 0;
	}

	return FMath::RoundToInt(Definition->RewardCurrency * Multiplier);
}

int32 UMGBountySubsystem::CalculateRewardExperience(const FString& BountyId, float Multiplier) const
{
	const FMGBountyDefinition* Definition = BountyDefinitions.Find(BountyId);
	if (!Definition)
	{
		return 0;
	}

	return FMath::RoundToInt(Definition->RewardExperience * Multiplier);
}

// Update
void UMGBountySubsystem::UpdateBountySystem(float MGDeltaTime)
{
	TickBounties(DeltaTime);
}

// Protected
void UMGBountySubsystem::TickBounties(float MGDeltaTime)
{
	CheckExpirations();
	UpdateBountyBoards();
}

void UMGBountySubsystem::CheckExpirations()
{
	FDateTime Now = FDateTime::Now();

	for (auto& PlayerPair : PlayerActiveBounties)
	{
		TArray<FString> ExpiredBounties;

		for (auto& BountyPair : PlayerPair.Value)
		{
			FMGActiveBounty& Bounty = BountyPair.Value;

			if (Bounty.Status == EMGBountyStatus::Accepted || Bounty.Status == EMGBountyStatus::InProgress)
			{
				// Check expiration
				if (Now >= Bounty.ExpirationTime)
				{
					ExpiredBounties.Add(BountyPair.Key);
				}
				else
				{
					// Check if expiring soon
					FTimespan TimeLeft = Bounty.ExpirationTime - Now;
					if (TimeLeft.GetTotalMinutes() <= 30)
					{
						OnBountyExpiring.Broadcast(Bounty.PlayerId, Bounty.BountyId, TimeLeft.GetTotalSeconds());
					}
				}
			}
		}

		for (const FString& BountyId : ExpiredBounties)
		{
			FMGActiveBounty* Bounty = PlayerPair.Value.Find(BountyId);
			if (Bounty)
			{
				Bounty->Status = EMGBountyStatus::Expired;
				FailBounty(PlayerPair.Key, BountyId, TEXT("Bounty expired"));
			}
		}
	}
}

void UMGBountySubsystem::UpdateBountyBoards()
{
	FDateTime Now = FDateTime::Now();

	for (auto& Pair : PlayerBountyBoards)
	{
		FMGPlayerBountyBoard& Board = Pair.Value;

		// Daily refresh of available bounties and refresh tokens
		FTimespan TimeSinceRefresh = Now - Board.LastRefreshTime;
		if (TimeSinceRefresh.GetTotalHours() >= 24.0)
		{
			Board.RefreshesRemaining = 3;
			RefreshBountyBoard(Pair.Key, true);
		}
	}
}

void UMGBountySubsystem::GenerateBountiesForPlayer(const FString& PlayerId)
{
	FMGPlayerBountyBoard* Board = PlayerBountyBoards.Find(PlayerId);
	if (!Board)
	{
		return;
	}

	Board->AvailableBountyIds = SelectRandomBounties(PlayerId, 5);
}

TArray<FString> UMGBountySubsystem::SelectRandomBounties(const FString& PlayerId, int32 Count)
{
	TArray<FString> Result;
	TArray<FString> EligibleBounties;

	// Collect eligible bounties
	for (const auto& Pair : BountyDefinitions)
	{
		if (CanAcceptBounty(PlayerId, Pair.Key))
		{
			EligibleBounties.Add(Pair.Key);
		}
	}

	// Randomly select
	while (Result.Num() < Count && EligibleBounties.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, EligibleBounties.Num() - 1);
		Result.Add(EligibleBounties[Index]);
		EligibleBounties.RemoveAt(Index);
	}

	return Result;
}

void UMGBountySubsystem::UpdatePlayerStats(const FString& PlayerId, const FMGBountyCompletionResult& Result)
{
	FMGBountyPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGBountyPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	if (Result.bSuccess)
	{
		Stats->TotalBountiesCompleted++;
		Stats->TotalCurrencyEarned += Result.CurrencyEarned;
		Stats->TotalExperienceEarned += Result.ExperienceEarned;
		Stats->TotalReputationEarned += Result.ReputationEarned;

		if (Result.bPerfectCompletion)
		{
			Stats->PerfectCompletions++;
		}

		if (Stats->FastestBountyTime == 0.0f || Result.CompletionTime < Stats->FastestBountyTime)
		{
			Stats->FastestBountyTime = Result.CompletionTime;
		}

		// Update by type
		const FMGBountyDefinition* Def = BountyDefinitions.Find(Result.BountyId);
		if (Def)
		{
			int32& TypeCount = Stats->CompletionsByType.FindOrAdd(Def->Type);
			TypeCount++;

			int32& DiffCount = Stats->CompletionsByDifficulty.FindOrAdd(Def->Difficulty);
			DiffCount++;

			if (!Def->TargetId.IsEmpty())
			{
				int32& TargetCount = Stats->TargetCaptureCount.FindOrAdd(Def->TargetId);
				TargetCount++;

				// Update most captured
				if (Stats->MostCapturedTargetId.IsEmpty())
				{
					Stats->MostCapturedTargetId = Def->TargetId;
				}
				else
				{
					int32* CurrentMostCount = Stats->TargetCaptureCount.Find(Stats->MostCapturedTargetId);
					if (!CurrentMostCount || TargetCount > *CurrentMostCount)
					{
						Stats->MostCapturedTargetId = Def->TargetId;
					}
				}
			}
		}
	}
}

void UMGBountySubsystem::UpdateStreak(const FString& PlayerId, bool bSuccess)
{
	FMGBountyPlayerStats* Stats = PlayerStats.Find(PlayerId);
	if (!Stats)
	{
		FMGBountyPlayerStats NewStats;
		NewStats.PlayerId = PlayerId;
		Stats = &PlayerStats.Add(PlayerId, NewStats);
	}

	bool bNewRecord = false;

	if (bSuccess)
	{
		Stats->CurrentStreak++;
		if (Stats->CurrentStreak > Stats->BestStreak)
		{
			Stats->BestStreak = Stats->CurrentStreak;
			bNewRecord = true;
		}
	}
	else
	{
		Stats->CurrentStreak = 0;
	}

	OnBountyStreakUpdated.Broadcast(PlayerId, Stats->CurrentStreak, bNewRecord);
}

FString UMGBountySubsystem::GenerateInstanceId() const
{
	return FString::Printf(TEXT("BOUNTY_INST_%d_%lld"), ++const_cast<UMGBountySubsystem*>(this)->InstanceCounter, FDateTime::Now().GetTicks());
}

float UMGBountySubsystem::GetDifficultyMultiplier(EMGBountyDifficulty Difficulty) const
{
	switch (Difficulty)
	{
		case EMGBountyDifficulty::Easy: return 1.0f;
		case EMGBountyDifficulty::Medium: return 1.25f;
		case EMGBountyDifficulty::Hard: return 1.5f;
		case EMGBountyDifficulty::Expert: return 2.0f;
		case EMGBountyDifficulty::Legendary: return 3.0f;
		case EMGBountyDifficulty::Impossible: return 5.0f;
		default: return 1.0f;
	}
}

// Persistence
void UMGBountySubsystem::SaveBountyData()
{
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("Bounty");
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	if (!PlatformFile.DirectoryExists(*SaveDir))
	{
		PlatformFile.CreateDirectory(*SaveDir);
	}
	FString FilePath = SaveDir / TEXT("bounty_data.dat");

	FBufferArchive SaveArchive;

	// Version for future compatibility
	int32 Version = 1;
	SaveArchive << Version;

	// Save player stats
	int32 NumStats = PlayerStats.Num();
	SaveArchive << NumStats;

	for (const auto& StatPair : PlayerStats)
	{
		FString PlayerId = StatPair.Key;
		SaveArchive << PlayerId;

		const FMGBountyPlayerStats& Stats = StatPair.Value;
		int32 TotalCompleted = Stats.TotalBountiesCompleted;
		int32 TotalFailed = Stats.TotalBountiesFailed;
		int32 TotalAbandoned = Stats.TotalBountiesAbandoned;
		int32 PerfectCompletions = Stats.PerfectCompletions;
		int64 TotalCurrency = Stats.TotalCurrencyEarned;
		int64 TotalXP = Stats.TotalExperienceEarned;
		int64 TotalRep = Stats.TotalReputationEarned;
		int32 CurrentStreak = Stats.CurrentStreak;
		int32 BestStreak = Stats.BestStreak;
		float FastestTime = Stats.FastestBountyTime;

		SaveArchive << TotalCompleted;
		SaveArchive << TotalFailed;
		SaveArchive << TotalAbandoned;
		SaveArchive << PerfectCompletions;
		SaveArchive << TotalCurrency;
		SaveArchive << TotalXP;
		SaveArchive << TotalRep;
		SaveArchive << CurrentStreak;
		SaveArchive << BestStreak;
		SaveArchive << FastestTime;

		// Save completions by type
		int32 NumTypes = Stats.CompletionsByType.Num();
		SaveArchive << NumTypes;
		for (const auto& TypePair : Stats.CompletionsByType)
		{
			int32 TypeInt = static_cast<int32>(TypePair.Key);
			int32 Count = TypePair.Value;
			SaveArchive << TypeInt;
			SaveArchive << Count;
		}

		// Save completions by difficulty
		int32 NumDiffs = Stats.CompletionsByDifficulty.Num();
		SaveArchive << NumDiffs;
		for (const auto& DiffPair : Stats.CompletionsByDifficulty)
		{
			int32 DiffInt = static_cast<int32>(DiffPair.Key);
			int32 Count = DiffPair.Value;
			SaveArchive << DiffInt;
			SaveArchive << Count;
		}

		// Save target capture counts
		int32 NumTargets = Stats.TargetCaptureCount.Num();
		SaveArchive << NumTargets;
		for (const auto& TargetPair : Stats.TargetCaptureCount)
		{
			FString TargetId = TargetPair.Key;
			int32 Count = TargetPair.Value;
			SaveArchive << TargetId;
			SaveArchive << Count;
		}
	}

	// Save completed bounties per player
	int32 NumCompletedPlayers = CompletedBounties.Num();
	SaveArchive << NumCompletedPlayers;

	for (const auto& CompletedPair : CompletedBounties)
	{
		FString PlayerId = CompletedPair.Key;
		SaveArchive << PlayerId;

		int32 NumCompleted = CompletedPair.Value.Num();
		SaveArchive << NumCompleted;

		for (const FString& BountyId : CompletedPair.Value)
		{
			FString Id = BountyId;
			SaveArchive << Id;
		}
	}

	// Write to file
	if (SaveArchive.Num() > 0)
	{
		FFileHelper::SaveArrayToFile(SaveArchive, *FilePath);
	}

	UE_LOG(LogTemp, Log, TEXT("MGBountySubsystem: Saved bounty data for %d players"), NumStats);
}

void UMGBountySubsystem::LoadBountyData()
{
	FString FilePath = FPaths::ProjectSavedDir() / TEXT("Bounty") / TEXT("bounty_data.dat");

	TArray<uint8> LoadData;
	if (!FFileHelper::LoadFileToArray(LoadData, *FilePath))
	{
		UE_LOG(LogTemp, Log, TEXT("MGBountySubsystem: No saved bounty data found"));
		return;
	}

	FMemoryReader LoadArchive(LoadData, true);

	int32 Version;
	LoadArchive << Version;

	if (Version != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("MGBountySubsystem: Unknown save version %d"), Version);
		return;
	}

	// Load player stats
	int32 NumStats;
	LoadArchive << NumStats;

	for (int32 i = 0; i < NumStats; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		FMGBountyPlayerStats Stats;
		Stats.PlayerId = PlayerId;

		LoadArchive << Stats.TotalBountiesCompleted;
		LoadArchive << Stats.TotalBountiesFailed;
		LoadArchive << Stats.TotalBountiesAbandoned;
		LoadArchive << Stats.PerfectCompletions;
		LoadArchive << Stats.TotalCurrencyEarned;
		LoadArchive << Stats.TotalExperienceEarned;
		LoadArchive << Stats.TotalReputationEarned;
		LoadArchive << Stats.CurrentStreak;
		LoadArchive << Stats.BestStreak;
		LoadArchive << Stats.FastestBountyTime;

		// Load completions by type
		int32 NumTypes;
		LoadArchive << NumTypes;
		for (int32 j = 0; j < NumTypes; ++j)
		{
			int32 TypeInt;
			int32 Count;
			LoadArchive << TypeInt;
			LoadArchive << Count;
			Stats.CompletionsByType.Add(static_cast<EMGBountyType>(TypeInt), Count);
		}

		// Load completions by difficulty
		int32 NumDiffs;
		LoadArchive << NumDiffs;
		for (int32 j = 0; j < NumDiffs; ++j)
		{
			int32 DiffInt;
			int32 Count;
			LoadArchive << DiffInt;
			LoadArchive << Count;
			Stats.CompletionsByDifficulty.Add(static_cast<EMGBountyDifficulty>(DiffInt), Count);
		}

		// Load target capture counts
		int32 NumTargets;
		LoadArchive << NumTargets;
		for (int32 j = 0; j < NumTargets; ++j)
		{
			FString TargetId;
			int32 Count;
			LoadArchive << TargetId;
			LoadArchive << Count;
			Stats.TargetCaptureCount.Add(TargetId, Count);
		}

		PlayerStats.Add(PlayerId, Stats);
	}

	// Load completed bounties per player
	int32 NumCompletedPlayers;
	LoadArchive << NumCompletedPlayers;

	for (int32 i = 0; i < NumCompletedPlayers; ++i)
	{
		FString PlayerId;
		LoadArchive << PlayerId;

		int32 NumCompleted;
		LoadArchive << NumCompleted;

		TSet<FString> CompletedSet;
		for (int32 j = 0; j < NumCompleted; ++j)
		{
			FString BountyId;
			LoadArchive << BountyId;
			CompletedSet.Add(BountyId);
		}

		CompletedBounties.Add(PlayerId, CompletedSet);
	}

	UE_LOG(LogTemp, Log, TEXT("MGBountySubsystem: Loaded bounty data for %d players"), NumStats);
}

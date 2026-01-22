// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/MGRaceFlowManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"

void UMGRaceFlowManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CacheSubsystems();
}

void UMGRaceFlowManager::Deinitialize()
{
	Super::Deinitialize();
}

// ==========================================
// RACE FLOW CONTROL
// ==========================================

void UMGRaceFlowManager::BeginRaceLoad(const FSoftObjectPath& TrackMapPath, const FMGRaceConfig& RaceConfig, FName PlayerVehicleID)
{
	if (CurrentState != EMGRaceFlowState::Idle && CurrentState != EMGRaceFlowState::PostRace)
	{
		return;
	}

	// Store config
	CurrentConfig = RaceConfig;
	CurrentPlayerVehicleID = PlayerVehicleID;
	PendingTrackPath = TrackMapPath;
	RestartCount = 0;

	// Show loading screen
	ShowLoadingScreen();

	SetFlowState(EMGRaceFlowState::LoadingTrack);

	// Begin async level load
	FString LevelName = TrackMapPath.GetAssetName();

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = FName(TEXT("OnTrackLoaded"));
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = FMath::Rand();

	UGameplayStatics::LoadStreamLevelBySoftObjectPtr(
		GetWorld(),
		TSoftObjectPtr<UWorld>(TrackMapPath),
		true, // Make visible
		false, // Should block
		LatentInfo
	);
}

void UMGRaceFlowManager::OnTrackLoaded()
{
	if (CurrentState != EMGRaceFlowState::LoadingTrack)
	{
		return;
	}

	// Hide loading screen
	HideLoadingScreen();

	SetFlowState(EMGRaceFlowState::PreRace);

	// Race game mode will handle the pre-race -> countdown -> racing flow
}

void UMGRaceFlowManager::OnRaceFinished(const FMGRaceResults& Results)
{
	if (CurrentState != EMGRaceFlowState::Racing && CurrentState != EMGRaceFlowState::PreRace)
	{
		// Could be from game mode, update state
	}

	CachedResults = Results;
	SetFlowState(EMGRaceFlowState::RaceEnding);

	// Build summary and show results
	BuildPostRaceSummary(Results);

	SetFlowState(EMGRaceFlowState::ShowingResults);
	ShowResultsWidget();
}

void UMGRaceFlowManager::ConfirmResults()
{
	if (CurrentState != EMGRaceFlowState::ShowingResults)
	{
		return;
	}

	SetFlowState(EMGRaceFlowState::ProcessingRewards);

	// Apply rewards
	ApplyRewards(PostRaceSummary.Rewards);

	// Apply unlocks
	ApplyUnlocks(PostRaceSummary.Unlocks);

	// Submit to leaderboards
	SubmitToLeaderboards(CachedResults);

	// Update rivals
	UpdateRivalRelationships(CachedResults);

	// Broadcast completion
	OnRewardsProcessed.Broadcast(PostRaceSummary);

	// Show unlocks if any
	if (PostRaceSummary.Unlocks.Num() > 0)
	{
		SetFlowState(EMGRaceFlowState::ShowingUnlocks);
	}
	else
	{
		SetFlowState(EMGRaceFlowState::PostRace);
	}
}

void UMGRaceFlowManager::SkipToPostRace()
{
	if (CurrentState == EMGRaceFlowState::ShowingUnlocks)
	{
		SetFlowState(EMGRaceFlowState::PostRace);
	}
}

void UMGRaceFlowManager::ExitRace(FName Destination)
{
	ExitDestination = Destination;
	SetFlowState(EMGRaceFlowState::Exiting);

	// Determine where to go
	FString LevelToOpen;
	if (Destination == FName(TEXT("Garage")))
	{
		LevelToOpen = TEXT("/Game/Maps/Garage");
	}
	else if (Destination == FName(TEXT("Career")))
	{
		LevelToOpen = TEXT("/Game/Maps/CareerHub");
	}
	else
	{
		LevelToOpen = TEXT("/Game/Maps/MainMenu");
	}

	UGameplayStatics::OpenLevel(GetWorld(), FName(*LevelToOpen));

	SetFlowState(EMGRaceFlowState::Idle);
	OnFlowComplete.Broadcast();
}

void UMGRaceFlowManager::QuickRestart()
{
	if (!CanRestart())
	{
		return;
	}

	RestartCount++;

	// Get race game mode and restart
	if (UWorld* World = GetWorld())
	{
		if (AMGRaceGameMode* GameMode = Cast<AMGRaceGameMode>(World->GetAuthGameMode()))
		{
			GameMode->RestartRace();
			SetFlowState(EMGRaceFlowState::PreRace);
		}
	}
}

// ==========================================
// STATE QUERIES
// ==========================================

bool UMGRaceFlowManager::IsRaceInProgress() const
{
	return CurrentState == EMGRaceFlowState::PreRace ||
		CurrentState == EMGRaceFlowState::Countdown ||
		CurrentState == EMGRaceFlowState::Racing;
}

bool UMGRaceFlowManager::CanRestart() const
{
	// Can restart during race or at results
	return CurrentState == EMGRaceFlowState::Racing ||
		CurrentState == EMGRaceFlowState::ShowingResults ||
		CurrentState == EMGRaceFlowState::PostRace;
}

// ==========================================
// INTERNAL METHODS
// ==========================================

void UMGRaceFlowManager::SetFlowState(EMGRaceFlowState NewState)
{
	if (CurrentState != NewState)
	{
		CurrentState = NewState;
		OnFlowStateChanged.Broadcast(NewState);
	}
}

FMGRaceRewardBreakdown UMGRaceFlowManager::CalculateRewards(const FMGRaceResults& Results)
{
	FMGRaceRewardBreakdown Breakdown;

	// Find player result
	int32 PlayerPosition = 1;
	float PlayerDriftScore = 0.0f;
	bool bHadBestLap = false;
	bool bCleanRace = true; // Assume clean unless collision data says otherwise

	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (!Racer.bIsAI)
		{
			PlayerPosition = Racer.Position;
			PlayerDriftScore = Racer.DriftScore;
			bHadBestLap = (Racer.BestLapTime > 0.0f && Racer.BestLapTime == Results.BestLapTime);
			break;
		}
	}

	// Base credits by position
	int32 PosIndex = FMath::Clamp(PlayerPosition - 1, 0, 7);
	Breakdown.BaseCredits = BaseCredits[PosIndex];
	Breakdown.BonusDescriptions.Add(FText::Format(
		NSLOCTEXT("MG", "PositionReward", "{0} Place"),
		FText::AsNumber(PlayerPosition)
	));

	// Lap bonus (extra for races > 3 laps)
	if (Results.Config.LapCount > 3)
	{
		Breakdown.LapBonus = (Results.Config.LapCount - 3) * 500;
		Breakdown.BonusDescriptions.Add(FText::Format(
			NSLOCTEXT("MG", "LapBonus", "+{0} Lap Bonus"),
			FText::AsNumber(Results.Config.LapCount)
		));
	}

	// Difficulty bonus
	if (Results.Config.AIDifficulty > 0.5f)
	{
		Breakdown.DifficultyBonus = static_cast<int64>((Results.Config.AIDifficulty - 0.5f) * 4000);
		Breakdown.BonusDescriptions.Add(NSLOCTEXT("MG", "DifficultyBonus", "Hard Mode Bonus"));
	}

	// Clean race bonus
	if (bCleanRace)
	{
		Breakdown.CleanRaceBonus = 1500;
		Breakdown.BonusDescriptions.Add(NSLOCTEXT("MG", "CleanRace", "Clean Race"));
	}

	// Drift bonus
	if (PlayerDriftScore > 1000.0f)
	{
		Breakdown.DriftBonus = static_cast<int64>(PlayerDriftScore * 0.5f);
		Breakdown.BonusDescriptions.Add(FText::Format(
			NSLOCTEXT("MG", "DriftBonus", "Drift Score: {0}"),
			FText::AsNumber(static_cast<int32>(PlayerDriftScore))
		));
	}

	// Best lap bonus
	if (bHadBestLap)
	{
		Breakdown.BestLapBonus = 2000;
		Breakdown.BonusDescriptions.Add(NSLOCTEXT("MG", "BestLap", "Fastest Lap"));
	}

	// Pink slip bonus
	if (Results.Config.bPinkSlipRace && PlayerPosition == 1)
	{
		Breakdown.PinkSlipBonus = 25000; // Value of opponent's car
		Breakdown.BonusDescriptions.Add(NSLOCTEXT("MG", "PinkSlip", "PINK SLIP WIN"));
	}

	// Modifier multiplier (from race modifiers if any)
	Breakdown.ModifierMultiplier = 1.0f; // Can be adjusted by active modifiers

	// Calculate XP
	Breakdown.XPEarned = BaseXP[PosIndex];
	if (PlayerPosition == 1) Breakdown.XPEarned += 100; // Win bonus
	if (bHadBestLap) Breakdown.XPEarned += 50;

	// Calculate reputation
	Breakdown.ReputationEarned = BaseReputation[PosIndex];
	if (Results.Config.bPinkSlipRace && PlayerPosition == 1)
	{
		Breakdown.ReputationEarned += 150; // Big rep for pink slip win
	}

	// Calculate total
	Breakdown.CalculateTotal();

	return Breakdown;
}

TArray<FMGRaceUnlock> UMGRaceFlowManager::CheckForUnlocks(const FMGRaceResults& Results)
{
	TArray<FMGRaceUnlock> Unlocks;

	// Check progression subsystem for new unlocks
	if (ProgressionSubsystem.IsValid())
	{
		// Query for any new unlocks earned based on new XP/level
		// This would check vehicle unlocks, part unlocks, customization unlocks, etc.
	}

	// Check for first-time track completion
	// Check for position-based unlocks (first 1st place, etc.)
	// Check for challenge-based unlocks

	// Example unlock for winning first race
	static bool bFirstWinAwarded = false;
	if (!bFirstWinAwarded && Results.bPlayerWon)
	{
		FMGRaceUnlock WinUnlock;
		WinUnlock.UnlockType = FName(TEXT("Achievement"));
		WinUnlock.ItemID = FName(TEXT("FirstVictory"));
		WinUnlock.DisplayName = NSLOCTEXT("MG", "FirstWin", "FIRST BLOOD");
		WinUnlock.Description = NSLOCTEXT("MG", "FirstWinDesc", "Won your first race");
		WinUnlock.Rarity = 1;
		Unlocks.Add(WinUnlock);
		bFirstWinAwarded = true;
	}

	return Unlocks;
}

TArray<FText> UMGRaceFlowManager::UpdateChallengeProgress(const FMGRaceResults& Results)
{
	TArray<FText> Progress;

	// Update daily/weekly challenges
	// Examples:
	// - "Complete 5 races" -> 3/5
	// - "Win a race without using NOS" -> Complete!
	// - "Drift 10,000 points" -> 8,500/10,000

	// This would integrate with the live events/challenges subsystem

	return Progress;
}

void UMGRaceFlowManager::ApplyRewards(const FMGRaceRewardBreakdown& Rewards)
{
	// Apply credits
	if (EconomySubsystem.IsValid())
	{
		// EconomySubsystem->AddCredits(Rewards.TotalCredits, TEXT("Race Reward"));
	}

	// Apply XP
	if (ProgressionSubsystem.IsValid())
	{
		// ProgressionSubsystem->AddXP(Rewards.XPEarned);
	}

	// Apply reputation
	if (CareerSubsystem.IsValid())
	{
		// CareerSubsystem->AddReputation(Rewards.ReputationEarned);
	}
}

void UMGRaceFlowManager::ApplyUnlocks(const TArray<FMGRaceUnlock>& Unlocks)
{
	for (const FMGRaceUnlock& Unlock : Unlocks)
	{
		// Apply unlock based on type
		if (Unlock.UnlockType == FName(TEXT("Vehicle")))
		{
			// Unlock vehicle in garage
		}
		else if (Unlock.UnlockType == FName(TEXT("Part")))
		{
			// Add part to inventory
		}
		else if (Unlock.UnlockType == FName(TEXT("Customization")))
		{
			// Unlock paint/vinyl/etc.
		}
		else if (Unlock.UnlockType == FName(TEXT("Achievement")))
		{
			// Mark achievement complete
		}
	}
}

void UMGRaceFlowManager::SubmitToLeaderboards(const FMGRaceResults& Results)
{
	if (!LeaderboardSubsystem.IsValid())
	{
		return;
	}

	// Find player's best lap and submit
	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (!Racer.bIsAI && Racer.BestLapTime > 0.0f)
		{
			// LeaderboardSubsystem->SubmitLapTime(
			//     CurrentConfig.TrackID,
			//     CurrentPlayerVehicleID,
			//     Racer.BestLapTime
			// );
			break;
		}
	}
}

void UMGRaceFlowManager::UpdateRivalRelationships(const FMGRaceResults& Results)
{
	// Check if racing against a rival
	// Update rival stats based on outcome
	// Potentially create new rivalries based on close finishes
}

void UMGRaceFlowManager::CacheSubsystems()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		// ProgressionSubsystem = GI->GetSubsystem<UMGProgressionSubsystem>();
		// EconomySubsystem = GI->GetSubsystem<UMGEconomySubsystem>();
		// CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>();
		// LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>();
	}
}

void UMGRaceFlowManager::ShowLoadingScreen()
{
	// Create and show loading screen widget
	// LoadingScreen = CreateWidget<UMGLoadingScreenWidget>(GetWorld());
	// if (LoadingScreen.IsValid())
	// {
	//     LoadingScreen->SetContext(EMGLoadingContext::Race);
	//     LoadingScreen->AddToViewport(100);
	// }
}

void UMGRaceFlowManager::HideLoadingScreen()
{
	if (LoadingScreen.IsValid())
	{
		LoadingScreen->RemoveFromParent();
		LoadingScreen.Reset();
	}
}

void UMGRaceFlowManager::ShowResultsWidget()
{
	// Create and show results widget
	// ResultsWidget = CreateWidget<UMGRaceResultsWidget>(GetWorld());
	// if (ResultsWidget.IsValid())
	// {
	//     ResultsWidget->ShowResults(PostRaceSummary);
	//     ResultsWidget->AddToViewport(90);
	// }
}

void UMGRaceFlowManager::BuildPostRaceSummary(const FMGRaceResults& Results)
{
	PostRaceSummary = FMGPostRaceSummary();
	PostRaceSummary.RaceResults = Results;

	// Calculate rewards
	PostRaceSummary.Rewards = CalculateRewards(Results);

	// Check for unlocks
	PostRaceSummary.Unlocks = CheckForUnlocks(Results);

	// Update challenge progress
	PostRaceSummary.ChallengeProgress = UpdateChallengeProgress(Results);

	// Get current level info (would come from progression subsystem)
	PostRaceSummary.LevelBefore = 1;
	PostRaceSummary.XPProgressBefore = 0.0f;

	// Calculate new level after XP
	// This would use the progression subsystem
	PostRaceSummary.LevelAfter = PostRaceSummary.LevelBefore;
	PostRaceSummary.XPProgressAfter = PostRaceSummary.XPProgressBefore;
	PostRaceSummary.bLeveledUp = PostRaceSummary.LevelAfter > PostRaceSummary.LevelBefore;

	// Check for records
	// PostRaceSummary.bNewPersonalBest = LeaderboardSubsystem->IsNewPersonalBest(...);
	// PostRaceSummary.bTrackRecord = LeaderboardSubsystem->IsTrackRecord(...);
}

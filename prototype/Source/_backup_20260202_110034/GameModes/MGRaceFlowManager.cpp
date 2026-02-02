// Copyright Midnight Grind. All Rights Reserved.

#include "GameModes/MGRaceFlowManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/StreamableManager.h"
#include "Engine/AssetManager.h"
#include "Progression/MGPlayerProgression.h"
#include "Career/MGCareerSubsystem.h"
#include "Social/MGLeaderboardSubsystem.h"
#include "Economy/MGEconomySubsystem.h"
#include "Rivals/MGRivalsSubsystem.h"
#include "DynamicDifficulty/MGDynamicDifficultySubsystem.h"

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

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::LoadStreamLevelBySoftObjectPtr(
		World,
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

	// Check collision count from DynamicDifficultySubsystem for clean race detection
	bool bCleanRace = false;
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGDynamicDifficultySubsystem* DifficultySubsystem = GI->GetSubsystem<UMGDynamicDifficultySubsystem>())
		{
			bCleanRace = (DifficultySubsystem->GetRaceCollisionCount() == 0);
		}
	}

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
	// Apply credits via EconomySubsystem (integrates with save system)
	if (UMGEconomySubsystem* EconSub = GetGameInstance()->GetSubsystem<UMGEconomySubsystem>())
	{
		EconSub->AwardRaceWinnings(Rewards.TotalCredits, CurrentConfig.TrackName);
	}

	// Apply XP via PlayerProgression
	if (ProgressionSubsystem.IsValid())
	{
		ProgressionSubsystem->AddXP(Rewards.XPEarned, true);
	}

	// Apply reputation via PlayerProgression (reputation is earned, not purchased)
	if (ProgressionSubsystem.IsValid() && Rewards.ReputationEarned > 0)
	{
		// Get the crew from current race config or default to Midnight
		EMGCrew RaceCrew = EMGCrew::Midnight; // Could be from CurrentConfig
		ProgressionSubsystem->AddCrewReputation(RaceCrew, Rewards.ReputationEarned);
	}

	// Record race result in career system
	if (CareerSubsystem.IsValid())
	{
		// Find player position from cached results
		int32 PlayerPosition = 1;
		int32 TotalRacers = CachedResults.RacerResults.Num();
		TArray<FString> DefeatedRivals;

		for (const FMGRacerData& Racer : CachedResults.RacerResults)
		{
			if (!Racer.bIsAI)
			{
				PlayerPosition = Racer.Position;
				break;
			}
		}

		// Clean race determined by CalculateRewards() based on collision count
		bool bCleanRace = Rewards.CleanRaceBonus > 0;

		CareerSubsystem->OnRaceCompleted(PlayerPosition, TotalRacers, bCleanRace, DefeatedRivals);
	}

	// Record race statistics
	if (ProgressionSubsystem.IsValid())
	{
		int32 PlayerPosition = 1;
		int32 TotalRacers = CachedResults.RacerResults.Num();

		for (const FMGRacerData& Racer : CachedResults.RacerResults)
		{
			if (!Racer.bIsAI)
			{
				PlayerPosition = Racer.Position;

				// Record drift score if any
				if (Racer.DriftScore > 0.0f)
				{
					ProgressionSubsystem->RecordDriftScore(Racer.DriftScore);
				}
				break;
			}
		}

		// Determine race type from config
		FName RaceTypeID;
		switch (CurrentConfig.RaceType)
		{
		case EMGRaceType::Circuit:   RaceTypeID = FName(TEXT("Circuit")); break;
		case EMGRaceType::Sprint:    RaceTypeID = FName(TEXT("Sprint")); break;
		case EMGRaceType::Drift:     RaceTypeID = FName(TEXT("Drift")); break;
		case EMGRaceType::TimeTrial: RaceTypeID = FName(TEXT("TimeTrial")); break;
		case EMGRaceType::Drag:      RaceTypeID = FName(TEXT("Drag")); break;
		case EMGRaceType::PinkSlip:  RaceTypeID = FName(TEXT("PinkSlip")); break;
		default:                     RaceTypeID = FName(TEXT("Circuit")); break;
		}

		ProgressionSubsystem->RecordRaceResult(PlayerPosition, TotalRacers, EMGCrew::None, RaceTypeID);
	}
}

void UMGRaceFlowManager::ApplyUnlocks(const TArray<FMGRaceUnlock>& Unlocks)
{
	if (!ProgressionSubsystem.IsValid())
	{
		return;
	}

	for (const FMGRaceUnlock& Unlock : Unlocks)
	{
		// Create unlock entry for progression system
		FMGUnlock ProgressionUnlock;
		ProgressionUnlock.UnlockID = Unlock.ItemID;
		ProgressionUnlock.DisplayName = Unlock.DisplayName;
		ProgressionUnlock.Description = Unlock.Description;
		ProgressionUnlock.UnlockedAt = FDateTime::Now();

		// Map unlock type
		if (Unlock.UnlockType == FName(TEXT("Vehicle")))
		{
			ProgressionUnlock.Type = EMGUnlockType::Vehicle;
		}
		else if (Unlock.UnlockType == FName(TEXT("Part")))
		{
			ProgressionUnlock.Type = EMGUnlockType::Part;
		}
		else if (Unlock.UnlockType == FName(TEXT("Customization")))
		{
			ProgressionUnlock.Type = EMGUnlockType::Cosmetic;
		}
		else if (Unlock.UnlockType == FName(TEXT("Track")))
		{
			ProgressionUnlock.Type = EMGUnlockType::Track;
		}
		else if (Unlock.UnlockType == FName(TEXT("Achievement")))
		{
			ProgressionUnlock.Type = EMGUnlockType::Feature;
		}
		else
		{
			ProgressionUnlock.Type = EMGUnlockType::Feature;
		}

		// Grant the unlock via progression system
		ProgressionSubsystem->GrantUnlock(ProgressionUnlock);
	}
}

void UMGRaceFlowManager::SubmitToLeaderboards(const FMGRaceResults& Results)
{
	if (!LeaderboardSubsystem.IsValid())
	{
		return;
	}

	// Find player's data and submit to leaderboards
	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (!Racer.bIsAI)
		{
			// Submit best lap time
			if (Racer.BestLapTime > 0.0f)
			{
				TArray<uint8> GhostData; // Empty for now - ghost replay data would go here
				LeaderboardSubsystem->SubmitLapTime(
					CurrentConfig.TrackName,
					Racer.BestLapTime,
					CurrentPlayerVehicleID,
					GhostData
				);
			}

			// Submit total race time
			if (Racer.TotalTime > 0.0f)
			{
				LeaderboardSubsystem->SubmitRaceTime(
					CurrentConfig.TrackName,
					Racer.TotalTime,
					CurrentConfig.LapCount,
					CurrentPlayerVehicleID
				);
			}

			break;
		}
	}
}

void UMGRaceFlowManager::UpdateRivalRelationships(const FMGRaceResults& Results)
{
	// Get rivals subsystem
	UMGRivalsSubsystem* RivalsSubsystem = nullptr;
	if (UGameInstance* GI = GetGameInstance())
	{
		RivalsSubsystem = GI->GetSubsystem<UMGRivalsSubsystem>();
	}

	if (!RivalsSubsystem)
	{
		return;
	}

	// Find player data
	int32 PlayerPosition = 0;
	float PlayerFinishTime = 0.0f;
	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (!Racer.bIsAI)
		{
			PlayerPosition = Racer.Position;
			PlayerFinishTime = Racer.FinishTime;
			break;
		}
	}

	if (PlayerPosition <= 0)
	{
		return;
	}

	// Process each AI racer as potential rival encounter
	for (const FMGRacerData& Racer : Results.RacerResults)
	{
		if (Racer.bIsAI && Racer.bFinished)
		{
			// Calculate time difference (positive = player faster)
			float TimeDiff = Racer.FinishTime - PlayerFinishTime;

			// Create rivalry encounter for close races
			FString AIPlayerID = FString::Printf(TEXT("AI_%d"), Racer.RacerIndex);
			RivalsSubsystem->OnRaceWithPlayer(
				AIPlayerID,
				Racer.DisplayName,
				PlayerPosition,
				Racer.Position,
				TimeDiff
			);
		}
	}

	// Check for defeated rival and update post-race summary
	if (Results.bPlayerWon)
	{
		FMGRival Nemesis = RivalsSubsystem->GetNemesis();
		if (RivalsSubsystem->HasNemesis())
		{
			PostRaceSummary.RivalDefeated = FText::Format(
				NSLOCTEXT("MG", "RivalDefeated", "Defeated rival: {0}"),
				Nemesis.DisplayName
			);
		}
	}
}

void UMGRaceFlowManager::CacheSubsystems()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		ProgressionSubsystem = GI->GetSubsystem<UMGPlayerProgression>();
		CareerSubsystem = GI->GetSubsystem<UMGCareerSubsystem>();
		LeaderboardSubsystem = GI->GetSubsystem<UMGLeaderboardSubsystem>();
		// Note: Economy handled via ShopSubsystem->AddCurrency()
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

	// Get current level info from progression subsystem
	if (ProgressionSubsystem.IsValid())
	{
		PostRaceSummary.LevelBefore = ProgressionSubsystem->GetCurrentLevel();
		PostRaceSummary.XPProgressBefore = ProgressionSubsystem->GetLevelProgress();

		// Calculate what level will be after XP is applied
		int64 CurrentXP = ProgressionSubsystem->GetCurrentXP();
		int64 XPAfter = CurrentXP + PostRaceSummary.Rewards.XPEarned;
		int64 XPForNext = ProgressionSubsystem->GetXPForNextLevel();

		// Guard against division by zero
		if (XPForNext <= 0)
		{
			XPForNext = 1;
		}

		if (XPAfter >= XPForNext)
		{
			PostRaceSummary.LevelAfter = PostRaceSummary.LevelBefore + 1;
			PostRaceSummary.XPProgressAfter = static_cast<float>(XPAfter - XPForNext) / static_cast<float>(XPForNext);
			PostRaceSummary.bLeveledUp = true;
		}
		else
		{
			PostRaceSummary.LevelAfter = PostRaceSummary.LevelBefore;
			PostRaceSummary.XPProgressAfter = static_cast<float>(XPAfter) / static_cast<float>(XPForNext);
			PostRaceSummary.bLeveledUp = false;
		}
	}
	else
	{
		PostRaceSummary.LevelBefore = 1;
		PostRaceSummary.XPProgressBefore = 0.0f;
		PostRaceSummary.LevelAfter = 1;
		PostRaceSummary.XPProgressAfter = 0.0f;
		PostRaceSummary.bLeveledUp = false;
	}

	// Check for personal best lap time
	if (LeaderboardSubsystem.IsValid())
	{
		// Find player's best lap from results
		for (const FMGRacerData& Racer : Results.RacerResults)
		{
			if (!Racer.bIsAI && Racer.BestLapTime > 0.0f)
			{
				// Get track records to compare against personal best
				const FName TrackID = Results.Config.TrackName;
				const FMGTrackRecord TrackRecord = LeaderboardSubsystem->GetTrackRecords(TrackID);

				// Convert lap time to milliseconds for comparison (scores stored as ms)
				const int64 PlayerLapTimeMs = static_cast<int64>(Racer.BestLapTime * 1000.0f);
				const int64 PersonalBestMs = TrackRecord.PersonalBest.Score;

				// New personal best if no previous record (Score == 0) or faster time
				PostRaceSummary.bNewPersonalBest = (PersonalBestMs == 0 || PlayerLapTimeMs < PersonalBestMs);

				// Track record if player got the best lap of this race
				PostRaceSummary.bTrackRecord = (Racer.BestLapTime == Results.BestLapTime);
				break;
			}
		}
	}
}

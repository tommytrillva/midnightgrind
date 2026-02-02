// Copyright Midnight Grind. All Rights Reserved.

#include "Progression/MGProgressionSaveGame.h"
#include "Progression/MGPlayerProgression.h"
#include "Progression/MGExtendedProgressionSubsystem.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogMGSaveGame, Log, All);

UMGProgressionSaveGame::UMGProgressionSaveGame()
{
	SaveTimestamp = FDateTime::Now();
}

void UMGProgressionSaveGame::CaptureProgressionState(UGameInstance* GameInstance)
{
	if (!GameInstance)
	{
		UE_LOG(LogMGSaveGame, Error, TEXT("Cannot capture progression: null GameInstance"));
		return;
	}

	// Update metadata
	SaveTimestamp = FDateTime::Now();
	SaveVersion = 1;

	// Capture from base progression
	UMGPlayerProgression* BaseProgression = GameInstance->GetSubsystem<UMGPlayerProgression>();
	if (BaseProgression)
	{
		CaptureBaseProgression(BaseProgression);
	}
	else
	{
		UE_LOG(LogMGSaveGame, Warning, TEXT("MGPlayerProgression subsystem not found"));
	}

	// Capture from extended progression
	UMGExtendedProgressionSubsystem* ExtendedProgression = GameInstance->GetSubsystem<UMGExtendedProgressionSubsystem>();
	if (ExtendedProgression)
	{
		CaptureExtendedProgression(ExtendedProgression);
	}
	else
	{
		UE_LOG(LogMGSaveGame, Warning, TEXT("MGExtendedProgressionSubsystem not found"));
	}

	UE_LOG(LogMGSaveGame, Log, TEXT("Captured progression state: Level %d, Rep %d, $%lld, %d cars, %d locations"),
		LevelProgression.CurrentLevel,
		0, // Total rep calculation needed
		PlayerMoney,
		OwnedCars.Num(),
		UnlockedLocations.Num());
}

void UMGProgressionSaveGame::RestoreProgressionState(UGameInstance* GameInstance)
{
	if (!GameInstance)
	{
		UE_LOG(LogMGSaveGame, Error, TEXT("Cannot restore progression: null GameInstance"));
		return;
	}

	// Restore to base progression
	UMGPlayerProgression* BaseProgression = GameInstance->GetSubsystem<UMGPlayerProgression>();
	if (BaseProgression)
	{
		RestoreBaseProgression(BaseProgression);
	}
	else
	{
		UE_LOG(LogMGSaveGame, Warning, TEXT("MGPlayerProgression subsystem not found"));
	}

	// Restore to extended progression
	UMGExtendedProgressionSubsystem* ExtendedProgression = GameInstance->GetSubsystem<UMGExtendedProgressionSubsystem>();
	if (ExtendedProgression)
	{
		RestoreExtendedProgression(ExtendedProgression);
	}
	else
	{
		UE_LOG(LogMGSaveGame, Warning, TEXT("MGExtendedProgressionSubsystem not found"));
	}

	UE_LOG(LogMGSaveGame, Log, TEXT("Restored progression state from %s (%.1f hours played)"),
		*SaveTimestamp.ToString(), PlayTimeHours);
}

FString UMGProgressionSaveGame::GetSaveInfo() const
{
	return FString::Printf(TEXT("%s | Level %d | %.1fh | $%lld | %d Cars"),
		*PlayerName,
		LevelProgression.CurrentLevel,
		PlayTimeHours,
		PlayerMoney,
		OwnedCars.Num());
}

void UMGProgressionSaveGame::CaptureBaseProgression(UMGPlayerProgression* Progression)
{
	if (!Progression)
	{
		return;
	}

	// Player name
	PlayerName = Progression->GetPlayerName();

	// Level progression
	LevelProgression = Progression->GetLevelProgression();

	// Crew reputations
	CrewReputations.Empty();
	TArray<FMGCrewReputation> AllReps = Progression->GetAllCrewReputations();
	for (const FMGCrewReputation& Rep : AllReps)
	{
		CrewReputations.Add(Rep.Crew, Rep);
	}

	// Unlocks
	Unlocks = Progression->GetAllUnlocks();

	// Statistics
	Statistics = Progression->GetRaceStatistics();

	// Play time for metadata
	PlayTimeHours = Statistics.PlayTimeSeconds / 3600.0f;

	UE_LOG(LogMGSaveGame, Verbose, TEXT("Captured base progression: %s, Level %d, %d XP"),
		*PlayerName, LevelProgression.CurrentLevel, LevelProgression.CurrentXP);
}

void UMGProgressionSaveGame::CaptureExtendedProgression(UMGExtendedProgressionSubsystem* Progression)
{
	if (!Progression)
	{
		return;
	}

	// Money
	PlayerMoney = Progression->GetMoney();

	// Owned cars
	OwnedCars = Progression->GetOwnedCars();

	// Unlocked locations
	UnlockedLocations = Progression->GetUnlockedLocations();

	// Housing
	HousingData.CurrentTier = Progression->GetCurrentHousing();
	HousingData.MaxDisplayCapacity = Progression->GetMaxCarDisplay();
	// Note: Housing cosmetics need to be captured from the subsystem's internal state
	// For now, we'll keep the existing cosmetics if they exist

	// Customization progress
	CustomizationProgress.Empty();
	for (int32 i = 0; i <= static_cast<int32>(EMGCustomizationType::Housing); ++i)
	{
		EMGCustomizationType Category = static_cast<EMGCustomizationType>(i);
		TArray<FName> Unlocked = Progression->GetUnlockedCustomization(Category);
		if (Unlocked.Num() > 0)
		{
			FMGCustomizationProgress Progress;
			Progress.Category = Category;
			Progress.UnlockedItems = Unlocked;
			CustomizationProgress.Add(Progress);
		}
	}

	// Milestones
	CompletedMilestones = Progression->GetCompletedMilestones();

	UE_LOG(LogMGSaveGame, Verbose, TEXT("Captured extended progression: $%lld, %d cars, %d locations"),
		PlayerMoney, OwnedCars.Num(), UnlockedLocations.Num());
}

void UMGProgressionSaveGame::RestoreBaseProgression(UMGPlayerProgression* Progression)
{
	if (!Progression)
	{
		return;
	}

	// Player name
	Progression->SetPlayerName(PlayerName);

	// Level progression (direct access to subsystem members would be needed)
	// For now, add XP to reach the saved level
	// NOTE: This would require exposing setters in MGPlayerProgression or making this class a friend

	// Crew reputations
	for (const auto& Pair : CrewReputations)
	{
		// Would need a SetCrewReputation method in MGPlayerProgression
		// For now, we'd add the difference
		int32 SavedRep = Pair.Value.ReputationPoints;
		int32 CurrentRep = Progression->GetCrewReputation(Pair.Key);
		int32 Delta = SavedRep - CurrentRep;
		if (Delta > 0)
		{
			Progression->AddCrewReputation(Pair.Key, Delta);
		}
	}

	// Unlocks - would need to grant each unlock
	// Statistics - would need setters

	UE_LOG(LogMGSaveGame, Verbose, TEXT("Restored base progression for %s"), *PlayerName);
}

void UMGProgressionSaveGame::RestoreExtendedProgression(UMGExtendedProgressionSubsystem* Progression)
{
	if (!Progression)
	{
		return;
	}

	// NOTE: This requires friend access or public setters in MGExtendedProgressionSubsystem
	// The proper implementation would directly set the subsystem's private member variables
	// For now, this is a placeholder showing the structure

	// Money
	int64 MoneyDelta = PlayerMoney - Progression->GetMoney();
	if (MoneyDelta != 0)
	{
		Progression->AddMoney(MoneyDelta, false);
	}

	// Owned cars - would need direct access to set
	// Unlocked locations - would need direct access to set
	// Housing - would need direct access to set
	// Customization progress - would need direct access to set
	// Milestones - would need direct access to set

	UE_LOG(LogMGSaveGame, Verbose, TEXT("Restored extended progression: $%lld, %d cars"),
		PlayerMoney, OwnedCars.Num());

	// TODO: Add friend class declaration to MGExtendedProgressionSubsystem
	// or create proper save/load interface methods
}

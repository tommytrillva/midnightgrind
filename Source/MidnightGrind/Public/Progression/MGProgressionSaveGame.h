// Copyright Midnight Grind. All Rights Reserved.

#pragma once

/**
 * @file MGProgressionSaveGame.h
 * @brief Save game object for complete progression state
 * 
 * This class serializes all player progression data for persistence:
 * - XP, Level, Reputation (from MGPlayerProgression)
 * - Money, Cars, Locations, Housing (from MGExtendedProgressionSubsystem)
 * - All unlock states and statistics
 * 
 * @see UMGPlayerProgression
 * @see UMGExtendedProgressionSubsystem
 */

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Progression/MGProgressionTypes.h"
#include "Progression/MGPlayerProgression.h"
#include "RaceDirector/MGRaceDirectorSubsystem.h"
#include "Progression/MGContentGatingSubsystem.h"
#include "MGProgressionSaveGame.generated.h"

/**
 * @class UMGProgressionSaveGame
 * @brief Save game object for all player progression data
 * 
 * Stores complete progression state that persists between game sessions.
 * Serializes data from both MGPlayerProgression and MGExtendedProgressionSubsystem.
 * 
 * ## Save/Load Flow
 * 1. Game Mode requests save from progression subsystems
 * 2. Each subsystem writes its state to this save game object
 * 3. Save game is serialized to disk (slot name: "ProgressionSlot")
 * 4. On load, save game data is read and applied to subsystems
 * 
 * ## Usage
 * @code
 * // Save
 * UMGProgressionSaveGame* SaveGame = Cast<UMGProgressionSaveGame>(
 *     UGameplayStatics::CreateSaveGameObject(UMGProgressionSaveGame::StaticClass()));
 * SaveGame->CaptureProgressionState(GetGameInstance());
 * UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("ProgressionSlot"), 0);
 * 
 * // Load
 * UMGProgressionSaveGame* SaveGame = Cast<UMGProgressionSaveGame>(
 *     UGameplayStatics::LoadGameFromSlot(TEXT("ProgressionSlot"), 0));
 * SaveGame->RestoreProgressionState(GetGameInstance());
 * @endcode
 */
UCLASS()
class MIDNIGHTGRIND_API UMGProgressionSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UMGProgressionSaveGame();

	// =============================================================================
	// SAVE/LOAD INTERFACE
	// =============================================================================

	/**
	 * @brief Capture current progression state from game instance subsystems
	 * @param GameInstance The game instance containing progression subsystems
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void CaptureProgressionState(UGameInstance* GameInstance);

	/**
	 * @brief Restore saved progression state to game instance subsystems
	 * @param GameInstance The game instance to restore progression to
	 */
	UFUNCTION(BlueprintCallable, Category = "Save Game")
	void RestoreProgressionState(UGameInstance* GameInstance);

	/**
	 * @brief Get save game metadata (last save time, version, etc.)
	 * @return Formatted string with save info
	 */
	UFUNCTION(BlueprintPure, Category = "Save Game")
	FString GetSaveInfo() const;

	// =============================================================================
	// METADATA
	// =============================================================================

	/** Save game version (for migration/compatibility) */
	UPROPERTY(SaveGame)
	int32 SaveVersion = 1;

	/** Timestamp when save was created */
	UPROPERTY(SaveGame)
	FDateTime SaveTimestamp;

	/** Total play time when saved (for display on load screen) */
	UPROPERTY(SaveGame)
	float PlayTimeHours = 0.0f;

	/** Player name */
	UPROPERTY(SaveGame)
	FString PlayerName;

	// =============================================================================
	// BASE PROGRESSION (MGPlayerProgression)
	// =============================================================================

	/** Level progression data */
	UPROPERTY(SaveGame)
	FMGLevelProgression LevelProgression;

	/** Crew reputations */
	UPROPERTY(SaveGame)
	TMap<EMGCrew, FMGCrewReputation> CrewReputations;

	/** Unlocked content */
	UPROPERTY(SaveGame)
	TArray<FMGUnlock> Unlocks;

	/** Race statistics */
	UPROPERTY(SaveGame)
	FMGRaceStatistics Statistics;

	// =============================================================================
	// EXTENDED PROGRESSION (MGExtendedProgressionSubsystem)
	// =============================================================================

	/** Player money */
	UPROPERTY(SaveGame)
	int64 PlayerMoney = 0;

	/** Owned cars with usage stats */
	UPROPERTY(SaveGame)
	TArray<FMGOwnedCar> OwnedCars;

	/** Unlocked locations */
	UPROPERTY(SaveGame)
	TArray<FMGUnlockedLocation> UnlockedLocations;

	/** Housing data */
	UPROPERTY(SaveGame)
	FMGHousingData HousingData;

	/** Customization progress per category */
	UPROPERTY(SaveGame)
	TArray<FMGCustomizationProgress> CustomizationProgress;

	/** Completed milestones */
	UPROPERTY(SaveGame)
	TArray<FMGMilestone> CompletedMilestones;

	/** Last unlock timestamp (for dead zone detection) */
	UPROPERTY(SaveGame)
	FDateTime LastUnlockTime;

	/** Consecutive losses (for difficulty adjustment) */
	UPROPERTY(SaveGame)
	int32 ConsecutiveLosses = 0;

	// =============================================================================
	// HELPER FUNCTIONS
	// =============================================================================

protected:
	/** Capture data from MGPlayerProgression */
	void CaptureBaseProgression(class UMGPlayerProgression* Progression);

	/** Capture data from MGExtendedProgressionSubsystem */
	void CaptureExtendedProgression(class UMGExtendedProgressionSubsystem* Progression);

	/** Restore data to MGPlayerProgression */
	void RestoreBaseProgression(class UMGPlayerProgression* Progression);

	/** Restore data to MGExtendedProgressionSubsystem */
	void RestoreExtendedProgression(class UMGExtendedProgressionSubsystem* Progression);
};

// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MGSaveGame.generated.h"

/**
 * Heat level save data
 */
USTRUCT(BlueprintType)
struct FMGHeatLevelSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 TotalPursuitsEscaped = 0;

	UPROPERTY(SaveGame)
	int32 TotalPursuitsBusted = 0;

	UPROPERTY(SaveGame)
	int32 MaxHeatLevelReached = 0;

	UPROPERTY(SaveGame)
	float LongestPursuitDuration = 0.0f;

	UPROPERTY(SaveGame)
	int64 TotalFinesPaid = 0;

	UPROPERTY(SaveGame)
	int64 TotalBountyEarned = 0;

	UPROPERTY(SaveGame)
	int32 CopsDisabledTotal = 0;

	UPROPERTY(SaveGame)
	int32 RoadblocksEvadedTotal = 0;

	UPROPERTY(SaveGame)
	int32 SpikeStripsEvadedTotal = 0;
};

/**
 * Shortcut save data
 */
USTRUCT(BlueprintType)
struct FMGShortcutSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FName> DiscoveredShortcuts;

	UPROPERTY(SaveGame)
	int32 TotalShortcutsUsed = 0;

	UPROPERTY(SaveGame)
	float TotalTimeSaved = 0.0f;

	UPROPERTY(SaveGame)
	int32 SecretShortcutsFound = 0;
};

/**
 * Takedown save data
 */
USTRUCT(BlueprintType)
struct FMGTakedownSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 TotalTakedowns = 0;

	UPROPERTY(SaveGame)
	int32 PoliceTakedowns = 0;

	UPROPERTY(SaveGame)
	int32 RacerTakedowns = 0;

	UPROPERTY(SaveGame)
	int32 TrafficTakedowns = 0;

	UPROPERTY(SaveGame)
	int32 PerfectTakedowns = 0;

	UPROPERTY(SaveGame)
	int32 DoubleTakedowns = 0;

	UPROPERTY(SaveGame)
	int32 TripleTakedowns = 0;

	UPROPERTY(SaveGame)
	int64 TotalTakedownScore = 0;
};

/**
 * Vehicle class save data
 */
USTRUCT(BlueprintType)
struct FMGVehicleClassSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TMap<FName, int32> ClassWins;

	UPROPERTY(SaveGame)
	TMap<FName, int32> ClassRaces;

	UPROPERTY(SaveGame)
	TMap<FName, float> ClassBestTimes;

	UPROPERTY(SaveGame)
	TArray<FName> UnlockedClasses;
};

/**
 * License save data
 */
USTRUCT(BlueprintType)
struct FMGLicenseSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FName> EarnedLicenses;

	UPROPERTY(SaveGame)
	TMap<FName, int32> LicenseScores;

	UPROPERTY(SaveGame)
	TMap<FName, FDateTime> LicenseEarnedDates;

	UPROPERTY(SaveGame)
	int32 CurrentLicenseLevel = 0;

	UPROPERTY(SaveGame)
	int32 TotalLicenseTests = 0;

	UPROPERTY(SaveGame)
	int32 PerfectLicenseTests = 0;
};

/**
 * Near miss save data
 */
USTRUCT(BlueprintType)
struct FMGNearMissSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 TotalNearMisses = 0;

	UPROPERTY(SaveGame)
	int32 TrafficNearMisses = 0;

	UPROPERTY(SaveGame)
	int32 RacerNearMisses = 0;

	UPROPERTY(SaveGame)
	int32 OncomingNearMisses = 0;

	UPROPERTY(SaveGame)
	float ClosestNearMissDistance = 0.0f;

	UPROPERTY(SaveGame)
	int32 NearMissChainMax = 0;

	UPROPERTY(SaveGame)
	int64 TotalNearMissScore = 0;
};

/**
 * Stunt save data
 */
USTRUCT(BlueprintType)
struct FMGStuntSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 TotalStunts = 0;

	UPROPERTY(SaveGame)
	int32 JumpsCompleted = 0;

	UPROPERTY(SaveGame)
	int32 DriftsCompleted = 0;

	UPROPERTY(SaveGame)
	int32 AirTimeStunts = 0;

	UPROPERTY(SaveGame)
	int32 TwoWheelStunts = 0;

	UPROPERTY(SaveGame)
	float LongestDrift = 0.0f;

	UPROPERTY(SaveGame)
	float LongestJump = 0.0f;

	UPROPERTY(SaveGame)
	float HighestAirTime = 0.0f;

	UPROPERTY(SaveGame)
	int64 TotalStuntScore = 0;

	UPROPERTY(SaveGame)
	int32 StuntComboMax = 0;
};

/**
 * Main save game class for Midnight Grind
 * Consolidates all subsystem save data
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UMGSaveGame();

	// ==========================================
	// SAVE METADATA
	// ==========================================

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Meta")
	FString SaveSlotName;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Meta")
	FDateTime SaveTimestamp;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Meta")
	int32 SaveVersion = 1;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Meta")
	float TotalPlayTime = 0.0f;

	// ==========================================
	// SUBSYSTEM DATA
	// ==========================================

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "HeatLevel")
	FMGHeatLevelSaveData HeatLevelData;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Shortcut")
	FMGShortcutSaveData ShortcutData;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Takedown")
	FMGTakedownSaveData TakedownData;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "VehicleClass")
	FMGVehicleClassSaveData VehicleClassData;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "License")
	FMGLicenseSaveData LicenseData;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "NearMiss")
	FMGNearMissSaveData NearMissData;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stunt")
	FMGStuntSaveData StuntData;

	// ==========================================
	// PLAYER PROGRESSION
	// ==========================================

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 PlayerLevel = 1;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int64 PlayerXP = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int64 PlayerCash = 5000;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	int32 StoryProgress = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	TArray<FName> CompletedMissions;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	TArray<FName> UnlockedDistricts;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Progression")
	TArray<FName> UnlockedVehicles;

	// ==========================================
	// RACE STATISTICS
	// ==========================================

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	int32 TotalRacesCompleted = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	int32 TotalRacesWon = 0;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	float TotalDistanceDriven = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	float TopSpeedReached = 0.0f;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	TMap<FName, float> TrackBestTimes;

	UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Stats")
	TMap<FName, int32> TrackMedals;

	// ==========================================
	// UTILITY METHODS
	// ==========================================

	/** Get default save slot name */
	static FString GetDefaultSaveSlotName();

	/** Get user index (for multi-user systems) */
	static int32 GetDefaultUserIndex();
};

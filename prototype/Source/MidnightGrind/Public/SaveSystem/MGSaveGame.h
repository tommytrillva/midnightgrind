// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Progression/MGPlayerProgression.h"
#include "Economy/MGEconomySubsystem.h"
#include "Garage/MGGarageSubsystem.h"
#include "MGSaveGame.generated.h"

/**
 * Save slot metadata
 */
USTRUCT(BlueprintType)
struct FMGSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString SlotName;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerLevel = 1;

	UPROPERTY(BlueprintReadOnly)
	int64 Credits = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 VehicleCount = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 TotalRaces = 0;

	UPROPERTY(BlueprintReadOnly)
	FTimespan PlayTime;

	UPROPERTY(BlueprintReadOnly)
	FDateTime LastSaved;

	UPROPERTY(BlueprintReadOnly)
	FString SaveVersion;
};

/**
 * Complete game save data
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

	/** Save version for compatibility checking */
	UPROPERTY(SaveGame)
	FString SaveVersion;

	/** When this save was created */
	UPROPERTY(SaveGame)
	FDateTime SaveTimestamp;

	/** Slot name */
	UPROPERTY(SaveGame)
	FString SlotName;

	// ==========================================
	// PLAYER PROFILE
	// ==========================================

	UPROPERTY(SaveGame)
	FString PlayerName;

	// ==========================================
	// PROGRESSION DATA
	// ==========================================

	UPROPERTY(SaveGame)
	FMGLevelProgression LevelProgression;

	UPROPERTY(SaveGame)
	TMap<EMGCrew, FMGCrewReputation> CrewReputations;

	UPROPERTY(SaveGame)
	TArray<FMGUnlock> Unlocks;

	UPROPERTY(SaveGame)
	FMGRaceStatistics Statistics;

	// ==========================================
	// ECONOMY DATA
	// ==========================================

	UPROPERTY(SaveGame)
	int64 Credits = 0;

	UPROPERTY(SaveGame)
	int64 TotalEarned = 0;

	UPROPERTY(SaveGame)
	int64 TotalSpent = 0;

	UPROPERTY(SaveGame)
	TArray<FMGTransaction> TransactionHistory;

	// ==========================================
	// GARAGE DATA
	// ==========================================

	UPROPERTY(SaveGame)
	TArray<FMGOwnedVehicle> OwnedVehicles;

	UPROPERTY(SaveGame)
	FGuid SelectedVehicleId;

	// ==========================================
	// SETTINGS
	// ==========================================

	UPROPERTY(SaveGame)
	TMap<FName, float> GameplaySettings;

	UPROPERTY(SaveGame)
	TMap<FName, FString> StringSettings;

	// ==========================================
	// UTILITY
	// ==========================================

	/** Get save slot info for UI display */
	FMGSaveSlotInfo GetSlotInfo() const;

	/** Current save version */
	static FString GetCurrentSaveVersion() { return TEXT("1.0.0"); }
};

/**
 * Save game subsystem - handles save/load operations
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// SAVE OPERATIONS
	// ==========================================

	/** Save current game state to a slot */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool SaveGame(const FString& SlotName, int32 UserIndex = 0);

	/** Load game from a slot */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool LoadGame(const FString& SlotName, int32 UserIndex = 0);

	/** Quick save to default slot */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool QuickSave();

	/** Quick load from default slot */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool QuickLoad();

	/** Delete a save slot */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool DeleteSave(const FString& SlotName, int32 UserIndex = 0);

	/** Check if a save exists */
	UFUNCTION(BlueprintPure, Category = "Save System")
	bool DoesSaveExist(const FString& SlotName, int32 UserIndex = 0) const;

	// ==========================================
	// SLOT MANAGEMENT
	// ==========================================

	/** Get all save slot names */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	TArray<FString> GetAllSaveSlots() const;

	/** Get info for a specific save slot */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	bool GetSaveSlotInfo(const FString& SlotName, FMGSaveSlotInfo& OutInfo, int32 UserIndex = 0) const;

	/** Get info for all save slots */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	TArray<FMGSaveSlotInfo> GetAllSaveSlotInfo() const;

	/** Get the current slot name */
	UFUNCTION(BlueprintPure, Category = "Save System")
	FString GetCurrentSlotName() const { return CurrentSlotName; }

	// ==========================================
	// AUTO-SAVE
	// ==========================================

	/** Enable/disable auto-save */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void SetAutoSaveEnabled(bool bEnabled);

	/** Is auto-save enabled? */
	UFUNCTION(BlueprintPure, Category = "Save System")
	bool IsAutoSaveEnabled() const { return bAutoSaveEnabled; }

	/** Trigger auto-save (if enough time has passed) */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void TriggerAutoSave();

	// ==========================================
	// NEW GAME
	// ==========================================

	/** Start a new game (resets all data) */
	UFUNCTION(BlueprintCallable, Category = "Save System")
	void StartNewGame(const FString& PlayerName);

	/** Is this a new game (no save loaded)? */
	UFUNCTION(BlueprintPure, Category = "Save System")
	bool IsNewGame() const { return bIsNewGame; }

	// ==========================================
	// EVENTS
	// ==========================================

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveCompleted, bool, bSuccess, const FString&, SlotName);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadCompleted, bool, bSuccess, const FString&, SlotName);

	UPROPERTY(BlueprintAssignable, Category = "Save System|Events")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Save System|Events")
	FOnLoadCompleted OnLoadCompleted;

protected:
	/** Collect current game state into save object */
	void CollectSaveData(UMGSaveGame* SaveGame);

	/** Apply loaded save data to game state */
	void ApplySaveData(const UMGSaveGame* SaveGame);

	/** Get the full slot name with prefix */
	FString GetFullSlotName(const FString& SlotName) const;

	// ==========================================
	// DATA
	// ==========================================

	/** Current save slot name */
	UPROPERTY()
	FString CurrentSlotName;

	/** Is auto-save enabled? */
	UPROPERTY()
	bool bAutoSaveEnabled = true;

	/** Auto-save interval in seconds */
	UPROPERTY()
	float AutoSaveInterval = 300.0f; // 5 minutes

	/** Time since last auto-save */
	float TimeSinceLastAutoSave = 0.0f;

	/** Is this a new game? */
	bool bIsNewGame = true;

	/** Save slot name prefix */
	static const FString SaveSlotPrefix;

	/** Default/quick save slot name */
	static const FString DefaultSlotName;

	/** Maximum save slots */
	static constexpr int32 MaxSaveSlots = 10;
};

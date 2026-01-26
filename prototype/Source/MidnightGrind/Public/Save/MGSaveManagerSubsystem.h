// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSaveGame.h"
#include "MGSaveManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

/**
 * Central save/load manager for all game systems
 * Coordinates persistence across all subsystems
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// SAVE OPERATIONS
	// ==========================================

	/** Save all game data to slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(const FString& SlotName = TEXT(""));

	/** Save game asynchronously */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGameAsync(const FString& SlotName = TEXT(""));

	/** Quick save to default slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool QuickSave();

	// ==========================================
	// LOAD OPERATIONS
	// ==========================================

	/** Load game from slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(const FString& SlotName = TEXT(""));

	/** Load game asynchronously */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGameAsync(const FString& SlotName = TEXT(""));

	/** Quick load from default slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool QuickLoad();

	// ==========================================
	// SAVE MANAGEMENT
	// ==========================================

	/** Check if save exists */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveExist(const FString& SlotName = TEXT("")) const;

	/** Delete save slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSave(const FString& SlotName);

	/** Get list of all save slots */
	UFUNCTION(BlueprintPure, Category = "Save")
	TArray<FString> GetAllSaveSlots() const;

	/** Create new game save */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void CreateNewGame(const FString& SlotName = TEXT(""));

	// ==========================================
	// DATA ACCESS
	// ==========================================

	/** Get current save data (read-only) */
	UFUNCTION(BlueprintPure, Category = "Save")
	const UMGSaveGame* GetCurrentSaveData() const { return CurrentSaveGame; }

	/** Get current save data (modifiable) */
	UFUNCTION(BlueprintCallable, Category = "Save")
	UMGSaveGame* GetSaveDataMutable() { return CurrentSaveGame; }

	/** Check if currently loading */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsLoading() const { return bIsLoading; }

	/** Check if currently saving */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsSaving() const { return bIsSaving; }

	// ==========================================
	// AUTOSAVE
	// ==========================================

	/** Enable/disable autosave */
	UFUNCTION(BlueprintCallable, Category = "Save|Autosave")
	void SetAutosaveEnabled(bool bEnabled);

	/** Set autosave interval (seconds) */
	UFUNCTION(BlueprintCallable, Category = "Save|Autosave")
	void SetAutosaveInterval(float Seconds);

	/** Trigger autosave now */
	UFUNCTION(BlueprintCallable, Category = "Save|Autosave")
	void TriggerAutosave();

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnLoadCompleted OnLoadCompleted;

protected:
	/** Gather data from all subsystems */
	void GatherSubsystemData();

	/** Distribute data to all subsystems */
	void DistributeSubsystemData();

	/** Validate save data integrity */
	bool ValidateSaveData(const UMGSaveGame* SaveData) const;

	/** Handle async save completion */
	void OnAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);

	/** Handle async load completion */
	void OnAsyncLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedGame);

	/** Autosave timer callback */
	void OnAutosaveTimer();

private:
	UPROPERTY()
	UMGSaveGame* CurrentSaveGame;

	FString CurrentSlotName;
	int32 UserIndex = 0;

	bool bIsLoading = false;
	bool bIsSaving = false;
	bool bAutosaveEnabled = true;
	float AutosaveInterval = 300.0f; // 5 minutes

	FTimerHandle AutosaveTimerHandle;
};

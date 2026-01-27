// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGSaveManagerSubsystem.h
 * @brief Central Save Manager Subsystem for Midnight Grind
 *
 * This subsystem serves as the primary coordinator for all save/load operations
 * in the game. It manages persistence of game state across play sessions,
 * handling both synchronous and asynchronous save operations.
 *
 * ## Overview
 * The Save Manager acts as a central hub that:
 * - Coordinates data collection from various game subsystems (profile, garage, progression, etc.)
 * - Manages save slots and quick save functionality
 * - Provides autosave capabilities with configurable intervals
 * - Handles async operations to prevent frame hitches during save/load
 *
 * ## Usage Example
 * @code
 * // Get the save manager from the game instance
 * UMGSaveManagerSubsystem* SaveManager = GameInstance->GetSubsystem<UMGSaveManagerSubsystem>();
 *
 * // Save to a specific slot
 * SaveManager->SaveGame("Slot_01");
 *
 * // Or use quick save for the default slot
 * SaveManager->QuickSave();
 *
 * // Load game data
 * SaveManager->LoadGame("Slot_01");
 * @endcode
 *
 * ## Architecture Notes
 * - Inherits from UGameInstanceSubsystem, so it persists across level transitions
 * - Works in conjunction with UMGCloudSaveSubsystem for cloud synchronization
 * - Broadcasts delegates when save/load operations complete
 *
 * @see UMGCloudSaveSubsystem For cloud save synchronization
 * @see UMGSaveGame For the save data structure
 * @see UMGProfileManagerSubsystem For player profile data
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGSaveGame.h"
#include "MGSaveManagerSubsystem.generated.h"

// ============================================================================
// DELEGATE DECLARATIONS
// ============================================================================

/// Delegate broadcast when a save operation completes (success or failure)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
/// Delegate broadcast when a load operation completes (success or failure)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

/**
 * @class UMGSaveManagerSubsystem
 * @brief Central save/load manager that coordinates persistence across all game subsystems.
 *
 * This subsystem is the main entry point for all save/load operations. It gathers data
 * from various game systems, serializes it to disk, and restores it when loading.
 *
 * Key responsibilities:
 * - Managing save slots and their metadata
 * - Coordinating data collection from other subsystems
 * - Handling synchronous and asynchronous save/load operations
 * - Managing autosave functionality
 * - Validating save data integrity
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSaveManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @brief Initialize the subsystem. Called automatically by the engine.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// @brief Cleanup the subsystem. Called automatically by the engine.
	virtual void Deinitialize() override;

	// ============================================================================
	// SAVE OPERATIONS
	// ============================================================================
	// Functions for persisting game state to disk. Use synchronous versions for
	// critical saves (e.g., after purchases) and async versions for regular saves
	// to avoid frame hitches.
	// ============================================================================

	/**
	 * @brief Saves all game data to the specified slot synchronously.
	 *
	 * This is a blocking operation that will cause a brief hitch. Use SaveGameAsync()
	 * for non-critical saves to maintain smooth gameplay.
	 *
	 * @param SlotName The name of the save slot. Empty string uses the current/default slot.
	 * @return true if the save was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(const FString& SlotName = TEXT(""));

	/**
	 * @brief Saves game data asynchronously without blocking the game thread.
	 *
	 * Preferred method for regular saves. The OnSaveCompleted delegate will fire
	 * when the operation completes.
	 *
	 * @param SlotName The name of the save slot. Empty string uses the current/default slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGameAsync(const FString& SlotName = TEXT(""));

	/**
	 * @brief Performs a quick save to the default slot.
	 *
	 * Convenience function that saves to the predefined quick save slot.
	 * Useful for binding to a hotkey (e.g., F5).
	 *
	 * @return true if the quick save was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool QuickSave();

	// ============================================================================
	// LOAD OPERATIONS
	// ============================================================================
	// Functions for restoring game state from disk. After loading, data is
	// automatically distributed to all relevant subsystems.
	// ============================================================================

	/**
	 * @brief Loads game data from the specified slot synchronously.
	 *
	 * This is a blocking operation. After loading, the data is distributed to
	 * all game subsystems automatically.
	 *
	 * @param SlotName The name of the save slot to load from. Empty uses current/default.
	 * @return true if the load was successful, false if the file doesn't exist or is corrupted.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(const FString& SlotName = TEXT(""));

	/**
	 * @brief Loads game data asynchronously without blocking the game thread.
	 *
	 * The OnLoadCompleted delegate will fire when the operation completes.
	 * Use IsSaving() to check if a load is in progress.
	 *
	 * @param SlotName The name of the save slot to load from.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void LoadGameAsync(const FString& SlotName = TEXT(""));

	/**
	 * @brief Performs a quick load from the default quick save slot.
	 *
	 * Convenience function for loading the quick save. Useful for binding
	 * to a hotkey (e.g., F9).
	 *
	 * @return true if the quick load was successful.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool QuickLoad();

	// ============================================================================
	// SAVE MANAGEMENT
	// ============================================================================
	// Functions for managing save slots: checking existence, deletion, listing,
	// and creating new games. Use these to build save slot selection UI.
	// ============================================================================

	/**
	 * @brief Checks whether a save file exists for the given slot.
	 *
	 * Use this to determine which slots to display as occupied in the UI
	 * and to validate before attempting a load operation.
	 *
	 * @param SlotName The save slot name to check. Empty checks the current slot.
	 * @return true if a save file exists for this slot.
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveExist(const FString& SlotName = TEXT("")) const;

	/**
	 * @brief Permanently deletes a save slot.
	 *
	 * WARNING: This operation cannot be undone. Consider prompting the user
	 * for confirmation before calling this.
	 *
	 * @param SlotName The save slot to delete.
	 * @return true if the save was deleted successfully.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSave(const FString& SlotName);

	/**
	 * @brief Retrieves a list of all available save slot names.
	 *
	 * Use this to populate a save slot selection screen.
	 *
	 * @return Array of save slot names that have existing save data.
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	TArray<FString> GetAllSaveSlots() const;

	/**
	 * @brief Creates a fresh save for a new game.
	 *
	 * Initializes default values for all game systems and saves to the specified slot.
	 * Call this when the player starts a new game from the main menu.
	 *
	 * @param SlotName The slot to create the new game in. Empty uses the default slot.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void CreateNewGame(const FString& SlotName = TEXT(""));

	// ============================================================================
	// DATA ACCESS
	// ============================================================================
	// Functions for accessing the current save data in memory. Use GetCurrentSaveData()
	// for read-only access and GetSaveDataMutable() when modifications are needed.
	// ============================================================================

	/**
	 * @brief Gets the current save data for read-only access.
	 *
	 * Use this when you need to inspect save data without modifying it.
	 * Returns nullptr if no save is currently loaded.
	 *
	 * @return Const pointer to the current save game data.
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	const UMGSaveGame* GetCurrentSaveData() const { return CurrentSaveGame; }

	/**
	 * @brief Gets the current save data for modification.
	 *
	 * Use this when you need to update save data. Remember to call SaveGame()
	 * or SaveGameAsync() after making changes to persist them.
	 *
	 * @return Mutable pointer to the current save game data.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	UMGSaveGame* GetSaveDataMutable() { return CurrentSaveGame; }

	/**
	 * @brief Checks if a load operation is currently in progress.
	 *
	 * Useful for showing loading indicators and preventing conflicting operations.
	 *
	 * @return true if an async load is in progress.
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsLoading() const { return bIsLoading; }

	/**
	 * @brief Checks if a save operation is currently in progress.
	 *
	 * Useful for showing saving indicators and preventing conflicting operations.
	 *
	 * @return true if an async save is in progress.
	 */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool IsSaving() const { return bIsSaving; }

	// ============================================================================
	// AUTOSAVE
	// ============================================================================
	// Automatic save functionality that periodically saves player progress.
	// Configure the interval and enable/disable as needed. Autosave uses
	// async operations to minimize gameplay interruption.
	// ============================================================================

	/**
	 * @brief Enables or disables the autosave feature.
	 *
	 * When enabled, the game will automatically save at regular intervals
	 * defined by SetAutosaveInterval(). Disabled by default during tutorials
	 * or cinematics.
	 *
	 * @param bEnabled true to enable autosave, false to disable.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save|Autosave")
	void SetAutosaveEnabled(bool bEnabled);

	/**
	 * @brief Sets how frequently autosave triggers.
	 *
	 * @param Seconds Time between autosaves. Default is 300 seconds (5 minutes).
	 *                Values below 60 seconds are not recommended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save|Autosave")
	void SetAutosaveInterval(float Seconds);

	/**
	 * @brief Forces an immediate autosave.
	 *
	 * Call this at important gameplay moments (e.g., after completing a race,
	 * making a purchase) to ensure progress is not lost. Resets the autosave timer.
	 */
	UFUNCTION(BlueprintCallable, Category = "Save|Autosave")
	void TriggerAutosave();

	// ============================================================================
	// EVENTS / DELEGATES
	// ============================================================================
	// Blueprint-assignable events for responding to save/load completion.
	// Bind to these to update UI, trigger notifications, etc.
	// ============================================================================

	/// Broadcast when a save operation completes. Check bSuccess for the result.
	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnSaveCompleted OnSaveCompleted;

	/// Broadcast when a load operation completes. Check bSuccess for the result.
	UPROPERTY(BlueprintAssignable, Category = "Save|Events")
	FOnLoadCompleted OnLoadCompleted;

protected:
	// ============================================================================
	// INTERNAL DATA MANAGEMENT
	// ============================================================================

	/**
	 * @brief Collects save data from all game subsystems.
	 *
	 * Called before saving to gather the latest state from profile manager,
	 * garage system, progression, etc. into the save game object.
	 */
	void GatherSubsystemData();

	/**
	 * @brief Pushes loaded data out to all game subsystems.
	 *
	 * Called after loading to restore game state. Each subsystem receives
	 * its relevant data portion and updates its internal state.
	 */
	void DistributeSubsystemData();

	/**
	 * @brief Validates save data integrity before use.
	 *
	 * Checks for corruption, version mismatches, and data consistency.
	 *
	 * @param SaveData The save data to validate.
	 * @return true if the data is valid and safe to use.
	 */
	bool ValidateSaveData(const UMGSaveGame* SaveData) const;

	/**
	 * @brief Callback for async save completion.
	 *
	 * Handles cleanup and broadcasts the OnSaveCompleted delegate.
	 */
	void OnAsyncSaveComplete(const FString& SlotName, int32 UserIndex, bool bSuccess);

	/**
	 * @brief Callback for async load completion.
	 *
	 * Validates loaded data, distributes to subsystems, and broadcasts OnLoadCompleted.
	 */
	void OnAsyncLoadComplete(const FString& SlotName, int32 UserIndex, USaveGame* LoadedGame);

	/**
	 * @brief Timer callback that triggers autosave.
	 *
	 * Called by the autosave timer at the configured interval.
	 */
	void OnAutosaveTimer();

private:
	// ============================================================================
	// MEMBER VARIABLES
	// ============================================================================

	/// The current save game data in memory
	UPROPERTY()
	UMGSaveGame* CurrentSaveGame;

	/// Name of the currently active save slot
	FString CurrentSlotName;

	/// User index for multi-user systems (typically 0 for single-player)
	int32 UserIndex = 0;

	/// Flag indicating an async load is in progress
	bool bIsLoading = false;

	/// Flag indicating an async save is in progress
	bool bIsSaving = false;

	/// Whether autosave functionality is enabled
	bool bAutosaveEnabled = true;

	/// Time between autosaves in seconds (default: 5 minutes)
	float AutosaveInterval = 300.0f;

	/// Timer handle for the autosave timer
	FTimerHandle AutosaveTimerHandle;
};

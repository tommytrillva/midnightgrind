// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGSaveSubsystem.h - Game Save/Load System
 * =============================================================================
 *
 * WHAT THIS FILE DOES:
 * --------------------
 * This file implements the complete save/load system for Midnight Grind.
 * It handles persisting player progress, vehicle collections, race records,
 * and all game settings to disk (and optionally cloud storage).
 *
 * KEY CONCEPTS FOR BEGINNERS:
 * ---------------------------
 *
 * 1. GAME INSTANCE SUBSYSTEM (UGameInstanceSubsystem):
 *    - A subsystem attached to the Game Instance
 *    - Lives for the entire game session (doesn't reset on level change)
 *    - Perfect for save systems that need to persist across levels
 *    - Auto-created by Unreal when the game starts
 *    - Access via: GameInstance->GetSubsystem<UMGSaveSubsystem>()
 *
 * 2. USAVEGAME:
 *    - Unreal's built-in base class for save data
 *    - Handles serialization (converting objects to bytes and back)
 *    - Works with UGameplayStatics::SaveGameToSlot/LoadGameFromSlot
 *    - Stores data in platform-specific save locations
 *
 * 3. USTRUCT (Data Structures):
 *    - Plain data containers (no inheritance, no virtual functions)
 *    - Lighter weight than UObjects - good for data storage
 *    - Can be nested, used in arrays, and serialized automatically
 *    - Each struct here represents a category of save data
 *
 * 4. SAVE SLOT SYSTEM:
 *    - Multiple save slots let players have separate playthroughs
 *    - Slot 0 = Quick Save, Slot 9 = Auto Save (by default)
 *    - Slot metadata (FMGSaveSlotInfo) provides preview without loading
 *
 * 5. DELEGATES (Events):
 *    - Broadcast notifications when save/load operations occur
 *    - UI can bind to these to show progress indicators
 *    - Async operations notify when complete
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 * -----------------------------------------
 *
 *   [UMGGameInstance]
 *         |
 *         +-- [UMGSaveSubsystem] <-- This file
 *         |         |
 *         |         +-- FMGSaveGameData (all save data)
 *         |                 |
 *         |                 +-- FMGSaveProfileData (player identity)
 *         |                 +-- FMGSaveVehicleData[] (garage)
 *         |                 +-- FMGSaveProgressionData (unlocks)
 *         |                 +-- FMGSaveRivalData[] (AI rivals)
 *         |
 *         +-- [UMGCloudSaveSubsystem] (optional cloud sync)
 *
 * SAVE DATA HIERARCHY:
 * --------------------
 * - FMGSaveSlotInfo: Quick metadata for slot selection UI
 * - FMGSaveProfileData: Player name, level, cash, settings
 * - FMGSaveVehicleData: Per-vehicle builds, colors, stats
 * - FMGSaveProgressionData: Story progress, unlocks, achievements
 * - FMGSaveRivalData: AI rival relationships and history
 * - FMGSaveGameData: Container holding all of the above
 *
 * COMMON PATTERNS:
 * ----------------
 * @code
 * // Get the save subsystem
 * UMGSaveSubsystem* Save = GameInstance->GetSubsystem<UMGSaveSubsystem>();
 *
 * // Save current game to slot 1
 * Save->SaveGame(1);
 *
 * // Load from slot 1
 * Save->LoadGame(1);
 *
 * // Check player's cash
 * int64 Cash = Save->GetCurrentCash();
 *
 * // Add a new vehicle to the garage
 * FMGSaveVehicleData NewCar;
 * NewCar.VehicleDefinitionID = "Mustang_GT";
 * FGuid CarID = Save->AddOwnedVehicle(NewCar);
 *
 * // Listen for save completion
 * Save->OnSaveCompleted.AddDynamic(this, &MyClass::OnSaveFinished);
 * @endcode
 *
 * PINK SLIP SYSTEM:
 * -----------------
 * The save system tracks "pink slip" racing where players can win or lose
 * vehicles permanently. Includes cooldown timers and trade locks to prevent
 * exploitation.
 *
 * @see UMGCloudSaveSubsystem For cloud save synchronization
 * @see UMGGameInstance For the parent game instance
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/SaveGame.h"
#include "Data/MGPartsCatalog.h"
#include "Racing/MGRaceModeSubsystem.h"
#include "MGSaveSubsystem.generated.h"

// ============================================================================
// SAVE SLOT INFO - Metadata for slot selection UI
// ============================================================================

/**
 * Save slot metadata - lightweight info for displaying in save/load UI
 *
 * This struct provides just enough information to show in a slot selection
 * menu without having to load the entire save file. Think of it as a
 * "preview" of what's in each save slot.
 */
USTRUCT(BlueprintType)
struct FMGSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int64 TotalCash = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 TotalREP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 TotalVehicles = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	float TotalPlaytime = 0.0f; // Hours

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	FDateTime LastSaveTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	FString CurrentVehicleName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	bool bIsValid = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot")
	int32 SaveVersion = 1;
};

// ============================================================================
// PLAYER PROFILE - Core player identity and preferences
// ============================================================================

/**
 * Saved player profile data - who the player is and their preferences
 *
 * This is the "identity" portion of the save - player name, level,
 * currency, and gameplay preferences like metric vs imperial units.
 */
USTRUCT(BlueprintType)
struct FMGSaveProfileData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 TotalXP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int64 Cash = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 REP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	float TotalPlaytime = 0.0f;

	// Crew
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	FName CrewID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 CrewRank = 0;

	// Settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	bool bMetricUnits = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	bool bManualTransmission = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile")
	int32 DifficultyLevel = 1; // 0=Easy, 4=Legendary
};

// ============================================================================
// VEHICLE DATA - Per-vehicle state for the player's garage
// ============================================================================

/**
 * Saved vehicle data - everything about a single owned vehicle
 *
 * Each vehicle the player owns gets one of these structs. It tracks:
 * - Which vehicle it is (definition ID) and unique instance (GUID)
 * - Current parts/build configuration
 * - Visual customization (colors, body kits, wheels)
 * - Wear/damage state (if wear simulation is enabled)
 * - Per-vehicle statistics (races, wins, miles driven)
 * - Economic data (purchase price, total invested)
 *
 * NOTE: VehicleInstanceID is a GUID (globally unique identifier) that
 * distinguishes this specific car instance from others of the same model.
 * A player might own two Mustangs - each has a different instance ID.
 */
USTRUCT(BlueprintType)
struct FMGSaveVehicleData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FGuid VehicleInstanceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FName VehicleDefinitionID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FString CustomName; // Player-assigned nickname

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	bool bIsFavorite = false;

	// Build/Parts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle")
	FMGVehicleBuild CurrentBuild;

	// Visual customization
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor BodyColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FLinearColor SecondaryColor = FLinearColor::Black;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 BodyKitIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 SpoilerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 HoodIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FName WheelID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 WheelSize = 17;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	float WindowTint = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	int32 LicensePlateIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FString LicensePlateText;

	// Condition
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float EngineWear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float TireWear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float BrakeWear = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float BodyDamage = 0.0f;

	// Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalRaces = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	int32 TotalWins = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TotalMilesDriven = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float BestQuarterMile = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float TopSpeedReached = 0.0f;

	// Economy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int64 PurchasePrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	int64 TotalInvested = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Economy")
	FDateTime PurchaseDate;
};

// ============================================================================
// PROGRESSION DATA - Career progress, unlocks, and achievements
// ============================================================================

/**
 * Saved progression data - tracking player's journey through the game
 *
 * This struct holds all the "progress" data - what the player has
 * accomplished, unlocked, and achieved. It includes:
 * - Story/career chapter progress
 * - Completed missions and unlocked areas
 * - Race statistics and personal best times
 * - Achievements and their progress
 * - Unlocked vehicles, parts, and cosmetics
 * - Police pursuit history (busts, escapes, fines)
 *
 * TIP: TMap<FName, int32> is like a dictionary/hash table mapping
 * names to integers. Great for tracking "how many times did X happen".
 */
USTRUCT(BlueprintType)
struct FMGSaveProgressionData
{
	GENERATED_BODY()

	// Story/Career
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
	int32 CurrentChapter = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
	TArray<FName> CompletedMissions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Career")
	TArray<FName> UnlockedAreas;

	// Races
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Races")
	int32 TotalRacesCompleted = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Races")
	int32 TotalRacesWon = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Races")
	TMap<FName, int32> RaceWinsByTrack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Races")
	TArray<FMGTrackRecord> PersonalBests;

	// Achievements
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievements")
	TArray<FName> UnlockedAchievements;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Achievements")
	TMap<FName, int32> AchievementProgress;

	// Unlocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
	TArray<FName> UnlockedVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
	TArray<FName> UnlockedParts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
	TArray<FName> UnlockedVisuals;

	// Police/Wanted
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police")
	int32 TotalBusts = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police")
	int32 TotalEscapes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police")
	int64 TotalFinesPaid = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Police")
	float LongestPursuitTime = 0.0f;
};

// ============================================================================
// RIVAL DATA - AI opponent relationships
// ============================================================================

/**
 * Saved rival/social data - tracks relationships with AI racers
 *
 * The rivalry system creates persistent AI opponents that remember
 * your race history. Beat someone enough times and they become
 * your rival. Lose to them and the "heat" between you increases.
 */
USTRUCT(BlueprintType)
struct FMGSaveRivalData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rival")
	FName RivalID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rival")
	int32 RivalryLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rival")
	int32 WinsAgainst = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rival")
	int32 LossesAgainst = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rival")
	int32 HeatLevel = 0;
};

// ============================================================================
// COMPLETE SAVE DATA - The master container for all save data
// ============================================================================

/**
 * Complete save game data - the top-level container holding everything
 *
 * This struct aggregates all the individual data types into one
 * complete save file. When you save, this entire struct gets
 * serialized to disk. When you load, it gets deserialized back.
 *
 * SAVE VERSIONING:
 * SaveVersion allows for backwards compatibility when the save format
 * changes. If you add new fields, increment the version and add
 * migration code to handle loading old saves.
 */
USTRUCT(BlueprintType)
struct FMGSaveGameData
{
	GENERATED_BODY()

	// Metadata
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	int32 SaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FDateTime SaveTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meta")
	FString GameVersion;

	// Core data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FMGSaveProfileData Profile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FMGSaveVehicleData> OwnedVehicles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FGuid CurrentVehicleID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FMGSaveProgressionData Progression;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	TArray<FMGSaveRivalData> Rivalries;

	// Inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, int32> PartsInventory; // PartID -> Quantity

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, int32> ConsumablesInventory;

	// Settings (stored in save for cloud sync)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TMap<FString, FString> GameSettings;

	// ==========================================
	// PINK SLIP DATA
	// ==========================================

	/** Pink slip cooldown state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	FDateTime PinkSlipCooldownExpires;

	/** Lost vehicle that triggered cooldown */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	FGuid PinkSlipCooldownTransferID;

	/** Name of vehicle lost (for UI display) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	FText PinkSlipCooldownVehicleName;

	/** Trade lock vehicle IDs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	TArray<FGuid> TradeLockVehicleIDs;

	/** Trade lock expiration times (parallel to TradeLockVehicleIDs) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	TArray<FDateTime> TradeLockExpirations;

	/** Pink slip wins count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	int32 PinkSlipWins = 0;

	/** Pink slip losses count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	int32 PinkSlipLosses = 0;

	/** Total value of vehicles won */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	int64 PinkSlipValueWon = 0;

	/** Total value of vehicles lost */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PinkSlip")
	int64 PinkSlipValueLost = 0;
};

// ============================================================================
// DELEGATES - Event notifications for save/load operations
// ============================================================================

/**
 * WHAT ARE DELEGATES?
 * Delegates are Unreal's type-safe callback/event system. They allow one
 * piece of code to notify many listeners when something happens.
 *
 * DECLARE_DYNAMIC_MULTICAST_DELEGATE_*:
 * - DYNAMIC: Can be bound from Blueprints (uses reflection)
 * - MULTICAST: Multiple listeners can subscribe (like C# events)
 * - The suffix indicates parameter count (OneParam, TwoParams, etc.)
 *
 * HOW TO USE:
 * @code
 * // In C++ - bind a member function:
 * SaveSubsystem->OnSaveCompleted.AddDynamic(this, &MyClass::HandleSaveComplete);
 *
 * // In Blueprint - use "Bind Event" node on the delegate
 *
 * // The delegate owner broadcasts when the event occurs:
 * OnSaveCompleted.Broadcast(SlotIndex, bSuccess);
 * @endcode
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveStarted, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveCompleted, int32, SlotIndex, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadStarted, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadCompleted, int32, SlotIndex, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutoSave, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveSlotDeleted, int32, SlotIndex, bool, bSuccess);

// ============================================================================
// SAVE GAME OBJECT - USaveGame wrapper for Unreal serialization
// ============================================================================

/**
 * Save game object wrapper for UE serialization
 *
 * WHY DO WE NEED THIS?
 * Unreal's built-in save functions (SaveGameToSlot/LoadGameFromSlot) require
 * a USaveGame-derived object. This wrapper holds our FMGSaveGameData struct.
 *
 * We use a struct for the actual data because:
 * - Structs are easier to copy and pass around
 * - We can nest structs cleanly
 * - The UObject wrapper is just for serialization compatibility
 *
 * This is a common pattern in Unreal - wrap your data struct in a
 * minimal USaveGame class for disk serialization.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGSaveGameObject : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FMGSaveGameData SaveData;
};

// ============================================================================
// MAIN SAVE SUBSYSTEM CLASS
// ============================================================================

/**
 * Save/Load System Subsystem - The main API for all save operations
 *
 * This is the primary interface for saving and loading game data.
 * It provides functions for:
 *
 * CORE OPERATIONS:
 * - SaveGame() / LoadGame() - Manual save/load to specific slots
 * - QuickSave() / QuickLoad() - Fast access to slot 0
 * - TriggerAutoSave() - Automatic periodic saves
 *
 * DATA ACCESS (Read/Write current save data):
 * - Profile: SetPlayerName, AddCash, SpendCash, AddREP, AddXP
 * - Vehicles: AddOwnedVehicle, RemoveOwnedVehicle, UpdateVehicleData
 * - Progression: CompleteRace, CompleteMission, UnlockAchievement
 * - Inventory: AddPartToInventory, RemovePartFromInventory
 * - Settings: SetGameSetting, GetGameSetting
 *
 * SLOT MANAGEMENT:
 * - GetAllSaveSlots() - List all slots with metadata
 * - DeleteSaveSlot() - Erase a save
 * - CopySaveSlot() - Duplicate a save to another slot
 *
 * IMPORTANT PATTERNS:
 * - Always call MarkDirty() after modifying data to enable auto-save
 * - Use OnSaveCompleted/OnLoadCompleted for async UI feedback
 * - Check IsSaveSlotValid() before loading
 *
 * THREAD SAFETY:
 * Save operations can be async (SaveGameAsync). The subsystem handles
 * queuing and prevents concurrent operations on the same slot.
 *
 * @see UMGGameInstance For the owning game instance
 * @see UMGCloudSaveSubsystem For cloud synchronization
 */
UCLASS(BlueprintType)
class MIDNIGHTGRIND_API UMGSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// DELEGATES
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSaveStarted OnSaveStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadStarted OnLoadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadCompleted OnLoadCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAutoSave OnAutoSave;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSaveSlotDeleted OnSaveSlotDeleted;

	// ==========================================
	// SAVE OPERATIONS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save")
	void SaveGameAsync(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Save")
	bool QuickSave();

	UFUNCTION(BlueprintCallable, Category = "Save")
	void TriggerAutoSave();

	// ==========================================
	// LOAD OPERATIONS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Load")
	bool LoadGame(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Load")
	void LoadGameAsync(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Load")
	bool QuickLoad();

	// ==========================================
	// SLOT MANAGEMENT
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Slots")
	TArray<FMGSaveSlotInfo> GetAllSaveSlots() const;

	UFUNCTION(BlueprintCallable, Category = "Slots")
	FMGSaveSlotInfo GetSaveSlotInfo(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Slots")
	bool IsSaveSlotValid(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Slots")
	bool DeleteSaveSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Slots")
	bool CopySaveSlot(int32 SourceSlot, int32 DestSlot);

	UFUNCTION(BlueprintPure, Category = "Slots")
	int32 GetMaxSaveSlots() const { return MaxSaveSlots; }

	UFUNCTION(BlueprintPure, Category = "Slots")
	int32 GetQuickSaveSlot() const { return QuickSaveSlotIndex; }

	UFUNCTION(BlueprintPure, Category = "Slots")
	int32 GetAutoSaveSlot() const { return AutoSaveSlotIndex; }

	// ==========================================
	// CURRENT SAVE DATA ACCESS
	// ==========================================

	UFUNCTION(BlueprintPure, Category = "Current")
	const FMGSaveGameData& GetCurrentSaveData() const { return CurrentSaveData; }

	UFUNCTION(BlueprintPure, Category = "Current")
	bool HasUnsavedChanges() const { return bHasUnsavedChanges; }

	UFUNCTION(BlueprintPure, Category = "Current")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	UFUNCTION(BlueprintCallable, Category = "Current")
	void MarkDirty();

	// ==========================================
	// PROFILE DATA
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Profile")
	void SetPlayerName(const FString& Name);

	UFUNCTION(BlueprintPure, Category = "Profile")
	FString GetPlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "Profile")
	void AddCash(int64 Amount);

	UFUNCTION(BlueprintCallable, Category = "Profile")
	bool SpendCash(int64 Amount);

	UFUNCTION(BlueprintPure, Category = "Profile")
	int64 GetCurrentCash() const;

	UFUNCTION(BlueprintCallable, Category = "Profile")
	void AddREP(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Profile")
	int32 GetCurrentREP() const;

	UFUNCTION(BlueprintCallable, Category = "Profile")
	void AddXP(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Profile")
	int32 GetCurrentLevel() const;

	UFUNCTION(BlueprintCallable, Category = "Profile")
	void AddPlaytime(float Hours);

	// ==========================================
	// VEHICLE DATA
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Vehicles")
	FGuid AddOwnedVehicle(const FMGSaveVehicleData& VehicleData);

	UFUNCTION(BlueprintCallable, Category = "Vehicles")
	bool RemoveOwnedVehicle(const FGuid& VehicleInstanceID);

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	TArray<FMGSaveVehicleData> GetOwnedVehicles() const;

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FMGSaveVehicleData GetVehicleData(const FGuid& VehicleInstanceID) const;

	UFUNCTION(BlueprintCallable, Category = "Vehicles")
	bool UpdateVehicleData(const FMGSaveVehicleData& VehicleData);

	UFUNCTION(BlueprintCallable, Category = "Vehicles")
	void SetCurrentVehicle(const FGuid& VehicleInstanceID);

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FGuid GetCurrentVehicleID() const;

	UFUNCTION(BlueprintPure, Category = "Vehicles")
	FMGSaveVehicleData GetCurrentVehicle() const;

	// ==========================================
	// PROGRESSION DATA
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void CompleteRace(FName TrackID, bool bWon);

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void CompleteMission(FName MissionID);

	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsMissionComplete(FName MissionID) const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void UnlockArea(FName AreaID);

	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsAreaUnlocked(FName AreaID) const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void UnlockAchievement(FName AchievementID);

	UFUNCTION(BlueprintPure, Category = "Progression")
	bool IsAchievementUnlocked(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void UpdateAchievementProgress(FName AchievementID, int32 Progress);

	UFUNCTION(BlueprintPure, Category = "Progression")
	int32 GetAchievementProgress(FName AchievementID) const;

	UFUNCTION(BlueprintCallable, Category = "Progression")
	void SetPersonalBest(const FMGTrackRecord& Record);

	UFUNCTION(BlueprintPure, Category = "Progression")
	FMGTrackRecord GetPersonalBest(FName TrackID, EMGRaceType RaceType) const;

	// ==========================================
	// INVENTORY
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddPartToInventory(FName PartID, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemovePartFromInventory(FName PartID, int32 Quantity = 1);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	int32 GetPartQuantity(FName PartID) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	TMap<FName, int32> GetAllInventoryParts() const;

	// ==========================================
	// SETTINGS
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetGameSetting(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintPure, Category = "Settings")
	FString GetGameSetting(const FString& Key, const FString& DefaultValue = TEXT("")) const;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMetricUnits(bool bMetric);

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool GetMetricUnits() const;

	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetManualTransmission(bool bManual);

	UFUNCTION(BlueprintPure, Category = "Settings")
	bool GetManualTransmission() const;

	// ==========================================
	// AUTO-SAVE CONFIGURATION
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "AutoSave")
	void SetAutoSaveEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "AutoSave")
	bool IsAutoSaveEnabled() const { return bAutoSaveEnabled; }

	UFUNCTION(BlueprintCallable, Category = "AutoSave")
	void SetAutoSaveInterval(float IntervalMinutes);

	UFUNCTION(BlueprintPure, Category = "AutoSave")
	float GetAutoSaveInterval() const { return AutoSaveIntervalMinutes; }

	// ==========================================
	// NEW GAME
	// ==========================================

	UFUNCTION(BlueprintCallable, Category = "NewGame")
	void CreateNewGame(int32 SlotIndex, const FString& PlayerName);

	UFUNCTION(BlueprintPure, Category = "NewGame")
	FMGSaveGameData GetDefaultSaveData() const;

protected:
	FString GetSaveSlotName(int32 SlotIndex) const;
	void GatherCurrentGameState(FMGSaveGameData& OutData);
	void ApplyLoadedGameState(const FMGSaveGameData& Data);
	void OnAutoSaveTimerTick();

private:
	// Current state
	UPROPERTY()
	FMGSaveGameData CurrentSaveData;

	UPROPERTY()
	int32 CurrentSlotIndex = -1;

	UPROPERTY()
	bool bHasUnsavedChanges = false;

	// Configuration
	UPROPERTY()
	int32 MaxSaveSlots = 10;

	UPROPERTY()
	int32 QuickSaveSlotIndex = 0;

	UPROPERTY()
	int32 AutoSaveSlotIndex = 9;

	UPROPERTY()
	bool bAutoSaveEnabled = true;

	UPROPERTY()
	float AutoSaveIntervalMinutes = 5.0f;

	// Timer
	FTimerHandle AutoSaveTimerHandle;

	// Save version for migration
	static constexpr int32 CurrentSaveVersion = 1;
};

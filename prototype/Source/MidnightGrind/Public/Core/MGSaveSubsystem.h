// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/MGPartsCatalog.h"
#include "Racing/MGRaceModeSubsystem.h"
#include "MGSaveSubsystem.generated.h"

/**
 * Save slot metadata
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

/**
 * Saved player profile data
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

/**
 * Saved vehicle data
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

/**
 * Saved progression data
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

/**
 * Saved rival/social data
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

/**
 * Complete save game data
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
};

/**
 * Delegates
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveStarted, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveCompleted, int32, SlotIndex, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadStarted, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLoadCompleted, int32, SlotIndex, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutoSave, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveSlotDeleted, int32, SlotIndex, bool, bSuccess);

/**
 * Save/Load System Subsystem
 *
 * Handles all game persistence including player progress,
 * vehicle builds, and game settings.
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

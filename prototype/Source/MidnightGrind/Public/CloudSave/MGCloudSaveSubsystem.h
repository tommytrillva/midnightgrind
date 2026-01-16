// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCloudSaveSubsystem.generated.h"

/**
 * Save Data Type
 */
UENUM(BlueprintType)
enum class EMGSaveDataType : uint8
{
	PlayerProfile		UMETA(DisplayName = "Player Profile"),
	GameProgress		UMETA(DisplayName = "Game Progress"),
	Achievements		UMETA(DisplayName = "Achievements"),
	Vehicles			UMETA(DisplayName = "Vehicles"),
	Customization		UMETA(DisplayName = "Customization"),
	Settings			UMETA(DisplayName = "Settings"),
	Statistics			UMETA(DisplayName = "Statistics"),
	Social				UMETA(DisplayName = "Social"),
	All					UMETA(DisplayName = "All")
};

/**
 * Sync Status
 */
UENUM(BlueprintType)
enum class EMGCloudSyncStatus : uint8
{
	Synced				UMETA(DisplayName = "Synced"),
	Syncing				UMETA(DisplayName = "Syncing"),
	PendingUpload		UMETA(DisplayName = "Pending Upload"),
	PendingDownload		UMETA(DisplayName = "Pending Download"),
	Conflict			UMETA(DisplayName = "Conflict"),
	Error				UMETA(DisplayName = "Error"),
	Offline				UMETA(DisplayName = "Offline")
};

/**
 * Conflict Resolution Strategy
 */
UENUM(BlueprintType)
enum class EMGConflictResolution : uint8
{
	UseLocal			UMETA(DisplayName = "Use Local"),
	UseCloud			UMETA(DisplayName = "Use Cloud"),
	UseMostRecent		UMETA(DisplayName = "Use Most Recent"),
	Merge				UMETA(DisplayName = "Merge"),
	AskUser				UMETA(DisplayName = "Ask User")
};

/**
 * Save Slot Info
 */
USTRUCT(BlueprintType)
struct FMGSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 PlayerLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 TotalPlayTime = 0; // In seconds

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	float CompletionPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FDateTime LastSaveTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FDateTime LastCloudSyncTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	bool bIsEmpty = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	bool bIsCorrupted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 SaveVersion = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString DeviceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString Platform;
};

/**
 * Cloud Save Metadata
 */
USTRUCT(BlueprintType)
struct FMGCloudSaveMetadata
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString CloudSaveID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString PlayerID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FDateTime CloudTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int32 CloudVersion = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int64 DataSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString Checksum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString OriginDevice;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString OriginPlatform;
};

/**
 * Save Conflict Data
 */
USTRUCT(BlueprintType)
struct FMGSaveConflict
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
	FMGSaveSlotInfo LocalSave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
	FMGCloudSaveMetadata CloudSave;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
	EMGSaveDataType DataType = EMGSaveDataType::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
	FDateTime LocalTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conflict")
	FDateTime CloudTimestamp;
};

/**
 * Sync Progress
 */
USTRUCT(BlueprintType)
struct FMGSyncProgress
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	EMGSaveDataType CurrentDataType = EMGSaveDataType::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 TotalItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int32 ProcessedItems = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int64 TotalBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	int64 TransferredBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	float ProgressPercent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
	bool bIsUploading = false;
};

/**
 * Auto-Save Settings
 */
USTRUCT(BlueprintType)
struct FMGAutoSaveSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	float IntervalMinutes = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	bool bSaveAfterRace = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	bool bSaveOnPurchase = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	bool bSaveOnLevelUp = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	bool bSaveOnAchievement = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AutoSave")
	bool bCloudSyncOnAutoSave = false;
};

/**
 * Backup Info
 */
USTRUCT(BlueprintType)
struct FMGSaveBackup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	FString BackupID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	FDateTime BackupTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	FString Reason; // "Manual", "Pre-Update", "Auto"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	int64 DataSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backup")
	bool bIsCloudBackup = false;
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCloudSyncStatusChanged, EMGCloudSyncStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCloudSyncProgress, const FMGSyncProgress&, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveConflictDetected, const FMGSaveConflict&, Conflict);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAutoSaveTriggered);

/**
 * Cloud Save Subsystem
 * Manages local saves, cloud synchronization, and backup/restore
 */
UCLASS()
class MIDNIGHTGRIND_API UMGCloudSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ==========================================
	// EVENTS
	// ==========================================

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoadCompleted OnLoadCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCloudSyncStatusChanged OnCloudSyncStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCloudSyncProgress OnCloudSyncProgress;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnSaveConflictDetected OnSaveConflictDetected;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAutoSaveTriggered OnAutoSaveTriggered;

	// ==========================================
	// LOCAL SAVE/LOAD
	// ==========================================

	/** Save game to slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveGame(int32 SlotIndex = 0);

	/** Save specific data type */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool SaveDataType(EMGSaveDataType DataType, int32 SlotIndex = 0);

	/** Load game from slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadGame(int32 SlotIndex = 0);

	/** Load specific data type */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool LoadDataType(EMGSaveDataType DataType, int32 SlotIndex = 0);

	/** Delete save slot */
	UFUNCTION(BlueprintCallable, Category = "Save")
	bool DeleteSaveSlot(int32 SlotIndex);

	/** Get save slot info */
	UFUNCTION(BlueprintCallable, Category = "Save")
	FMGSaveSlotInfo GetSaveSlotInfo(int32 SlotIndex) const;

	/** Get all save slots */
	UFUNCTION(BlueprintPure, Category = "Save")
	TArray<FMGSaveSlotInfo> GetAllSaveSlots() const;

	/** Does save slot exist */
	UFUNCTION(BlueprintPure, Category = "Save")
	bool DoesSaveSlotExist(int32 SlotIndex) const;

	/** Get current slot index */
	UFUNCTION(BlueprintPure, Category = "Save")
	int32 GetCurrentSlotIndex() const { return CurrentSlotIndex; }

	/** Get max save slots */
	UFUNCTION(BlueprintPure, Category = "Save")
	int32 GetMaxSaveSlots() const { return MaxSaveSlots; }

	// ==========================================
	// CLOUD SYNC
	// ==========================================

	/** Sync with cloud */
	UFUNCTION(BlueprintCallable, Category = "Cloud")
	void SyncWithCloud();

	/** Upload to cloud */
	UFUNCTION(BlueprintCallable, Category = "Cloud")
	void UploadToCloud(int32 SlotIndex = 0);

	/** Download from cloud */
	UFUNCTION(BlueprintCallable, Category = "Cloud")
	void DownloadFromCloud(int32 SlotIndex = 0);

	/** Get cloud sync status */
	UFUNCTION(BlueprintPure, Category = "Cloud")
	EMGCloudSyncStatus GetCloudSyncStatus() const { return CloudSyncStatus; }

	/** Get cloud save metadata */
	UFUNCTION(BlueprintCallable, Category = "Cloud")
	FMGCloudSaveMetadata GetCloudSaveMetadata() const;

	/** Is cloud sync enabled */
	UFUNCTION(BlueprintPure, Category = "Cloud")
	bool IsCloudSyncEnabled() const { return bCloudSyncEnabled; }

	/** Set cloud sync enabled */
	UFUNCTION(BlueprintCallable, Category = "Cloud")
	void SetCloudSyncEnabled(bool bEnabled);

	/** Get last sync time */
	UFUNCTION(BlueprintPure, Category = "Cloud")
	FDateTime GetLastCloudSyncTime() const { return LastCloudSyncTime; }

	/** Is online */
	UFUNCTION(BlueprintPure, Category = "Cloud")
	bool IsOnline() const;

	// ==========================================
	// CONFLICT RESOLUTION
	// ==========================================

	/** Resolve conflict */
	UFUNCTION(BlueprintCallable, Category = "Conflict")
	void ResolveConflict(EMGConflictResolution Resolution);

	/** Get current conflict */
	UFUNCTION(BlueprintPure, Category = "Conflict")
	FMGSaveConflict GetCurrentConflict() const { return CurrentConflict; }

	/** Has pending conflict */
	UFUNCTION(BlueprintPure, Category = "Conflict")
	bool HasPendingConflict() const { return bHasConflict; }

	/** Set default conflict resolution */
	UFUNCTION(BlueprintCallable, Category = "Conflict")
	void SetDefaultConflictResolution(EMGConflictResolution Resolution);

	// ==========================================
	// AUTO-SAVE
	// ==========================================

	/** Get auto-save settings */
	UFUNCTION(BlueprintPure, Category = "AutoSave")
	FMGAutoSaveSettings GetAutoSaveSettings() const { return AutoSaveSettings; }

	/** Set auto-save settings */
	UFUNCTION(BlueprintCallable, Category = "AutoSave")
	void SetAutoSaveSettings(const FMGAutoSaveSettings& Settings);

	/** Trigger auto-save check */
	UFUNCTION(BlueprintCallable, Category = "AutoSave")
	void TriggerAutoSaveCheck();

	/** Notify event for auto-save triggers */
	UFUNCTION(BlueprintCallable, Category = "AutoSave")
	void NotifyAutoSaveEvent(EMGSaveDataType DataType);

	// ==========================================
	// BACKUP & RESTORE
	// ==========================================

	/** Create backup */
	UFUNCTION(BlueprintCallable, Category = "Backup")
	bool CreateBackup(const FString& Reason = TEXT("Manual"));

	/** Restore from backup */
	UFUNCTION(BlueprintCallable, Category = "Backup")
	bool RestoreFromBackup(const FString& BackupID);

	/** Get all backups */
	UFUNCTION(BlueprintPure, Category = "Backup")
	TArray<FMGSaveBackup> GetAllBackups() const;

	/** Delete backup */
	UFUNCTION(BlueprintCallable, Category = "Backup")
	bool DeleteBackup(const FString& BackupID);

	/** Delete old backups (keep N most recent) */
	UFUNCTION(BlueprintCallable, Category = "Backup")
	void CleanupOldBackups(int32 KeepCount = 5);

	// ==========================================
	// IMPORT/EXPORT
	// ==========================================

	/** Export save to file */
	UFUNCTION(BlueprintCallable, Category = "Export")
	bool ExportSaveToFile(int32 SlotIndex, const FString& FilePath);

	/** Import save from file */
	UFUNCTION(BlueprintCallable, Category = "Export")
	bool ImportSaveFromFile(const FString& FilePath, int32 TargetSlotIndex);

	/** Get export data as string (for sharing) */
	UFUNCTION(BlueprintCallable, Category = "Export")
	FString GetExportDataAsString(int32 SlotIndex) const;

	/** Import from string */
	UFUNCTION(BlueprintCallable, Category = "Export")
	bool ImportFromString(const FString& Data, int32 TargetSlotIndex);

	// ==========================================
	// VALIDATION
	// ==========================================

	/** Validate save data integrity */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool ValidateSaveData(int32 SlotIndex) const;

	/** Repair corrupted save */
	UFUNCTION(BlueprintCallable, Category = "Validation")
	bool RepairSaveData(int32 SlotIndex);

	/** Calculate checksum */
	UFUNCTION(BlueprintPure, Category = "Validation")
	FString CalculateChecksum(int32 SlotIndex) const;

protected:
	// ==========================================
	// DATA
	// ==========================================

	/** Current save slot */
	UPROPERTY()
	int32 CurrentSlotIndex = 0;

	/** Max save slots */
	UPROPERTY()
	int32 MaxSaveSlots = 3;

	/** Save slot information */
	UPROPERTY()
	TArray<FMGSaveSlotInfo> SaveSlots;

	/** Cloud sync status */
	UPROPERTY()
	EMGCloudSyncStatus CloudSyncStatus = EMGCloudSyncStatus::Offline;

	/** Cloud sync enabled */
	UPROPERTY()
	bool bCloudSyncEnabled = true;

	/** Last cloud sync time */
	UPROPERTY()
	FDateTime LastCloudSyncTime;

	/** Cloud metadata */
	UPROPERTY()
	FMGCloudSaveMetadata CloudMetadata;

	/** Current conflict */
	UPROPERTY()
	FMGSaveConflict CurrentConflict;

	/** Has pending conflict */
	UPROPERTY()
	bool bHasConflict = false;

	/** Default conflict resolution */
	UPROPERTY()
	EMGConflictResolution DefaultConflictResolution = EMGConflictResolution::AskUser;

	/** Auto-save settings */
	UPROPERTY()
	FMGAutoSaveSettings AutoSaveSettings;

	/** Backups */
	UPROPERTY()
	TArray<FMGSaveBackup> Backups;

	/** Device ID */
	UPROPERTY()
	FString DeviceID;

	/** Platform */
	UPROPERTY()
	FString Platform;

	/** Last auto-save time */
	UPROPERTY()
	FDateTime LastAutoSaveTime;

	/** Timer handle for auto-save */
	FTimerHandle AutoSaveTimerHandle;

	/** Is currently saving */
	UPROPERTY()
	bool bIsSaving = false;

	/** Is currently loading */
	UPROPERTY()
	bool bIsLoading = false;

	/** Is currently syncing */
	UPROPERTY()
	bool bIsSyncing = false;

	// ==========================================
	// INTERNAL
	// ==========================================

	/** Initialize save slots */
	void InitializeSaveSlots();

	/** Get save file path */
	FString GetSaveFilePath(int32 SlotIndex, EMGSaveDataType DataType) const;

	/** Get backup folder path */
	FString GetBackupFolderPath() const;

	/** Serialize save data */
	TArray<uint8> SerializeSaveData(int32 SlotIndex, EMGSaveDataType DataType) const;

	/** Deserialize save data */
	bool DeserializeSaveData(const TArray<uint8>& Data, int32 SlotIndex, EMGSaveDataType DataType);

	/** Write data to file */
	bool WriteDataToFile(const FString& FilePath, const TArray<uint8>& Data);

	/** Read data from file */
	bool ReadDataFromFile(const FString& FilePath, TArray<uint8>& OutData);

	/** Compress data */
	TArray<uint8> CompressData(const TArray<uint8>& Data) const;

	/** Decompress data */
	TArray<uint8> DecompressData(const TArray<uint8>& CompressedData) const;

	/** Encrypt data */
	TArray<uint8> EncryptData(const TArray<uint8>& Data) const;

	/** Decrypt data */
	TArray<uint8> DecryptData(const TArray<uint8>& EncryptedData) const;

	/** Update save slot info */
	void UpdateSaveSlotInfo(int32 SlotIndex);

	/** Handle cloud upload complete */
	void OnCloudUploadComplete(bool bSuccess);

	/** Handle cloud download complete */
	void OnCloudDownloadComplete(bool bSuccess, const TArray<uint8>& Data);

	/** Check for conflicts */
	bool CheckForConflicts(int32 SlotIndex);

	/** Merge save data */
	bool MergeSaveData(int32 SlotIndex);

	/** Auto-save tick */
	void AutoSaveTick();

	/** Generate device ID */
	FString GenerateDeviceID() const;

	/** Get platform name */
	FString GetPlatformName() const;
};

// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGCloudSaveSubsystem.h
 * @brief Cloud Save and Synchronization Subsystem for Midnight Grind
 *
 * This subsystem manages both local save operations and cloud synchronization,
 * enabling players to access their progress across multiple devices and platforms.
 *
 * ## Overview
 * The Cloud Save Subsystem provides:
 * - **Local Save Management**: Multiple save slots with metadata (playtime, level, etc.)
 * - **Cloud Synchronization**: Upload/download saves to remote servers
 * - **Conflict Resolution**: Handle cases where local and cloud saves diverge
 * - **Auto-Save**: Configurable automatic save triggers based on game events
 * - **Backup & Restore**: Create and restore save backups for data safety
 * - **Import/Export**: Share saves between players or platforms
 *
 * ## Sync Architecture
 * The system follows a "local-first" approach:
 * 1. All saves are written locally first for reliability
 * 2. Cloud sync happens in the background when online
 * 3. Conflicts are detected by comparing timestamps and versions
 * 4. Resolution can be automatic or prompt the user
 *
 * ## Usage Example
 * @code
 * UMGCloudSaveSubsystem* CloudSave = GameInstance->GetSubsystem<UMGCloudSaveSubsystem>();
 *
 * // Save locally
 * CloudSave->SaveGame(0);  // Save to slot 0
 *
 * // Sync with cloud when ready
 * if (CloudSave->IsOnline())
 * {
 *     CloudSave->SyncWithCloud();
 * }
 *
 * // Handle conflicts via delegate
 * CloudSave->OnSaveConflictDetected.AddDynamic(this, &MyClass::HandleConflict);
 * @endcode
 *
 * @see UMGSaveManagerSubsystem For high-level save coordination
 * @see UMGDataMigrationSubsystem For handling save format changes between versions
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCloudSaveSubsystem.generated.h"

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @enum EMGSaveDataType
 * @brief Categories of save data that can be saved/synced independently.
 *
 * Allows granular control over what data to save or sync, which is useful
 * for optimizing bandwidth and handling partial updates.
 */
UENUM(BlueprintType)
enum class EMGSaveDataType : uint8
{
	PlayerProfile		UMETA(DisplayName = "Player Profile"),    ///< Basic player info: name, avatar, level
	GameProgress		UMETA(DisplayName = "Game Progress"),     ///< Campaign progress, unlocked content
	Achievements		UMETA(DisplayName = "Achievements"),      ///< Achievement unlock status and progress
	Vehicles			UMETA(DisplayName = "Vehicles"),          ///< Owned vehicles and their upgrades
	Customization		UMETA(DisplayName = "Customization"),     ///< Visual customizations, paints, decals
	Settings			UMETA(DisplayName = "Settings"),          ///< Game settings, controls, preferences
	Statistics			UMETA(DisplayName = "Statistics"),        ///< Gameplay stats: races, wins, distances
	Social				UMETA(DisplayName = "Social"),            ///< Friends list, crew membership
	All					UMETA(DisplayName = "All")                ///< All data types combined
};

/**
 * @enum EMGCloudSyncStatus
 * @brief Current state of cloud synchronization.
 *
 * Use this to display sync status to the player (e.g., cloud icon in UI)
 * and to determine when it's safe to perform operations.
 */
UENUM(BlueprintType)
enum class EMGCloudSyncStatus : uint8
{
	Synced				UMETA(DisplayName = "Synced"),           ///< Local and cloud data match
	Syncing				UMETA(DisplayName = "Syncing"),          ///< Currently uploading or downloading
	PendingUpload		UMETA(DisplayName = "Pending Upload"),   ///< Local changes need to be uploaded
	PendingDownload		UMETA(DisplayName = "Pending Download"), ///< Cloud has newer data to download
	Conflict			UMETA(DisplayName = "Conflict"),         ///< Both local and cloud have changes
	Error				UMETA(DisplayName = "Error"),            ///< Sync failed due to an error
	Offline				UMETA(DisplayName = "Offline")           ///< No network connection available
};

/**
 * @enum EMGConflictResolution
 * @brief Strategy for resolving conflicts between local and cloud saves.
 *
 * Conflicts occur when both local and cloud data have changed since the last sync.
 * This enum defines how to resolve such situations.
 */
UENUM(BlueprintType)
enum class EMGConflictResolution : uint8
{
	UseLocal			UMETA(DisplayName = "Use Local"),        ///< Overwrite cloud with local data
	UseCloud			UMETA(DisplayName = "Use Cloud"),        ///< Overwrite local with cloud data
	UseMostRecent		UMETA(DisplayName = "Use Most Recent"),  ///< Automatically pick based on timestamp
	Merge				UMETA(DisplayName = "Merge"),            ///< Attempt intelligent merge of both
	AskUser				UMETA(DisplayName = "Ask User")          ///< Prompt player to choose
};

// ============================================================================
// DATA STRUCTURES - SAVE SLOT INFORMATION
// ============================================================================

/**
 * @struct FMGSaveSlotInfo
 * @brief Metadata about a save slot, displayed in the save/load UI.
 *
 * Contains summary information about a save without loading the full save data.
 * Used to populate save slot selection screens with preview information.
 */
USTRUCT(BlueprintType)
struct FMGSaveSlotInfo
{
	GENERATED_BODY()

	/// Index of this save slot (0, 1, 2, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 SlotIndex = 0;

	/// Display name for this slot (e.g., "Save Slot 1" or custom name)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString SlotName;

	/// Player's display name at time of save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString PlayerName;

	/// Player's level at time of save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 PlayerLevel = 1;

	/// Total play time in seconds for this save file
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 TotalPlayTime = 0;

	/// Game completion percentage (0.0 to 100.0)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	float CompletionPercent = 0.0f;

	/// Timestamp of last local save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FDateTime LastSaveTime;

	/// Timestamp of last successful cloud sync
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FDateTime LastCloudSyncTime;

	/// True if this slot has no save data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	bool bIsEmpty = true;

	/// True if save data failed validation (may need repair)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	bool bIsCorrupted = false;

	/// Data format version (for migration purposes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	int32 SaveVersion = 1;

	/// Unique identifier of the device that created this save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString DeviceID;

	/// Platform name (e.g., "Windows", "PlayStation", "Xbox")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveSlot")
	FString Platform;
};

// ============================================================================
// DATA STRUCTURES - CLOUD METADATA
// ============================================================================

/**
 * @struct FMGCloudSaveMetadata
 * @brief Information about a save stored in the cloud.
 *
 * Contains metadata retrieved from the cloud server without downloading
 * the full save data. Used for conflict detection and UI display.
 */
USTRUCT(BlueprintType)
struct FMGCloudSaveMetadata
{
	GENERATED_BODY()

	/// Unique identifier for this cloud save record
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString CloudSaveID;

	/// Player account ID that owns this save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString PlayerID;

	/// Server timestamp of when this save was uploaded
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FDateTime CloudTimestamp;

	/// Version counter incremented on each cloud upload
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int32 CloudVersion = 0;

	/// Size of the save data in bytes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	int64 DataSize = 0;

	/// Hash for data integrity verification
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString Checksum;

	/// Device ID that last uploaded this save
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CloudSave")
	FString OriginDevice;

	/// Platform that last uploaded this save
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

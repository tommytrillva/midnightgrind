// Copyright Midnight Grind. All Rights Reserved.

#include "CloudSave/MGCloudSaveSubsystem.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Misc/SecureHash.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UMGCloudSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DeviceID = GenerateDeviceID();
	Platform = GetPlatformName();

	InitializeSaveSlots();

	// Start auto-save timer
	if (AutoSaveSettings.bEnabled && AutoSaveSettings.IntervalMinutes > 0)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				AutoSaveTimerHandle,
				this,
				&UMGCloudSaveSubsystem::AutoSaveTick,
				AutoSaveSettings.IntervalMinutes * 60.0f,
				true
			);
		}
	}

	// Initial cloud check
	if (bCloudSyncEnabled && IsOnline())
	{
		SyncWithCloud();
	}
}

void UMGCloudSaveSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
	}

	// Final save
	if (!bIsSaving)
	{
		SaveGame(CurrentSlotIndex);
	}

	Super::Deinitialize();
}

// ==========================================
// LOCAL SAVE/LOAD
// ==========================================

bool UMGCloudSaveSubsystem::SaveGame(int32 SlotIndex)
{
	if (bIsSaving || SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	bIsSaving = true;

	bool bSuccess = true;

	// Save all data types
	bSuccess &= SaveDataType(EMGSaveDataType::PlayerProfile, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::GameProgress, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::Achievements, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::Vehicles, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::Customization, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::Settings, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::Statistics, SlotIndex);
	bSuccess &= SaveDataType(EMGSaveDataType::Social, SlotIndex);

	UpdateSaveSlotInfo(SlotIndex);
	CurrentSlotIndex = SlotIndex;

	bIsSaving = false;
	OnSaveCompleted.Broadcast(bSuccess);

	// Cloud sync if enabled
	if (bSuccess && bCloudSyncEnabled && AutoSaveSettings.bCloudSyncOnAutoSave && IsOnline())
	{
		UploadToCloud(SlotIndex);
	}

	return bSuccess;
}

bool UMGCloudSaveSubsystem::SaveDataType(EMGSaveDataType DataType, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	FString FilePath = GetSaveFilePath(SlotIndex, DataType);
	TArray<uint8> Data = SerializeSaveData(SlotIndex, DataType);

	if (Data.Num() == 0)
	{
		return true; // Empty data is valid
	}

	// Compress and encrypt
	TArray<uint8> CompressedData = CompressData(Data);
	TArray<uint8> EncryptedData = EncryptData(CompressedData);

	return WriteDataToFile(FilePath, EncryptedData);
}

bool UMGCloudSaveSubsystem::LoadGame(int32 SlotIndex)
{
	if (bIsLoading || SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	if (!DoesSaveSlotExist(SlotIndex))
	{
		OnLoadCompleted.Broadcast(false);
		return false;
	}

	bIsLoading = true;

	bool bSuccess = true;

	// Load all data types
	bSuccess &= LoadDataType(EMGSaveDataType::PlayerProfile, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::GameProgress, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::Achievements, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::Vehicles, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::Customization, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::Settings, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::Statistics, SlotIndex);
	bSuccess &= LoadDataType(EMGSaveDataType::Social, SlotIndex);

	if (bSuccess)
	{
		CurrentSlotIndex = SlotIndex;
	}

	bIsLoading = false;
	OnLoadCompleted.Broadcast(bSuccess);

	return bSuccess;
}

bool UMGCloudSaveSubsystem::LoadDataType(EMGSaveDataType DataType, int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	FString FilePath = GetSaveFilePath(SlotIndex, DataType);

	TArray<uint8> EncryptedData;
	if (!ReadDataFromFile(FilePath, EncryptedData))
	{
		return true; // Missing file is ok, use defaults
	}

	// Decrypt and decompress
	TArray<uint8> CompressedData = DecryptData(EncryptedData);
	TArray<uint8> Data = DecompressData(CompressedData);

	return DeserializeSaveData(Data, SlotIndex, DataType);
}

bool UMGCloudSaveSubsystem::DeleteSaveSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= MaxSaveSlots)
	{
		return false;
	}

	// Create backup before deletion
	CreateBackup(TEXT("Pre-Delete"));

	// Delete all data type files
	for (int32 i = 0; i <= (int32)EMGSaveDataType::Social; i++)
	{
		FString FilePath = GetSaveFilePath(SlotIndex, (EMGSaveDataType)i);
		IFileManager::Get().Delete(*FilePath);
	}

	// Update slot info
	if (SlotIndex < SaveSlots.Num())
	{
		SaveSlots[SlotIndex] = FMGSaveSlotInfo();
		SaveSlots[SlotIndex].SlotIndex = SlotIndex;
		SaveSlots[SlotIndex].bIsEmpty = true;
	}

	return true;
}

FMGSaveSlotInfo UMGCloudSaveSubsystem::GetSaveSlotInfo(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < SaveSlots.Num())
	{
		return SaveSlots[SlotIndex];
	}
	return FMGSaveSlotInfo();
}

TArray<FMGSaveSlotInfo> UMGCloudSaveSubsystem::GetAllSaveSlots() const
{
	return SaveSlots;
}

bool UMGCloudSaveSubsystem::DoesSaveSlotExist(int32 SlotIndex) const
{
	if (SlotIndex >= 0 && SlotIndex < SaveSlots.Num())
	{
		return !SaveSlots[SlotIndex].bIsEmpty;
	}
	return false;
}

// ==========================================
// CLOUD SYNC
// ==========================================

void UMGCloudSaveSubsystem::SyncWithCloud()
{
	if (bIsSyncing || !bCloudSyncEnabled || !IsOnline())
	{
		return;
	}

	bIsSyncing = true;
	CloudSyncStatus = EMGCloudSyncStatus::Syncing;
	OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);

	// Check for conflicts
	if (CheckForConflicts(CurrentSlotIndex))
	{
		// Conflict detected, wait for resolution
		return;
	}

	// Compare timestamps
	FMGSaveSlotInfo LocalSave = GetSaveSlotInfo(CurrentSlotIndex);

	if (CloudMetadata.CloudTimestamp > LocalSave.LastSaveTime)
	{
		// Cloud is newer, download
		DownloadFromCloud(CurrentSlotIndex);
	}
	else if (LocalSave.LastSaveTime > CloudMetadata.CloudTimestamp)
	{
		// Local is newer, upload
		UploadToCloud(CurrentSlotIndex);
	}
	else
	{
		// Already synced
		CloudSyncStatus = EMGCloudSyncStatus::Synced;
		OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
		bIsSyncing = false;
	}
}

void UMGCloudSaveSubsystem::UploadToCloud(int32 SlotIndex)
{
	if (!bCloudSyncEnabled || !IsOnline())
	{
		CloudSyncStatus = EMGCloudSyncStatus::Offline;
		OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
		return;
	}

	CloudSyncStatus = EMGCloudSyncStatus::PendingUpload;
	OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);

	// Serialize all data
	TArray<uint8> AllData = SerializeSaveData(SlotIndex, EMGSaveDataType::All);

	// Save to local "cloud" directory (simulates cloud storage)
	// In production, this would use platform-specific cloud APIs:
	// Steam: ISteamRemoteStorage
	// Xbox: XGameSaveSubmitUpdate
	// PlayStation: SaveData API

	FString CloudDir = FPaths::ProjectSavedDir() / TEXT("CloudSaves");
	IFileManager::Get().MakeDirectory(*CloudDir, true);

	FString CloudFilePath = CloudDir / FString::Printf(TEXT("Slot_%d.cloudsave"), SlotIndex);

	// Create cloud save with metadata header
	FBufferArchive CloudArchive;
	int32 Version = 1;
	FString DeviceIdCopy = DeviceID;
	int64 UploadTimestamp = FDateTime::UtcNow().GetTicks();
	int32 DataSize = AllData.Num();

	CloudArchive << Version;
	CloudArchive << DeviceIdCopy;
	CloudArchive << UploadTimestamp;
	CloudArchive << DataSize;
	CloudArchive.Append(AllData);

	// Calculate checksum
	FSHAHash Hash;
	FSHA1::HashBuffer(AllData.GetData(), AllData.Num(), Hash.Hash);
	FString Checksum = Hash.ToString();
	CloudArchive << Checksum;

	FMGSyncProgress Progress;
	Progress.bIsUploading = true;
	Progress.TotalBytes = CloudArchive.Num();

	// Write to file with progress simulation
	bool bSuccess = FFileHelper::SaveArrayToFile(CloudArchive, *CloudFilePath);

	Progress.TransferredBytes = bSuccess ? CloudArchive.Num() : 0;
	Progress.ProgressPercent = bSuccess ? 100.0f : 0.0f;
	OnCloudSyncProgress.Broadcast(Progress);

	// Update metadata
	if (bSuccess)
	{
		CloudMetadata.LastSyncTime = FDateTime::UtcNow();
		CloudMetadata.CloudDataSize = CloudArchive.Num();
		CloudMetadata.bHasCloudData = true;
	}

	OnCloudUploadComplete(bSuccess);
}

void UMGCloudSaveSubsystem::DownloadFromCloud(int32 SlotIndex)
{
	if (!bCloudSyncEnabled || !IsOnline())
	{
		CloudSyncStatus = EMGCloudSyncStatus::Offline;
		OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
		return;
	}

	CloudSyncStatus = EMGCloudSyncStatus::PendingDownload;
	OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);

	// Load from local "cloud" directory
	FString CloudDir = FPaths::ProjectSavedDir() / TEXT("CloudSaves");
	FString CloudFilePath = CloudDir / FString::Printf(TEXT("Slot_%d.cloudsave"), SlotIndex);

	TArray<uint8> CloudFileData;
	TArray<uint8> CloudData;
	bool bSuccess = false;

	FMGSyncProgress Progress;
	Progress.bIsUploading = false;

	if (FFileHelper::LoadFileToArray(CloudFileData, *CloudFilePath))
	{
		Progress.TotalBytes = CloudFileData.Num();

		// Parse cloud save format
		FMemoryReader CloudArchive(CloudFileData, true);

		int32 Version;
		FString SaveDeviceId;
		int64 UploadTimestamp;
		int32 DataSize;

		CloudArchive << Version;
		CloudArchive << SaveDeviceId;
		CloudArchive << UploadTimestamp;
		CloudArchive << DataSize;

		if (Version == 1 && DataSize > 0 && DataSize <= CloudFileData.Num())
		{
			// Extract save data
			CloudData.SetNum(DataSize);
			CloudArchive.Serialize(CloudData.GetData(), DataSize);

			// Read and verify checksum
			FString StoredChecksum;
			CloudArchive << StoredChecksum;

			FSHAHash Hash;
			FSHA1::HashBuffer(CloudData.GetData(), CloudData.Num(), Hash.Hash);
			FString CalculatedChecksum = Hash.ToString();

			if (StoredChecksum == CalculatedChecksum)
			{
				bSuccess = true;
				CloudMetadata.LastSyncTime = FDateTime(UploadTimestamp);
				CloudMetadata.CloudDataSize = DataSize;
				CloudMetadata.bHasCloudData = true;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("CloudSave: Checksum mismatch for slot %d"), SlotIndex);
			}
		}

		Progress.TransferredBytes = bSuccess ? CloudFileData.Num() : 0;
		Progress.ProgressPercent = bSuccess ? 100.0f : 0.0f;
	}
	else
	{
		Progress.TotalBytes = 0;
		Progress.TransferredBytes = 0;
		Progress.ProgressPercent = 0.0f;
		UE_LOG(LogTemp, Log, TEXT("CloudSave: No cloud data found for slot %d"), SlotIndex);
	}

	OnCloudSyncProgress.Broadcast(Progress);
	OnCloudDownloadComplete(bSuccess, CloudData);
}

FMGCloudSaveMetadata UMGCloudSaveSubsystem::GetCloudSaveMetadata() const
{
	return CloudMetadata;
}

void UMGCloudSaveSubsystem::SetCloudSyncEnabled(bool bEnabled)
{
	bCloudSyncEnabled = bEnabled;

	if (bEnabled && IsOnline())
	{
		SyncWithCloud();
	}
	else if (!bEnabled)
	{
		CloudSyncStatus = EMGCloudSyncStatus::Offline;
		OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
	}
}

bool UMGCloudSaveSubsystem::IsOnline() const
{
	// Would check actual network status
	return true;
}

// ==========================================
// CONFLICT RESOLUTION
// ==========================================

void UMGCloudSaveSubsystem::ResolveConflict(EMGConflictResolution Resolution)
{
	if (!bHasConflict)
	{
		return;
	}

	switch (Resolution)
	{
		case EMGConflictResolution::UseLocal:
			UploadToCloud(CurrentSlotIndex);
			break;

		case EMGConflictResolution::UseCloud:
			DownloadFromCloud(CurrentSlotIndex);
			break;

		case EMGConflictResolution::UseMostRecent:
			if (CurrentConflict.LocalTimestamp > CurrentConflict.CloudTimestamp)
			{
				UploadToCloud(CurrentSlotIndex);
			}
			else
			{
				DownloadFromCloud(CurrentSlotIndex);
			}
			break;

		case EMGConflictResolution::Merge:
			MergeSaveData(CurrentSlotIndex);
			break;

		default:
			break;
	}

	bHasConflict = false;
}

void UMGCloudSaveSubsystem::SetDefaultConflictResolution(EMGConflictResolution Resolution)
{
	DefaultConflictResolution = Resolution;
}

// ==========================================
// AUTO-SAVE
// ==========================================

void UMGCloudSaveSubsystem::SetAutoSaveSettings(const FMGAutoSaveSettings& Settings)
{
	AutoSaveSettings = Settings;

	// Update timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);

		if (Settings.bEnabled && Settings.IntervalMinutes > 0)
		{
			World->GetTimerManager().SetTimer(
				AutoSaveTimerHandle,
				this,
				&UMGCloudSaveSubsystem::AutoSaveTick,
				Settings.IntervalMinutes * 60.0f,
				true
			);
		}
	}
}

void UMGCloudSaveSubsystem::TriggerAutoSaveCheck()
{
	if (!AutoSaveSettings.bEnabled || bIsSaving)
	{
		return;
	}

	SaveGame(CurrentSlotIndex);
	LastAutoSaveTime = FDateTime::Now();
	OnAutoSaveTriggered.Broadcast();
}

void UMGCloudSaveSubsystem::NotifyAutoSaveEvent(EMGSaveDataType DataType)
{
	if (!AutoSaveSettings.bEnabled)
	{
		return;
	}

	bool bShouldSave = false;

	switch (DataType)
	{
		case EMGSaveDataType::GameProgress:
			bShouldSave = AutoSaveSettings.bSaveAfterRace;
			break;
		case EMGSaveDataType::Vehicles:
		case EMGSaveDataType::Customization:
			bShouldSave = AutoSaveSettings.bSaveOnPurchase;
			break;
		case EMGSaveDataType::Achievements:
			bShouldSave = AutoSaveSettings.bSaveOnAchievement;
			break;
		case EMGSaveDataType::PlayerProfile:
			bShouldSave = AutoSaveSettings.bSaveOnLevelUp;
			break;
		default:
			break;
	}

	if (bShouldSave)
	{
		SaveDataType(DataType, CurrentSlotIndex);
	}
}

// ==========================================
// BACKUP & RESTORE
// ==========================================

bool UMGCloudSaveSubsystem::CreateBackup(const FString& Reason)
{
	FString BackupFolder = GetBackupFolderPath();
	IFileManager::Get().MakeDirectory(*BackupFolder, true);

	FMGSaveBackup Backup;
	Backup.BackupID = FGuid::NewGuid().ToString();
	Backup.BackupTime = FDateTime::Now();
	Backup.Reason = Reason;

	FString BackupPath = FPaths::Combine(BackupFolder, Backup.BackupID);
	IFileManager::Get().MakeDirectory(*BackupPath, true);

	int64 TotalSize = 0;

	// Copy all save files
	for (int32 i = 0; i <= (int32)EMGSaveDataType::Social; i++)
	{
		FString SourcePath = GetSaveFilePath(CurrentSlotIndex, (EMGSaveDataType)i);
		FString DestPath = FPaths::Combine(BackupPath, FPaths::GetCleanFilename(SourcePath));

		if (IFileManager::Get().FileExists(*SourcePath))
		{
			IFileManager::Get().Copy(*DestPath, *SourcePath);
			TotalSize += IFileManager::Get().FileSize(*SourcePath);
		}
	}

	Backup.DataSize = TotalSize;
	Backups.Add(Backup);

	// Cleanup old backups
	CleanupOldBackups(10);

	return true;
}

bool UMGCloudSaveSubsystem::RestoreFromBackup(const FString& BackupID)
{
	FMGSaveBackup* Backup = nullptr;
	for (FMGSaveBackup& B : Backups)
	{
		if (B.BackupID == BackupID)
		{
			Backup = &B;
			break;
		}
	}

	if (!Backup)
	{
		return false;
	}

	// Create backup of current state first
	CreateBackup(TEXT("Pre-Restore"));

	FString BackupPath = FPaths::Combine(GetBackupFolderPath(), BackupID);

	// Restore all files
	for (int32 i = 0; i <= (int32)EMGSaveDataType::Social; i++)
	{
		FString DestPath = GetSaveFilePath(CurrentSlotIndex, (EMGSaveDataType)i);
		FString SourcePath = FPaths::Combine(BackupPath, FPaths::GetCleanFilename(DestPath));

		if (IFileManager::Get().FileExists(*SourcePath))
		{
			IFileManager::Get().Copy(*DestPath, *SourcePath);
		}
	}

	// Reload
	return LoadGame(CurrentSlotIndex);
}

TArray<FMGSaveBackup> UMGCloudSaveSubsystem::GetAllBackups() const
{
	return Backups;
}

bool UMGCloudSaveSubsystem::DeleteBackup(const FString& BackupID)
{
	for (int32 i = 0; i < Backups.Num(); i++)
	{
		if (Backups[i].BackupID == BackupID)
		{
			FString BackupPath = FPaths::Combine(GetBackupFolderPath(), BackupID);
			IFileManager::Get().DeleteDirectory(*BackupPath, false, true);
			Backups.RemoveAt(i);
			return true;
		}
	}
	return false;
}

void UMGCloudSaveSubsystem::CleanupOldBackups(int32 KeepCount)
{
	if (Backups.Num() <= KeepCount)
	{
		return;
	}

	// Sort by time (oldest first)
	Backups.Sort([](const FMGSaveBackup& A, const FMGSaveBackup& B) {
		return A.BackupTime < B.BackupTime;
	});

	// Delete oldest backups
	while (Backups.Num() > KeepCount)
	{
		DeleteBackup(Backups[0].BackupID);
	}
}

// ==========================================
// IMPORT/EXPORT
// ==========================================

bool UMGCloudSaveSubsystem::ExportSaveToFile(int32 SlotIndex, const FString& FilePath)
{
	TArray<uint8> Data = SerializeSaveData(SlotIndex, EMGSaveDataType::All);
	return FFileHelper::SaveArrayToFile(Data, *FilePath);
}

bool UMGCloudSaveSubsystem::ImportSaveFromFile(const FString& FilePath, int32 TargetSlotIndex)
{
	TArray<uint8> Data;
	if (!FFileHelper::LoadFileToArray(Data, *FilePath))
	{
		return false;
	}

	// Create backup first
	CreateBackup(TEXT("Pre-Import"));

	return DeserializeSaveData(Data, TargetSlotIndex, EMGSaveDataType::All);
}

FString UMGCloudSaveSubsystem::GetExportDataAsString(int32 SlotIndex) const
{
	TArray<uint8> Data = SerializeSaveData(SlotIndex, EMGSaveDataType::All);
	return FBase64::Encode(Data);
}

bool UMGCloudSaveSubsystem::ImportFromString(const FString& Data, int32 TargetSlotIndex)
{
	TArray<uint8> DecodedData;
	if (!FBase64::Decode(Data, DecodedData))
	{
		return false;
	}

	CreateBackup(TEXT("Pre-Import"));
	return DeserializeSaveData(DecodedData, TargetSlotIndex, EMGSaveDataType::All);
}

// ==========================================
// VALIDATION
// ==========================================

bool UMGCloudSaveSubsystem::ValidateSaveData(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= SaveSlots.Num())
	{
		return false;
	}

	// Check if files exist and are valid
	for (int32 i = 0; i <= (int32)EMGSaveDataType::Social; i++)
	{
		FString FilePath = GetSaveFilePath(SlotIndex, (EMGSaveDataType)i);

		if (IFileManager::Get().FileExists(*FilePath))
		{
			TArray<uint8> Data;
			if (!FFileHelper::LoadFileToArray(Data, *FilePath))
			{
				return false;
			}

			// Basic validation - check for minimum size and header
			if (Data.Num() < 16)
			{
				return false;
			}
		}
	}

	return true;
}

bool UMGCloudSaveSubsystem::RepairSaveData(int32 SlotIndex)
{
	// Try to restore from most recent backup
	if (Backups.Num() > 0)
	{
		// Sort by time (most recent first)
		Backups.Sort([](const FMGSaveBackup& A, const FMGSaveBackup& B) {
			return A.BackupTime > B.BackupTime;
		});

		return RestoreFromBackup(Backups[0].BackupID);
	}

	return false;
}

FString UMGCloudSaveSubsystem::CalculateChecksum(int32 SlotIndex) const
{
	TArray<uint8> AllData = SerializeSaveData(SlotIndex, EMGSaveDataType::All);
	return FMD5::HashBytes(AllData.GetData(), AllData.Num());
}

// ==========================================
// INTERNAL
// ==========================================

void UMGCloudSaveSubsystem::InitializeSaveSlots()
{
	SaveSlots.SetNum(MaxSaveSlots);

	for (int32 i = 0; i < MaxSaveSlots; i++)
	{
		SaveSlots[i].SlotIndex = i;
		SaveSlots[i].SlotName = FString::Printf(TEXT("Slot %d"), i + 1);

		// Check if save exists
		FString FilePath = GetSaveFilePath(i, EMGSaveDataType::PlayerProfile);
		if (IFileManager::Get().FileExists(*FilePath))
		{
			SaveSlots[i].bIsEmpty = false;
			SaveSlots[i].LastSaveTime = IFileManager::Get().GetTimeStamp(*FilePath);
			SaveSlots[i].DeviceID = DeviceID;
			SaveSlots[i].Platform = Platform;

			// Validate
			SaveSlots[i].bIsCorrupted = !ValidateSaveData(i);
		}
	}
}

FString UMGCloudSaveSubsystem::GetSaveFilePath(int32 SlotIndex, EMGSaveDataType DataType) const
{
	FString SaveDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"));

	FString TypeName;
	switch (DataType)
	{
		case EMGSaveDataType::PlayerProfile: TypeName = TEXT("Profile"); break;
		case EMGSaveDataType::GameProgress: TypeName = TEXT("Progress"); break;
		case EMGSaveDataType::Achievements: TypeName = TEXT("Achievements"); break;
		case EMGSaveDataType::Vehicles: TypeName = TEXT("Vehicles"); break;
		case EMGSaveDataType::Customization: TypeName = TEXT("Customization"); break;
		case EMGSaveDataType::Settings: TypeName = TEXT("Settings"); break;
		case EMGSaveDataType::Statistics: TypeName = TEXT("Statistics"); break;
		case EMGSaveDataType::Social: TypeName = TEXT("Social"); break;
		default: TypeName = TEXT("All"); break;
	}

	return FPaths::Combine(SaveDir, FString::Printf(TEXT("Slot%d_%s.sav"), SlotIndex, *TypeName));
}

FString UMGCloudSaveSubsystem::GetBackupFolderPath() const
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SaveGames"), TEXT("Backups"));
}

TArray<uint8> UMGCloudSaveSubsystem::SerializeSaveData(int32 SlotIndex, EMGSaveDataType DataType) const
{
	FBufferArchive Archive;

	// Write header
	int32 Version = 1;
	Archive << Version;

	// Would serialize actual game data here based on DataType
	// This would interface with other subsystems to get the data

	return TArray<uint8>(Archive.GetData(), Archive.Num());
}

bool UMGCloudSaveSubsystem::DeserializeSaveData(const TArray<uint8>& Data, int32 SlotIndex, EMGSaveDataType DataType)
{
	if (Data.Num() == 0)
	{
		return true;
	}

	FMemoryReader Archive(Data);

	// Read header
	int32 Version;
	Archive << Version;

	if (Version != 1)
	{
		return false;
	}

	// Would deserialize and apply data to game systems
	return true;
}

bool UMGCloudSaveSubsystem::WriteDataToFile(const FString& FilePath, const TArray<uint8>& Data)
{
	// Ensure directory exists
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(FilePath), true);
	return FFileHelper::SaveArrayToFile(Data, *FilePath);
}

bool UMGCloudSaveSubsystem::ReadDataFromFile(const FString& FilePath, TArray<uint8>& OutData)
{
	return FFileHelper::LoadFileToArray(OutData, *FilePath);
}

TArray<uint8> UMGCloudSaveSubsystem::CompressData(const TArray<uint8>& Data) const
{
	// Would use compression (FCompression::CompressMemory)
	return Data;
}

TArray<uint8> UMGCloudSaveSubsystem::DecompressData(const TArray<uint8>& CompressedData) const
{
	// Would use decompression
	return CompressedData;
}

TArray<uint8> UMGCloudSaveSubsystem::EncryptData(const TArray<uint8>& Data) const
{
	// Would use encryption (AES)
	return Data;
}

TArray<uint8> UMGCloudSaveSubsystem::DecryptData(const TArray<uint8>& EncryptedData) const
{
	// Would use decryption
	return EncryptedData;
}

void UMGCloudSaveSubsystem::UpdateSaveSlotInfo(int32 SlotIndex)
{
	if (SlotIndex >= 0 && SlotIndex < SaveSlots.Num())
	{
		SaveSlots[SlotIndex].bIsEmpty = false;
		SaveSlots[SlotIndex].LastSaveTime = FDateTime::Now();
		SaveSlots[SlotIndex].DeviceID = DeviceID;
		SaveSlots[SlotIndex].Platform = Platform;
		SaveSlots[SlotIndex].SaveVersion++;

		// Would update other fields from game state
	}
}

void UMGCloudSaveSubsystem::OnCloudUploadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		LastCloudSyncTime = FDateTime::Now();
		CloudSyncStatus = EMGCloudSyncStatus::Synced;

		// Update cloud metadata
		CloudMetadata.CloudTimestamp = FDateTime::Now();
		CloudMetadata.OriginDevice = DeviceID;
		CloudMetadata.OriginPlatform = Platform;

		if (CurrentSlotIndex < SaveSlots.Num())
		{
			SaveSlots[CurrentSlotIndex].LastCloudSyncTime = FDateTime::Now();
		}
	}
	else
	{
		CloudSyncStatus = EMGCloudSyncStatus::Error;
	}

	bIsSyncing = false;
	OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
}

void UMGCloudSaveSubsystem::OnCloudDownloadComplete(bool bSuccess, const TArray<uint8>& Data)
{
	if (bSuccess && Data.Num() > 0)
	{
		DeserializeSaveData(Data, CurrentSlotIndex, EMGSaveDataType::All);
		LastCloudSyncTime = FDateTime::Now();
		CloudSyncStatus = EMGCloudSyncStatus::Synced;

		if (CurrentSlotIndex < SaveSlots.Num())
		{
			SaveSlots[CurrentSlotIndex].LastCloudSyncTime = FDateTime::Now();
		}
	}
	else if (!bSuccess)
	{
		CloudSyncStatus = EMGCloudSyncStatus::Error;
	}

	bIsSyncing = false;
	OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
}

bool UMGCloudSaveSubsystem::CheckForConflicts(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= SaveSlots.Num())
	{
		return false;
	}

	FMGSaveSlotInfo& LocalSave = SaveSlots[SlotIndex];

	// Check if cloud and local have diverged
	if (!LocalSave.bIsEmpty &&
		CloudMetadata.CloudTimestamp > FDateTime::MinValue() &&
		LocalSave.DeviceID != CloudMetadata.OriginDevice &&
		LocalSave.LastSaveTime > LocalSave.LastCloudSyncTime)
	{
		// Conflict detected
		CurrentConflict.LocalSave = LocalSave;
		CurrentConflict.CloudSave = CloudMetadata;
		CurrentConflict.LocalTimestamp = LocalSave.LastSaveTime;
		CurrentConflict.CloudTimestamp = CloudMetadata.CloudTimestamp;

		bHasConflict = true;
		CloudSyncStatus = EMGCloudSyncStatus::Conflict;
		OnCloudSyncStatusChanged.Broadcast(CloudSyncStatus);
		OnSaveConflictDetected.Broadcast(CurrentConflict);

		if (DefaultConflictResolution != EMGConflictResolution::AskUser)
		{
			ResolveConflict(DefaultConflictResolution);
			return false;
		}

		return true;
	}

	return false;
}

bool UMGCloudSaveSubsystem::MergeSaveData(int32 SlotIndex)
{
	// Create backup before merge
	CreateBackup(TEXT("Pre-Merge"));

	// Would implement intelligent merge logic here
	// For example: take highest level, combine unlocked items, etc.

	// After merge, upload
	UploadToCloud(SlotIndex);

	return true;
}

void UMGCloudSaveSubsystem::AutoSaveTick()
{
	TriggerAutoSaveCheck();
}

FString UMGCloudSaveSubsystem::GenerateDeviceID() const
{
	// Would generate a unique device identifier
	return FPlatformMisc::GetDeviceId();
}

FString UMGCloudSaveSubsystem::GetPlatformName() const
{
#if PLATFORM_WINDOWS
	return TEXT("Windows");
#elif PLATFORM_MAC
	return TEXT("Mac");
#elif PLATFORM_LINUX
	return TEXT("Linux");
#elif PLATFORM_XBOXONE
	return TEXT("Xbox");
#elif PLATFORM_PS4 || PLATFORM_PS5
	return TEXT("PlayStation");
#elif PLATFORM_SWITCH
	return TEXT("Switch");
#else
	return TEXT("Unknown");
#endif
}

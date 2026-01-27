// Copyright Midnight Grind. All Rights Reserved.

// MGCrossProgressionSubsystem.h
// Cross-Platform Progression System - Syncs progress across platforms and devices
// Midnight Grind - Y2K Arcade Street Racing

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrossProgressionSubsystem.generated.h"

// Forward declarations
class UWorld;

// ============================================================================
// Enums
// ============================================================================

UENUM(BlueprintType)
enum class EPlatformType : uint8
{
    PC_Steam,
    PC_Epic,
    PC_GOG,
    PlayStation5,
    PlayStation4,
    XboxSeriesX,
    XboxOne,
    NintendoSwitch,
    Mobile_iOS,
    Mobile_Android,
    Cloud,
    Unknown
};

UENUM(BlueprintType)
enum class ESyncStatus : uint8
{
    NotSynced,
    Syncing,
    Synced,
    SyncFailed,
    ConflictDetected,
    Offline,
    Disabled
};

UENUM(BlueprintType)
enum class EConflictResolution : uint8
{
    UseLocal,
    UseCloud,
    UseMostRecent,
    UseHighestProgress,
    MergeData,
    AskUser
};

UENUM(BlueprintType)
enum class EProgressionDataType : uint8
{
    PlayerProfile,
    VehicleCollection,
    GarageData,
    Currency,
    Reputation,
    Unlocks,
    Achievements,
    Statistics,
    Settings,
    Cosmetics,
    BattlePass,
    Challenges,
    Social,
    All
};

UENUM(BlueprintType)
enum class ETransferDirection : uint8
{
    Upload,
    Download,
    Bidirectional
};

UENUM(BlueprintType)
enum class EBackupFrequency : uint8
{
    Never,
    OnMajorChange,
    Hourly,
    Daily,
    Weekly
};

UENUM(BlueprintType)
enum class ELinkStatus : uint8
{
    NotLinked,
    Pending,
    Linked,
    Expired,
    Revoked
};

UENUM(BlueprintType)
enum class ECloudProvider : uint8
{
    Default,
    Steam,
    PlayStation,
    Xbox,
    Nintendo,
    Epic,
    Custom
};

// ============================================================================
// Structs
// ============================================================================

USTRUCT(BlueprintType)
struct FPlatformAccount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AccountId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType Platform = EPlatformType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELinkStatus LinkStatus = ELinkStatus::NotLinked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LinkedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastSyncedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPrimary = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AvatarUrl;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> PlatformMetadata;
};

USTRUCT(BlueprintType)
struct FUnifiedPlayerId
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid UnifiedId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Email;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FPlatformAccount> LinkedAccounts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType PrimaryPlatform = EPlatformType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastLoginAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEmailVerified = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCrossProgressionEnabled = true;
};

USTRUCT(BlueprintType)
struct FProgressionSnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid SnapshotId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType SourcePlatform = EPlatformType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlayerLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalXP = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 Currency = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 PremiumCurrency = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ReputationLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 VehicleCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UnlockCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AchievementCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TotalPlaytimeHours = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalRaces = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalWins = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BattlePassTier = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DataHash;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 DataVersion = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> CustomData;
};

USTRUCT(BlueprintType)
struct FSyncConflict
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ConflictId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EProgressionDataType DataType = EProgressionDataType::All;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProgressionSnapshot LocalData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProgressionSnapshot CloudData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ConflictDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime DetectedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bResolved = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConflictResolution ResolutionUsed = EConflictResolution::AskUser;
};

USTRUCT(BlueprintType)
struct FSyncOperation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid OperationId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EProgressionDataType DataType = EProgressionDataType::All;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETransferDirection Direction = ETransferDirection::Bidirectional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESyncStatus Status = ESyncStatus::NotSynced;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime StartedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CompletedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BytesTransferred = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RetryCount = 0;
};

USTRUCT(BlueprintType)
struct FCloudBackup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid BackupId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BackupName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreatedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType SourcePlatform = EPlatformType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProgressionSnapshot Snapshot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 BackupSizeBytes = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAutomatic = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BackupReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EProgressionDataType> IncludedDataTypes;
};

USTRUCT(BlueprintType)
struct FCrossSaveSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bAutoSync = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AutoSyncIntervalMinutes = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSyncOnStartup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSyncOnShutdown = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSyncOnMajorProgress = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConflictResolution DefaultConflictResolution = EConflictResolution::UseMostRecent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBackupFrequency BackupFrequency = EBackupFrequency::Daily;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxBackupsToKeep = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSyncAchievements = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSyncCosmetics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSyncSettings = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWifiOnlySync = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECloudProvider PreferredProvider = ECloudProvider::Default;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EProgressionDataType> SyncedDataTypes;
};

USTRUCT(BlueprintType)
struct FPlatformEntitlement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EntitlementId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EntitlementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType Platform = EPlatformType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsOwned = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCrossPlatform = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime PurchasedAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime ExpiresAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UnlockedContent;
};

USTRUCT(BlueprintType)
struct FProgressTransferResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FProgressionSnapshot TransferredData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSyncConflict> Conflicts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ItemsTransferred = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ItemsSkipped = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransferDurationSeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrossProgressionStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalSyncs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SuccessfulSyncs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FailedSyncs = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ConflictsResolved = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int64 TotalDataTransferred = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AverageSyncTimeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastSuccessfulSync;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastFailedSync;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 LinkedPlatformCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> SyncCountByPlatform;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> ErrorCountByType;
};

USTRUCT(BlueprintType)
struct FDeviceInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DeviceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DeviceName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EPlatformType Platform = EPlatformType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OSVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GameVersion;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastActiveAt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCurrentDevice = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTrusted = true;
};

// ============================================================================
// Delegates
// ============================================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyncStarted, const FSyncOperation&, Operation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSyncProgress, const FSyncOperation&, Operation, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSyncCompleted, const FSyncOperation&, Operation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSyncFailed, const FSyncOperation&, Operation, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConflictDetected, const FSyncConflict&, Conflict);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConflictResolved, const FSyncConflict&, Conflict, EConflictResolution, Resolution);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccountLinked, const FPlatformAccount&, Account);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAccountUnlinked, const FPlatformAccount&, Account);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackupCreated, const FCloudBackup&, Backup);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackupRestored, const FCloudBackup&, Backup);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProgressTransferred, EPlatformType, FromPlatform, EPlatformType, ToPlatform);

// ============================================================================
// Main Subsystem
// ============================================================================

UCLASS()
class MIDNIGHTGRIND_API UMGCrossProgressionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ========================================================================
    // Account Management
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Account")
    void CreateUnifiedAccount(const FString& Email, const FString& DisplayName);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Account")
    void LoginWithEmail(const FString& Email, const FString& Password);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Account")
    void LoginWithPlatform(EPlatformType Platform);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Account")
    void Logout();

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Account")
    FUnifiedPlayerId GetUnifiedPlayer() const { return UnifiedPlayer; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Account")
    bool IsLoggedIn() const { return bIsLoggedIn; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Account")
    EPlatformType GetCurrentPlatform() const { return CurrentPlatform; }

    // ========================================================================
    // Platform Linking
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Linking")
    void LinkPlatformAccount(EPlatformType Platform);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Linking")
    void UnlinkPlatformAccount(EPlatformType Platform);

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Linking")
    TArray<FPlatformAccount> GetLinkedAccounts() const;

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Linking")
    bool IsPlatformLinked(EPlatformType Platform) const;

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Linking")
    void SetPrimaryPlatform(EPlatformType Platform);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Linking")
    FString GenerateLinkCode(EPlatformType Platform);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Linking")
    void RedeemLinkCode(const FString& Code);

    // ========================================================================
    // Sync Operations
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Sync")
    void SyncAllData();

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Sync")
    void SyncDataType(EProgressionDataType DataType);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Sync")
    void UploadProgress();

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Sync")
    void DownloadProgress();

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Sync")
    void CancelSync();

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Sync")
    ESyncStatus GetSyncStatus() const { return CurrentSyncStatus; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Sync")
    FSyncOperation GetCurrentOperation() const { return CurrentOperation; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Sync")
    bool IsSyncing() const { return CurrentSyncStatus == ESyncStatus::Syncing; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Sync")
    FDateTime GetLastSyncTime() const { return LastSyncTime; }

    // ========================================================================
    // Conflict Resolution
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Conflicts")
    void ResolveConflict(const FGuid& ConflictId, EConflictResolution Resolution);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Conflicts")
    void ResolveAllConflicts(EConflictResolution Resolution);

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Conflicts")
    TArray<FSyncConflict> GetPendingConflicts() const { return PendingConflicts; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Conflicts")
    bool HasPendingConflicts() const { return PendingConflicts.Num() > 0; }

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Conflicts")
    FSyncConflict CompareSnapshots(const FProgressionSnapshot& Local, const FProgressionSnapshot& Cloud);

    // ========================================================================
    // Backups
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Backup")
    void CreateBackup(const FString& BackupName, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Backup")
    void RestoreBackup(const FGuid& BackupId);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Backup")
    void DeleteBackup(const FGuid& BackupId);

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Backup")
    TArray<FCloudBackup> GetAvailableBackups() const { return CloudBackups; }

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Backup")
    FCloudBackup GetLatestBackup() const;

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Backup")
    void CleanupOldBackups();

    // ========================================================================
    // Progress Snapshots
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Snapshot")
    FProgressionSnapshot CreateLocalSnapshot();

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Snapshot")
    FProgressionSnapshot FetchCloudSnapshot();

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Snapshot")
    void ApplySnapshot(const FProgressionSnapshot& Snapshot);

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Snapshot")
    FProgressionSnapshot GetLastKnownCloudState() const { return LastKnownCloudState; }

    // ========================================================================
    // Progress Transfer
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Transfer")
    FProgressTransferResult TransferProgressToPlatform(EPlatformType TargetPlatform);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Transfer")
    FProgressTransferResult ImportProgressFromPlatform(EPlatformType SourcePlatform);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Transfer")
    void MergeProgressFromAllPlatforms();

    // ========================================================================
    // Entitlements
    // ========================================================================

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Entitlements")
    void SyncEntitlements();

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Entitlements")
    TArray<FPlatformEntitlement> GetAllEntitlements() const { return AllEntitlements; }

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Entitlements")
    TArray<FPlatformEntitlement> GetCrossPlatformEntitlements() const;

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Entitlements")
    bool HasEntitlement(const FString& EntitlementId) const;

    // ========================================================================
    // Device Management
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Devices")
    TArray<FDeviceInfo> GetLinkedDevices() const { return LinkedDevices; }

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Devices")
    void RemoveDevice(const FString& DeviceId);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Devices")
    void TrustDevice(const FString& DeviceId);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Devices")
    void UntrustDevice(const FString& DeviceId);

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Devices")
    FDeviceInfo GetCurrentDeviceInfo() const;

    // ========================================================================
    // Settings
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Settings")
    FCrossSaveSettings GetSettings() const { return Settings; }

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Settings")
    void UpdateSettings(const FCrossSaveSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Settings")
    void EnableCrossProgression(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Settings")
    void SetAutoSync(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Settings")
    void SetConflictResolutionMode(EConflictResolution Mode);

    // ========================================================================
    // Statistics
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Stats")
    FCrossProgressionStats GetStats() const { return Stats; }

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Stats")
    void ResetStats();

    // ========================================================================
    // Utilities
    // ========================================================================

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Utility")
    FString GetPlatformName(EPlatformType Platform) const;

    UFUNCTION(BlueprintPure, Category = "CrossProgression|Utility")
    bool IsNetworkAvailable() const;

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Utility")
    void RefreshNetworkStatus();

    UFUNCTION(BlueprintCallable, Category = "CrossProgression|Utility")
    FString CalculateDataHash(const FProgressionSnapshot& Snapshot) const;

    // ========================================================================
    // Events
    // ========================================================================

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnSyncStarted OnSyncStarted;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnSyncProgress OnSyncProgress;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnSyncCompleted OnSyncCompleted;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnSyncFailed OnSyncFailed;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnConflictDetected OnConflictDetected;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnConflictResolved OnConflictResolved;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnAccountLinked OnAccountLinked;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnAccountUnlinked OnAccountUnlinked;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnBackupCreated OnBackupCreated;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnBackupRestored OnBackupRestored;

    UPROPERTY(BlueprintAssignable, Category = "CrossProgression|Events")
    FOnProgressTransferred OnProgressTransferred;

protected:
    void InitializeCurrentPlatform();
    void InitializeDefaultSettings();
    void LoadLocalData();
    void SaveLocalData();

    void StartAutoSyncTimer();
    void StopAutoSyncTimer();
    void OnAutoSyncTick();

    void PerformSync(ETransferDirection Direction, EProgressionDataType DataType);
    void HandleSyncResult(bool bSuccess, const FString& ErrorMessage);

    FSyncConflict DetectConflict(const FProgressionSnapshot& Local, const FProgressionSnapshot& Cloud, EProgressionDataType DataType);
    FProgressionSnapshot ResolveConflictData(const FSyncConflict& Conflict, EConflictResolution Resolution);

    void CreateAutomaticBackup();

private:
    UPROPERTY()
    FUnifiedPlayerId UnifiedPlayer;

    UPROPERTY()
    EPlatformType CurrentPlatform;

    UPROPERTY()
    ESyncStatus CurrentSyncStatus;

    UPROPERTY()
    FSyncOperation CurrentOperation;

    UPROPERTY()
    FCrossSaveSettings Settings;

    UPROPERTY()
    FCrossProgressionStats Stats;

    UPROPERTY()
    FProgressionSnapshot LastKnownCloudState;

    UPROPERTY()
    FProgressionSnapshot LastLocalSnapshot;

    UPROPERTY()
    TArray<FSyncConflict> PendingConflicts;

    UPROPERTY()
    TArray<FCloudBackup> CloudBackups;

    UPROPERTY()
    TArray<FPlatformEntitlement> AllEntitlements;

    UPROPERTY()
    TArray<FDeviceInfo> LinkedDevices;

    UPROPERTY()
    FDateTime LastSyncTime;

    UPROPERTY()
    FDateTime LastBackupTime;

    UPROPERTY()
    bool bIsLoggedIn;

    UPROPERTY()
    bool bNetworkAvailable;

    UPROPERTY()
    bool bSyncInProgress;

    FTimerHandle AutoSyncTimer;
    FTimerHandle BackupTimer;

    TMap<FString, FString> PendingLinkCodes;
};

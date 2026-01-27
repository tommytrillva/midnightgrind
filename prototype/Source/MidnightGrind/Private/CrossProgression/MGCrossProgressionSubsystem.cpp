// MGCrossProgressionSubsystem.cpp
// Cross-Platform Progression System - Implementation
// Midnight Grind - Y2K Arcade Street Racing

#include "CrossProgression/MGCrossProgressionSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/SecureHash.h"

void UMGCrossProgressionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentSyncStatus = ESyncStatus::NotSynced;
    bIsLoggedIn = false;
    bNetworkAvailable = true;
    bSyncInProgress = false;

    InitializeCurrentPlatform();
    InitializeDefaultSettings();
    LoadLocalData();

    // Start auto-sync if enabled
    if (Settings.bAutoSync && Settings.bEnabled)
    {
        StartAutoSyncTimer();
    }

    UE_LOG(LogTemp, Log, TEXT("MGCrossProgressionSubsystem initialized on platform: %s"), *GetPlatformName(CurrentPlatform));
}

void UMGCrossProgressionSubsystem::Deinitialize()
{
    // Perform final sync if enabled
    if (Settings.bSyncOnShutdown && bIsLoggedIn && bNetworkAvailable)
    {
        UploadProgress();
    }

    StopAutoSyncTimer();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BackupTimer);
    }

    SaveLocalData();

    Super::Deinitialize();
}

void UMGCrossProgressionSubsystem::InitializeCurrentPlatform()
{
    // Detect current platform
#if PLATFORM_WINDOWS
    CurrentPlatform = EPlatformType::PC_Steam; // Default to Steam on PC
#elif PLATFORM_MAC
    CurrentPlatform = EPlatformType::PC_Steam;
#elif PLATFORM_LINUX
    CurrentPlatform = EPlatformType::PC_Steam;
#elif PLATFORM_PS5
    CurrentPlatform = EPlatformType::PlayStation5;
#elif PLATFORM_PS4
    CurrentPlatform = EPlatformType::PlayStation4;
#elif PLATFORM_XBOXONE
    CurrentPlatform = EPlatformType::XboxOne;
#elif PLATFORM_XSX
    CurrentPlatform = EPlatformType::XboxSeriesX;
#elif PLATFORM_SWITCH
    CurrentPlatform = EPlatformType::NintendoSwitch;
#elif PLATFORM_IOS
    CurrentPlatform = EPlatformType::Mobile_iOS;
#elif PLATFORM_ANDROID
    CurrentPlatform = EPlatformType::Mobile_Android;
#else
    CurrentPlatform = EPlatformType::Unknown;
#endif
}

void UMGCrossProgressionSubsystem::InitializeDefaultSettings()
{
    Settings.bEnabled = true;
    Settings.bAutoSync = true;
    Settings.AutoSyncIntervalMinutes = 15.0f;
    Settings.bSyncOnStartup = true;
    Settings.bSyncOnShutdown = true;
    Settings.bSyncOnMajorProgress = true;
    Settings.DefaultConflictResolution = EConflictResolution::UseMostRecent;
    Settings.BackupFrequency = EBackupFrequency::Daily;
    Settings.MaxBackupsToKeep = 10;
    Settings.bSyncAchievements = true;
    Settings.bSyncCosmetics = true;
    Settings.bSyncSettings = false;
    Settings.bWifiOnlySync = false;
    Settings.PreferredProvider = ECloudProvider::Default;

    // Default synced data types
    Settings.SyncedDataTypes = {
        EProgressionDataType::PlayerProfile,
        EProgressionDataType::VehicleCollection,
        EProgressionDataType::Currency,
        EProgressionDataType::Reputation,
        EProgressionDataType::Unlocks,
        EProgressionDataType::Achievements,
        EProgressionDataType::Statistics,
        EProgressionDataType::BattlePass
    };
}

void UMGCrossProgressionSubsystem::LoadLocalData()
{
    // Initialize stats
    Stats.TotalSyncs = 0;
    Stats.SuccessfulSyncs = 0;
    Stats.FailedSyncs = 0;
    Stats.ConflictsResolved = 0;
    Stats.TotalDataTransferred = 0;
    Stats.AverageSyncTimeSeconds = 0.0f;
    Stats.LinkedPlatformCount = 0;

    // Initialize unified player with default data
    UnifiedPlayer.UnifiedId = FGuid::NewGuid();
    UnifiedPlayer.DisplayName = TEXT("Player");
    UnifiedPlayer.PrimaryPlatform = CurrentPlatform;
    UnifiedPlayer.CreatedAt = FDateTime::Now();
    UnifiedPlayer.bCrossProgressionEnabled = true;

    // Register current device
    FDeviceInfo CurrentDevice;
    CurrentDevice.DeviceId = FGuid::NewGuid().ToString();
    CurrentDevice.DeviceName = FPlatformProcess::ComputerName();
    CurrentDevice.Platform = CurrentPlatform;
    CurrentDevice.GameVersion = TEXT("1.0.0");
    CurrentDevice.LastActiveAt = FDateTime::Now();
    CurrentDevice.bCurrentDevice = true;
    CurrentDevice.bTrusted = true;
    LinkedDevices.Add(CurrentDevice);
}

void UMGCrossProgressionSubsystem::SaveLocalData()
{
    // Save cross progression data placeholder
    UE_LOG(LogTemp, Log, TEXT("Saving cross progression data"));
}

void UMGCrossProgressionSubsystem::StartAutoSyncTimer()
{
    if (UWorld* World = GetWorld())
    {
        float IntervalSeconds = Settings.AutoSyncIntervalMinutes * 60.0f;
        World->GetTimerManager().SetTimer(AutoSyncTimer, this, &UMGCrossProgressionSubsystem::OnAutoSyncTick, IntervalSeconds, true);
    }
}

void UMGCrossProgressionSubsystem::StopAutoSyncTimer()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSyncTimer);
    }
}

void UMGCrossProgressionSubsystem::OnAutoSyncTick()
{
    if (Settings.bEnabled && Settings.bAutoSync && bIsLoggedIn && bNetworkAvailable && !bSyncInProgress)
    {
        SyncAllData();
    }
}

// ============================================================================
// Account Management
// ============================================================================

void UMGCrossProgressionSubsystem::CreateUnifiedAccount(const FString& Email, const FString& DisplayName)
{
    UnifiedPlayer.UnifiedId = FGuid::NewGuid();
    UnifiedPlayer.Email = Email;
    UnifiedPlayer.DisplayName = DisplayName;
    UnifiedPlayer.CreatedAt = FDateTime::Now();
    UnifiedPlayer.LastLoginAt = FDateTime::Now();
    UnifiedPlayer.PrimaryPlatform = CurrentPlatform;
    UnifiedPlayer.bCrossProgressionEnabled = true;
    UnifiedPlayer.bEmailVerified = false;

    // Link current platform
    FPlatformAccount CurrentAccount;
    CurrentAccount.AccountId = FGuid::NewGuid().ToString();
    CurrentAccount.DisplayName = DisplayName;
    CurrentAccount.Platform = CurrentPlatform;
    CurrentAccount.LinkStatus = ELinkStatus::Linked;
    CurrentAccount.LinkedAt = FDateTime::Now();
    CurrentAccount.bIsPrimary = true;

    UnifiedPlayer.LinkedAccounts.Add(CurrentAccount);

    bIsLoggedIn = true;
    Stats.LinkedPlatformCount = 1;

    UE_LOG(LogTemp, Log, TEXT("Created unified account for %s"), *DisplayName);
}

void UMGCrossProgressionSubsystem::LoginWithEmail(const FString& Email, const FString& Password)
{
    // Simulate login
    bIsLoggedIn = true;
    UnifiedPlayer.LastLoginAt = FDateTime::Now();

    if (Settings.bSyncOnStartup)
    {
        DownloadProgress();
    }

    UE_LOG(LogTemp, Log, TEXT("Logged in with email: %s"), *Email);
}

void UMGCrossProgressionSubsystem::LoginWithPlatform(EPlatformType Platform)
{
    // Simulate platform login
    bIsLoggedIn = true;
    UnifiedPlayer.LastLoginAt = FDateTime::Now();

    // Check if platform is already linked
    bool bFound = false;
    for (FPlatformAccount& Account : UnifiedPlayer.LinkedAccounts)
    {
        if (Account.Platform == Platform)
        {
            Account.LastSyncedAt = FDateTime::Now();
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        // Link this platform
        LinkPlatformAccount(Platform);
    }

    if (Settings.bSyncOnStartup)
    {
        DownloadProgress();
    }

    UE_LOG(LogTemp, Log, TEXT("Logged in with platform: %s"), *GetPlatformName(Platform));
}

void UMGCrossProgressionSubsystem::Logout()
{
    if (Settings.bSyncOnShutdown && bNetworkAvailable)
    {
        UploadProgress();
    }

    bIsLoggedIn = false;
    CurrentSyncStatus = ESyncStatus::NotSynced;
}

// ============================================================================
// Platform Linking
// ============================================================================

void UMGCrossProgressionSubsystem::LinkPlatformAccount(EPlatformType Platform)
{
    // Check if already linked
    if (IsPlatformLinked(Platform))
    {
        UE_LOG(LogTemp, Warning, TEXT("Platform %s is already linked"), *GetPlatformName(Platform));
        return;
    }

    FPlatformAccount NewAccount;
    NewAccount.AccountId = FGuid::NewGuid().ToString();
    NewAccount.Platform = Platform;
    NewAccount.LinkStatus = ELinkStatus::Linked;
    NewAccount.LinkedAt = FDateTime::Now();
    NewAccount.bIsPrimary = UnifiedPlayer.LinkedAccounts.Num() == 0;

    // Generate display name based on platform
    NewAccount.DisplayName = FString::Printf(TEXT("%s Player"), *GetPlatformName(Platform));

    UnifiedPlayer.LinkedAccounts.Add(NewAccount);
    Stats.LinkedPlatformCount++;

    OnAccountLinked.Broadcast(NewAccount);

    UE_LOG(LogTemp, Log, TEXT("Linked platform account: %s"), *GetPlatformName(Platform));
}

void UMGCrossProgressionSubsystem::UnlinkPlatformAccount(EPlatformType Platform)
{
    for (int32 i = UnifiedPlayer.LinkedAccounts.Num() - 1; i >= 0; i--)
    {
        if (UnifiedPlayer.LinkedAccounts[i].Platform == Platform)
        {
            FPlatformAccount UnlinkedAccount = UnifiedPlayer.LinkedAccounts[i];
            UnifiedPlayer.LinkedAccounts.RemoveAt(i);
            Stats.LinkedPlatformCount--;

            OnAccountUnlinked.Broadcast(UnlinkedAccount);

            UE_LOG(LogTemp, Log, TEXT("Unlinked platform account: %s"), *GetPlatformName(Platform));
            return;
        }
    }
}

TArray<FPlatformAccount> UMGCrossProgressionSubsystem::GetLinkedAccounts() const
{
    return UnifiedPlayer.LinkedAccounts;
}

bool UMGCrossProgressionSubsystem::IsPlatformLinked(EPlatformType Platform) const
{
    for (const FPlatformAccount& Account : UnifiedPlayer.LinkedAccounts)
    {
        if (Account.Platform == Platform && Account.LinkStatus == ELinkStatus::Linked)
        {
            return true;
        }
    }
    return false;
}

void UMGCrossProgressionSubsystem::SetPrimaryPlatform(EPlatformType Platform)
{
    for (FPlatformAccount& Account : UnifiedPlayer.LinkedAccounts)
    {
        Account.bIsPrimary = (Account.Platform == Platform);
    }
    UnifiedPlayer.PrimaryPlatform = Platform;
}

FString UMGCrossProgressionSubsystem::GenerateLinkCode(EPlatformType Platform)
{
    // Generate 6-character alphanumeric code
    FString Code;
    const FString Chars = TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

    for (int32 i = 0; i < 6; i++)
    {
        int32 Index = FMath::RandRange(0, Chars.Len() - 1);
        Code.AppendChar(Chars[Index]);
    }

    // Store code with expiration
    PendingLinkCodes.Add(Code, FString::Printf(TEXT("%d"), static_cast<int32>(Platform)));

    UE_LOG(LogTemp, Log, TEXT("Generated link code: %s for platform %s"), *Code, *GetPlatformName(Platform));

    return Code;
}

void UMGCrossProgressionSubsystem::RedeemLinkCode(const FString& Code)
{
    if (PendingLinkCodes.Contains(Code))
    {
        FString PlatformStr = PendingLinkCodes[Code];
        EPlatformType Platform = static_cast<EPlatformType>(FCString::Atoi(*PlatformStr));

        LinkPlatformAccount(Platform);
        PendingLinkCodes.Remove(Code);

        UE_LOG(LogTemp, Log, TEXT("Redeemed link code: %s"), *Code);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid or expired link code: %s"), *Code);
    }
}

// ============================================================================
// Sync Operations
// ============================================================================

void UMGCrossProgressionSubsystem::SyncAllData()
{
    if (!Settings.bEnabled || !bIsLoggedIn)
    {
        return;
    }

    PerformSync(ETransferDirection::Bidirectional, EProgressionDataType::All);
}

void UMGCrossProgressionSubsystem::SyncDataType(EProgressionDataType DataType)
{
    if (!Settings.bEnabled || !bIsLoggedIn)
    {
        return;
    }

    PerformSync(ETransferDirection::Bidirectional, DataType);
}

void UMGCrossProgressionSubsystem::UploadProgress()
{
    if (!Settings.bEnabled || !bIsLoggedIn)
    {
        return;
    }

    PerformSync(ETransferDirection::Upload, EProgressionDataType::All);
}

void UMGCrossProgressionSubsystem::DownloadProgress()
{
    if (!Settings.bEnabled || !bIsLoggedIn)
    {
        return;
    }

    PerformSync(ETransferDirection::Download, EProgressionDataType::All);
}

void UMGCrossProgressionSubsystem::CancelSync()
{
    if (bSyncInProgress)
    {
        bSyncInProgress = false;
        CurrentSyncStatus = ESyncStatus::NotSynced;
        CurrentOperation.Status = ESyncStatus::SyncFailed;
        CurrentOperation.ErrorMessage = TEXT("Sync cancelled by user");

        OnSyncFailed.Broadcast(CurrentOperation, CurrentOperation.ErrorMessage);
    }
}

void UMGCrossProgressionSubsystem::PerformSync(ETransferDirection Direction, EProgressionDataType DataType)
{
    if (bSyncInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Sync already in progress"));
        return;
    }

    if (!bNetworkAvailable)
    {
        CurrentSyncStatus = ESyncStatus::Offline;
        return;
    }

    bSyncInProgress = true;
    CurrentSyncStatus = ESyncStatus::Syncing;

    // Create sync operation
    CurrentOperation.OperationId = FGuid::NewGuid();
    CurrentOperation.DataType = DataType;
    CurrentOperation.Direction = Direction;
    CurrentOperation.Status = ESyncStatus::Syncing;
    CurrentOperation.Progress = 0.0f;
    CurrentOperation.StartedAt = FDateTime::Now();
    CurrentOperation.ErrorMessage = TEXT("");
    CurrentOperation.RetryCount = 0;

    OnSyncStarted.Broadcast(CurrentOperation);

    // Simulate sync process
    if (UWorld* World = GetWorld())
    {
        float SyncDuration = 1.0f + FMath::RandRange(0.0f, 1.0f); // 1-2 seconds

        // Progress updates
        TWeakObjectPtr<UMGCrossProgressionSubsystem> WeakThis(this);
        World->GetTimerManager().SetTimer(BackupTimer, [WeakThis]()
        {
            if (WeakThis.IsValid())
            {
                WeakThis->CurrentOperation.Progress += 0.25f;
                WeakThis->OnSyncProgress.Broadcast(WeakThis->CurrentOperation, WeakThis->CurrentOperation.Progress);
            }
        }, SyncDuration * 0.25f, true, 0.0f);

        // Completion
        World->GetTimerManager().SetTimerForNextTick([WeakThis, SyncDuration]()
        {
            if (!WeakThis.IsValid())
            {
                return;
            }
            if (UWorld* InnerWorld = WeakThis->GetWorld())
            {
                InnerWorld->GetTimerManager().SetTimer(WeakThis->AutoSyncTimer, [WeakThis]()
                {
                    if (WeakThis.IsValid())
                    {
                        // Simulate success (90% chance) or failure (10% chance)
                        bool bSuccess = FMath::RandRange(0.0f, 1.0f) < 0.9f;
                        WeakThis->HandleSyncResult(bSuccess, bSuccess ? TEXT("") : TEXT("Network timeout"));
                    }
                }, SyncDuration, false);
            }
        });
    }

    Stats.TotalSyncs++;
}

void UMGCrossProgressionSubsystem::HandleSyncResult(bool bSuccess, const FString& ErrorMessage)
{
    bSyncInProgress = false;
    CurrentOperation.CompletedAt = FDateTime::Now();

    float SyncDuration = (CurrentOperation.CompletedAt - CurrentOperation.StartedAt).GetTotalSeconds();

    if (bSuccess)
    {
        CurrentSyncStatus = ESyncStatus::Synced;
        CurrentOperation.Status = ESyncStatus::Synced;
        CurrentOperation.Progress = 1.0f;
        LastSyncTime = FDateTime::Now();

        Stats.SuccessfulSyncs++;
        Stats.LastSuccessfulSync = LastSyncTime;

        // Update local snapshot
        LastLocalSnapshot = CreateLocalSnapshot();
        LastKnownCloudState = LastLocalSnapshot;

        // Update average sync time
        float TotalSyncTime = Stats.AverageSyncTimeSeconds * (Stats.SuccessfulSyncs - 1);
        Stats.AverageSyncTimeSeconds = (TotalSyncTime + SyncDuration) / Stats.SuccessfulSyncs;

        // Update linked account sync times
        for (FPlatformAccount& Account : UnifiedPlayer.LinkedAccounts)
        {
            if (Account.Platform == CurrentPlatform)
            {
                Account.LastSyncedAt = LastSyncTime;
            }
        }

        OnSyncCompleted.Broadcast(CurrentOperation);
        UE_LOG(LogTemp, Log, TEXT("Sync completed successfully in %.2f seconds"), SyncDuration);
    }
    else
    {
        CurrentSyncStatus = ESyncStatus::SyncFailed;
        CurrentOperation.Status = ESyncStatus::SyncFailed;
        CurrentOperation.ErrorMessage = ErrorMessage;

        Stats.FailedSyncs++;
        Stats.LastFailedSync = FDateTime::Now();

        // Track error type
        FString ErrorType = TEXT("Unknown");
        if (ErrorMessage.Contains(TEXT("timeout")))
        {
            ErrorType = TEXT("Timeout");
        }
        else if (ErrorMessage.Contains(TEXT("network")))
        {
            ErrorType = TEXT("Network");
        }

        if (Stats.ErrorCountByType.Contains(ErrorType))
        {
            Stats.ErrorCountByType[ErrorType]++;
        }
        else
        {
            Stats.ErrorCountByType.Add(ErrorType, 1);
        }

        OnSyncFailed.Broadcast(CurrentOperation, ErrorMessage);
        UE_LOG(LogTemp, Warning, TEXT("Sync failed: %s"), *ErrorMessage);
    }
}

// ============================================================================
// Conflict Resolution
// ============================================================================

void UMGCrossProgressionSubsystem::ResolveConflict(const FGuid& ConflictId, EConflictResolution Resolution)
{
    for (int32 i = PendingConflicts.Num() - 1; i >= 0; i--)
    {
        if (PendingConflicts[i].ConflictId == ConflictId)
        {
            FSyncConflict Conflict = PendingConflicts[i];
            Conflict.bResolved = true;
            Conflict.ResolutionUsed = Resolution;

            // Apply resolution
            FProgressionSnapshot ResolvedData = ResolveConflictData(Conflict, Resolution);
            ApplySnapshot(ResolvedData);

            Stats.ConflictsResolved++;

            OnConflictResolved.Broadcast(Conflict, Resolution);
            PendingConflicts.RemoveAt(i);

            UE_LOG(LogTemp, Log, TEXT("Resolved conflict %s with %d"), *ConflictId.ToString(), static_cast<int32>(Resolution));
            return;
        }
    }
}

void UMGCrossProgressionSubsystem::ResolveAllConflicts(EConflictResolution Resolution)
{
    for (const FSyncConflict& Conflict : PendingConflicts)
    {
        ResolveConflict(Conflict.ConflictId, Resolution);
    }
}

FSyncConflict UMGCrossProgressionSubsystem::CompareSnapshots(const FProgressionSnapshot& Local, const FProgressionSnapshot& Cloud)
{
    return DetectConflict(Local, Cloud, EProgressionDataType::All);
}

FSyncConflict UMGCrossProgressionSubsystem::DetectConflict(const FProgressionSnapshot& Local, const FProgressionSnapshot& Cloud, EProgressionDataType DataType)
{
    FSyncConflict Conflict;
    Conflict.ConflictId = FGuid::NewGuid();
    Conflict.DataType = DataType;
    Conflict.LocalData = Local;
    Conflict.CloudData = Cloud;
    Conflict.DetectedAt = FDateTime::Now();
    Conflict.bResolved = false;

    // Check for differences
    TArray<FString> Differences;

    if (Local.PlayerLevel != Cloud.PlayerLevel)
    {
        Differences.Add(FString::Printf(TEXT("Level: Local=%d, Cloud=%d"), Local.PlayerLevel, Cloud.PlayerLevel));
    }
    if (Local.TotalXP != Cloud.TotalXP)
    {
        Differences.Add(FString::Printf(TEXT("XP: Local=%lld, Cloud=%lld"), Local.TotalXP, Cloud.TotalXP));
    }
    if (Local.Currency != Cloud.Currency)
    {
        Differences.Add(FString::Printf(TEXT("Currency: Local=%lld, Cloud=%lld"), Local.Currency, Cloud.Currency));
    }
    if (Local.VehicleCount != Cloud.VehicleCount)
    {
        Differences.Add(FString::Printf(TEXT("Vehicles: Local=%d, Cloud=%d"), Local.VehicleCount, Cloud.VehicleCount));
    }

    if (Differences.Num() > 0)
    {
        Conflict.ConflictDescription = FString::Join(Differences, TEXT("; "));
    }
    else
    {
        Conflict.bResolved = true; // No actual conflict
    }

    return Conflict;
}

FProgressionSnapshot UMGCrossProgressionSubsystem::ResolveConflictData(const FSyncConflict& Conflict, EConflictResolution Resolution)
{
    FProgressionSnapshot Resolved;

    switch (Resolution)
    {
        case EConflictResolution::UseLocal:
            Resolved = Conflict.LocalData;
            break;

        case EConflictResolution::UseCloud:
            Resolved = Conflict.CloudData;
            break;

        case EConflictResolution::UseMostRecent:
            Resolved = (Conflict.LocalData.CreatedAt > Conflict.CloudData.CreatedAt) ?
                Conflict.LocalData : Conflict.CloudData;
            break;

        case EConflictResolution::UseHighestProgress:
            Resolved = (Conflict.LocalData.TotalXP > Conflict.CloudData.TotalXP) ?
                Conflict.LocalData : Conflict.CloudData;
            break;

        case EConflictResolution::MergeData:
            // Take highest values from each
            Resolved.PlayerLevel = FMath::Max(Conflict.LocalData.PlayerLevel, Conflict.CloudData.PlayerLevel);
            Resolved.TotalXP = FMath::Max(Conflict.LocalData.TotalXP, Conflict.CloudData.TotalXP);
            Resolved.Currency = FMath::Max(Conflict.LocalData.Currency, Conflict.CloudData.Currency);
            Resolved.PremiumCurrency = FMath::Max(Conflict.LocalData.PremiumCurrency, Conflict.CloudData.PremiumCurrency);
            Resolved.VehicleCount = FMath::Max(Conflict.LocalData.VehicleCount, Conflict.CloudData.VehicleCount);
            Resolved.UnlockCount = FMath::Max(Conflict.LocalData.UnlockCount, Conflict.CloudData.UnlockCount);
            Resolved.AchievementCount = FMath::Max(Conflict.LocalData.AchievementCount, Conflict.CloudData.AchievementCount);
            Resolved.TotalPlaytimeHours = Conflict.LocalData.TotalPlaytimeHours + Conflict.CloudData.TotalPlaytimeHours;
            Resolved.TotalRaces = FMath::Max(Conflict.LocalData.TotalRaces, Conflict.CloudData.TotalRaces);
            Resolved.TotalWins = FMath::Max(Conflict.LocalData.TotalWins, Conflict.CloudData.TotalWins);
            Resolved.BattlePassTier = FMath::Max(Conflict.LocalData.BattlePassTier, Conflict.CloudData.BattlePassTier);
            break;

        default:
            Resolved = Conflict.LocalData;
            break;
    }

    Resolved.SnapshotId = FGuid::NewGuid();
    Resolved.CreatedAt = FDateTime::Now();
    Resolved.SourcePlatform = CurrentPlatform;
    Resolved.DataVersion++;

    return Resolved;
}

// ============================================================================
// Backups
// ============================================================================

void UMGCrossProgressionSubsystem::CreateBackup(const FString& BackupName, const FString& Reason)
{
    FCloudBackup Backup;
    Backup.BackupId = FGuid::NewGuid();
    Backup.BackupName = BackupName;
    Backup.CreatedAt = FDateTime::Now();
    Backup.SourcePlatform = CurrentPlatform;
    Backup.Snapshot = CreateLocalSnapshot();
    Backup.bIsAutomatic = false;
    Backup.BackupReason = Reason;
    Backup.BackupSizeBytes = 1024 * 50; // Estimate ~50KB

    Backup.IncludedDataTypes = Settings.SyncedDataTypes;

    CloudBackups.Add(Backup);
    LastBackupTime = FDateTime::Now();

    // Clean up old backups
    CleanupOldBackups();

    OnBackupCreated.Broadcast(Backup);

    UE_LOG(LogTemp, Log, TEXT("Created backup: %s"), *BackupName);
}

void UMGCrossProgressionSubsystem::RestoreBackup(const FGuid& BackupId)
{
    for (const FCloudBackup& Backup : CloudBackups)
    {
        if (Backup.BackupId == BackupId)
        {
            ApplySnapshot(Backup.Snapshot);
            OnBackupRestored.Broadcast(Backup);

            UE_LOG(LogTemp, Log, TEXT("Restored backup: %s"), *Backup.BackupName);
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Backup not found: %s"), *BackupId.ToString());
}

void UMGCrossProgressionSubsystem::DeleteBackup(const FGuid& BackupId)
{
    for (int32 i = CloudBackups.Num() - 1; i >= 0; i--)
    {
        if (CloudBackups[i].BackupId == BackupId)
        {
            CloudBackups.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Deleted backup: %s"), *BackupId.ToString());
            return;
        }
    }
}

FCloudBackup UMGCrossProgressionSubsystem::GetLatestBackup() const
{
    if (CloudBackups.Num() == 0)
    {
        return FCloudBackup();
    }

    FCloudBackup Latest = CloudBackups[0];
    for (const FCloudBackup& Backup : CloudBackups)
    {
        if (Backup.CreatedAt > Latest.CreatedAt)
        {
            Latest = Backup;
        }
    }
    return Latest;
}

void UMGCrossProgressionSubsystem::CleanupOldBackups()
{
    // Keep only the configured maximum number of backups
    while (CloudBackups.Num() > Settings.MaxBackupsToKeep)
    {
        // Find oldest backup
        int32 OldestIndex = 0;
        FDateTime OldestTime = CloudBackups[0].CreatedAt;

        for (int32 i = 1; i < CloudBackups.Num(); i++)
        {
            if (CloudBackups[i].CreatedAt < OldestTime)
            {
                OldestTime = CloudBackups[i].CreatedAt;
                OldestIndex = i;
            }
        }

        CloudBackups.RemoveAt(OldestIndex);
    }
}

void UMGCrossProgressionSubsystem::CreateAutomaticBackup()
{
    FString BackupName = FString::Printf(TEXT("Auto Backup %s"),
        *FDateTime::Now().ToString(TEXT("%Y-%m-%d %H:%M")));

    FCloudBackup Backup;
    Backup.BackupId = FGuid::NewGuid();
    Backup.BackupName = BackupName;
    Backup.CreatedAt = FDateTime::Now();
    Backup.SourcePlatform = CurrentPlatform;
    Backup.Snapshot = CreateLocalSnapshot();
    Backup.bIsAutomatic = true;
    Backup.BackupReason = TEXT("Automatic scheduled backup");
    Backup.BackupSizeBytes = 1024 * 50;
    Backup.IncludedDataTypes = Settings.SyncedDataTypes;

    CloudBackups.Add(Backup);
    LastBackupTime = FDateTime::Now();

    CleanupOldBackups();
    OnBackupCreated.Broadcast(Backup);
}

// ============================================================================
// Progress Snapshots
// ============================================================================

FProgressionSnapshot UMGCrossProgressionSubsystem::CreateLocalSnapshot()
{
    FProgressionSnapshot Snapshot;
    Snapshot.SnapshotId = FGuid::NewGuid();
    Snapshot.CreatedAt = FDateTime::Now();
    Snapshot.SourcePlatform = CurrentPlatform;

    // Populate with sample progression data
    Snapshot.PlayerLevel = 25;
    Snapshot.TotalXP = 150000;
    Snapshot.Currency = 50000;
    Snapshot.PremiumCurrency = 500;
    Snapshot.ReputationLevel = 15;
    Snapshot.VehicleCount = 12;
    Snapshot.UnlockCount = 45;
    Snapshot.AchievementCount = 30;
    Snapshot.TotalPlaytimeHours = 48.5f;
    Snapshot.TotalRaces = 200;
    Snapshot.TotalWins = 65;
    Snapshot.BattlePassTier = 35;

    // Calculate hash
    Snapshot.DataHash = CalculateDataHash(Snapshot);
    Snapshot.DataVersion = FDateTime::Now().ToUnixTimestamp();

    LastLocalSnapshot = Snapshot;
    return Snapshot;
}

FProgressionSnapshot UMGCrossProgressionSubsystem::FetchCloudSnapshot()
{
    // In real implementation, this would fetch from cloud
    // For now, return last known state
    return LastKnownCloudState;
}

void UMGCrossProgressionSubsystem::ApplySnapshot(const FProgressionSnapshot& Snapshot)
{
    // Apply the snapshot data to game state
    // This would integrate with other subsystems

    LastLocalSnapshot = Snapshot;
    LastKnownCloudState = Snapshot;

    UE_LOG(LogTemp, Log, TEXT("Applied snapshot from %s with level %d"),
        *GetPlatformName(Snapshot.SourcePlatform), Snapshot.PlayerLevel);
}

// ============================================================================
// Progress Transfer
// ============================================================================

FProgressTransferResult UMGCrossProgressionSubsystem::TransferProgressToPlatform(EPlatformType TargetPlatform)
{
    FProgressTransferResult Result;
    Result.bSuccess = false;

    if (!IsPlatformLinked(TargetPlatform))
    {
        Result.ErrorMessage = TEXT("Target platform is not linked");
        return Result;
    }

    float StartTime = FPlatformTime::Seconds();

    // Create snapshot of current progress
    FProgressionSnapshot Snapshot = CreateLocalSnapshot();

    // Simulate transfer
    Result.TransferredData = Snapshot;
    Result.ItemsTransferred = Snapshot.VehicleCount + Snapshot.UnlockCount;
    Result.ItemsSkipped = 0;
    Result.bSuccess = true;
    Result.TransferDurationSeconds = FPlatformTime::Seconds() - StartTime;

    OnProgressTransferred.Broadcast(CurrentPlatform, TargetPlatform);

    UE_LOG(LogTemp, Log, TEXT("Transferred progress to %s"), *GetPlatformName(TargetPlatform));

    return Result;
}

FProgressTransferResult UMGCrossProgressionSubsystem::ImportProgressFromPlatform(EPlatformType SourcePlatform)
{
    FProgressTransferResult Result;
    Result.bSuccess = false;

    if (!IsPlatformLinked(SourcePlatform))
    {
        Result.ErrorMessage = TEXT("Source platform is not linked");
        return Result;
    }

    float StartTime = FPlatformTime::Seconds();

    // Fetch cloud snapshot (would be from source platform)
    FProgressionSnapshot CloudSnapshot = FetchCloudSnapshot();

    // Check for conflicts
    FProgressionSnapshot LocalSnapshot = CreateLocalSnapshot();
    FSyncConflict Conflict = DetectConflict(LocalSnapshot, CloudSnapshot, EProgressionDataType::All);

    if (!Conflict.bResolved && Conflict.ConflictDescription.Len() > 0)
    {
        Result.Conflicts.Add(Conflict);
        PendingConflicts.Add(Conflict);
        OnConflictDetected.Broadcast(Conflict);
    }
    else
    {
        ApplySnapshot(CloudSnapshot);
        Result.TransferredData = CloudSnapshot;
        Result.bSuccess = true;
    }

    Result.TransferDurationSeconds = FPlatformTime::Seconds() - StartTime;

    return Result;
}

void UMGCrossProgressionSubsystem::MergeProgressFromAllPlatforms()
{
    FProgressionSnapshot MergedSnapshot;
    MergedSnapshot.SnapshotId = FGuid::NewGuid();
    MergedSnapshot.CreatedAt = FDateTime::Now();
    MergedSnapshot.SourcePlatform = CurrentPlatform;

    // In real implementation, fetch from all linked platforms
    // For now, use current local snapshot
    MergedSnapshot = CreateLocalSnapshot();

    ApplySnapshot(MergedSnapshot);
}

// ============================================================================
// Entitlements
// ============================================================================

void UMGCrossProgressionSubsystem::SyncEntitlements()
{
    // Sample entitlements
    AllEntitlements.Empty();

    FPlatformEntitlement BaseGame;
    BaseGame.EntitlementId = TEXT("base_game");
    BaseGame.EntitlementName = TEXT("Midnight Grind Base Game");
    BaseGame.Platform = CurrentPlatform;
    BaseGame.bIsOwned = true;
    BaseGame.bIsCrossPlatform = true;
    BaseGame.PurchasedAt = FDateTime::Now() - FTimespan::FromDays(30);
    AllEntitlements.Add(BaseGame);

    FPlatformEntitlement DeluxeEdition;
    DeluxeEdition.EntitlementId = TEXT("deluxe_edition");
    DeluxeEdition.EntitlementName = TEXT("Deluxe Edition Content");
    DeluxeEdition.Platform = CurrentPlatform;
    DeluxeEdition.bIsOwned = true;
    DeluxeEdition.bIsCrossPlatform = true;
    DeluxeEdition.PurchasedAt = FDateTime::Now() - FTimespan::FromDays(30);
    DeluxeEdition.UnlockedContent = { TEXT("ExclusiveVehicle_001"), TEXT("ExclusiveVinyl_001") };
    AllEntitlements.Add(DeluxeEdition);

    FPlatformEntitlement SeasonPass;
    SeasonPass.EntitlementId = TEXT("season_pass_1");
    SeasonPass.EntitlementName = TEXT("Season 1 Pass");
    SeasonPass.Platform = CurrentPlatform;
    SeasonPass.bIsOwned = true;
    SeasonPass.bIsCrossPlatform = true;
    SeasonPass.PurchasedAt = FDateTime::Now() - FTimespan::FromDays(15);
    SeasonPass.ExpiresAt = FDateTime::Now() + FTimespan::FromDays(75);
    AllEntitlements.Add(SeasonPass);
}

TArray<FPlatformEntitlement> UMGCrossProgressionSubsystem::GetCrossPlatformEntitlements() const
{
    TArray<FPlatformEntitlement> CrossPlatform;
    for (const FPlatformEntitlement& Entitlement : AllEntitlements)
    {
        if (Entitlement.bIsCrossPlatform && Entitlement.bIsOwned)
        {
            CrossPlatform.Add(Entitlement);
        }
    }
    return CrossPlatform;
}

bool UMGCrossProgressionSubsystem::HasEntitlement(const FString& EntitlementId) const
{
    for (const FPlatformEntitlement& Entitlement : AllEntitlements)
    {
        if (Entitlement.EntitlementId == EntitlementId && Entitlement.bIsOwned)
        {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Device Management
// ============================================================================

void UMGCrossProgressionSubsystem::RemoveDevice(const FString& DeviceId)
{
    for (int32 i = LinkedDevices.Num() - 1; i >= 0; i--)
    {
        if (LinkedDevices[i].DeviceId == DeviceId && !LinkedDevices[i].bCurrentDevice)
        {
            LinkedDevices.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Removed device: %s"), *DeviceId);
            return;
        }
    }
}

void UMGCrossProgressionSubsystem::TrustDevice(const FString& DeviceId)
{
    for (FDeviceInfo& Device : LinkedDevices)
    {
        if (Device.DeviceId == DeviceId)
        {
            Device.bTrusted = true;
            return;
        }
    }
}

void UMGCrossProgressionSubsystem::UntrustDevice(const FString& DeviceId)
{
    for (FDeviceInfo& Device : LinkedDevices)
    {
        if (Device.DeviceId == DeviceId)
        {
            Device.bTrusted = false;
            return;
        }
    }
}

FDeviceInfo UMGCrossProgressionSubsystem::GetCurrentDeviceInfo() const
{
    for (const FDeviceInfo& Device : LinkedDevices)
    {
        if (Device.bCurrentDevice)
        {
            return Device;
        }
    }
    return FDeviceInfo();
}

// ============================================================================
// Settings
// ============================================================================

void UMGCrossProgressionSubsystem::UpdateSettings(const FCrossSaveSettings& NewSettings)
{
    bool bAutoSyncChanged = Settings.bAutoSync != NewSettings.bAutoSync;
    Settings = NewSettings;

    if (bAutoSyncChanged)
    {
        if (Settings.bAutoSync)
        {
            StartAutoSyncTimer();
        }
        else
        {
            StopAutoSyncTimer();
        }
    }
}

void UMGCrossProgressionSubsystem::EnableCrossProgression(bool bEnable)
{
    Settings.bEnabled = bEnable;
    UnifiedPlayer.bCrossProgressionEnabled = bEnable;
}

void UMGCrossProgressionSubsystem::SetAutoSync(bool bEnable)
{
    Settings.bAutoSync = bEnable;

    if (bEnable)
    {
        StartAutoSyncTimer();
    }
    else
    {
        StopAutoSyncTimer();
    }
}

void UMGCrossProgressionSubsystem::SetConflictResolutionMode(EConflictResolution Mode)
{
    Settings.DefaultConflictResolution = Mode;
}

// ============================================================================
// Statistics
// ============================================================================

void UMGCrossProgressionSubsystem::ResetStats()
{
    Stats = FCrossProgressionStats();
    Stats.LinkedPlatformCount = UnifiedPlayer.LinkedAccounts.Num();
}

// ============================================================================
// Utilities
// ============================================================================

FString UMGCrossProgressionSubsystem::GetPlatformName(EPlatformType Platform) const
{
    switch (Platform)
    {
        case EPlatformType::PC_Steam: return TEXT("Steam");
        case EPlatformType::PC_Epic: return TEXT("Epic Games");
        case EPlatformType::PC_GOG: return TEXT("GOG");
        case EPlatformType::PlayStation5: return TEXT("PlayStation 5");
        case EPlatformType::PlayStation4: return TEXT("PlayStation 4");
        case EPlatformType::XboxSeriesX: return TEXT("Xbox Series X|S");
        case EPlatformType::XboxOne: return TEXT("Xbox One");
        case EPlatformType::NintendoSwitch: return TEXT("Nintendo Switch");
        case EPlatformType::Mobile_iOS: return TEXT("iOS");
        case EPlatformType::Mobile_Android: return TEXT("Android");
        case EPlatformType::Cloud: return TEXT("Cloud Gaming");
        default: return TEXT("Unknown");
    }
}

bool UMGCrossProgressionSubsystem::IsNetworkAvailable() const
{
    return bNetworkAvailable;
}

void UMGCrossProgressionSubsystem::RefreshNetworkStatus()
{
    // In real implementation, check actual network connectivity
    bNetworkAvailable = true;

    if (!bNetworkAvailable)
    {
        CurrentSyncStatus = ESyncStatus::Offline;
    }
}

FString UMGCrossProgressionSubsystem::CalculateDataHash(const FProgressionSnapshot& Snapshot) const
{
    // Create a hash of the snapshot data for integrity checking
    FString DataString = FString::Printf(TEXT("%d_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d"),
        Snapshot.PlayerLevel,
        Snapshot.TotalXP,
        Snapshot.Currency,
        Snapshot.PremiumCurrency,
        Snapshot.ReputationLevel,
        Snapshot.VehicleCount,
        Snapshot.UnlockCount,
        Snapshot.AchievementCount,
        Snapshot.TotalRaces,
        Snapshot.TotalWins
    );

    return FMD5::HashAnsiString(*DataString);
}

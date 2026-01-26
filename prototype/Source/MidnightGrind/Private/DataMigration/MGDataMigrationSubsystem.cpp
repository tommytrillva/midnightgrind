// Copyright Midnight Grind. All Rights Reserved.

#include "DataMigration/MGDataMigrationSubsystem.h"
#include "Misc/SecureHash.h"
#include "Misc/FileHelper.h"
#include "HAL/FileManager.h"

void UMGDataMigrationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Set current game version
	GameVersion.Major = 1;
	GameVersion.Minor = 0;
	GameVersion.Patch = 0;
	GameVersion.Build = 48;

	RegisterBuiltInMigrations();
	LoadMigrationHistory();
	DetectDataVersion();
}

void UMGDataMigrationSubsystem::Deinitialize()
{
	SaveMigrationHistory();
	Super::Deinitialize();
}

bool UMGDataMigrationSubsystem::NeedsMigration() const
{
	return CurrentDataVersion < GameVersion;
}

bool UMGDataMigrationSubsystem::IsDataVersionCompatible(const FMGDataVersion& Version) const
{
	// Major version must match for compatibility
	return Version.Major == GameVersion.Major;
}

FMGMigrationPlan UMGDataMigrationSubsystem::CreateMigrationPlan()
{
	FMGMigrationPlan Plan;
	Plan.SourceVersion = CurrentDataVersion;
	Plan.TargetVersion = GameVersion;

	// Find all applicable migrations
	for (const FMGMigrationScript& Script : RegisteredMigrations)
	{
		if (CurrentDataVersion < Script.ToVersion && Script.FromVersion <= CurrentDataVersion)
		{
			Plan.Scripts.Add(Script);
			Plan.EstimatedTotalDuration += Script.EstimatedDurationSeconds;
		}
	}

	// Sort by version
	Plan.Scripts.Sort([](const FMGMigrationScript& A, const FMGMigrationScript& B)
	{
		return A.FromVersion < B.FromVersion;
	});

	// Check if user confirmation needed
	for (const FMGMigrationScript& Script : Plan.Scripts)
	{
		if (Script.Priority == EMGMigrationPriority::Critical)
		{
			Plan.bRequiresUserConfirmation = true;
			Plan.UserMessage = FText::FromString(TEXT("Critical data migration required. A backup will be created before proceeding."));
			break;
		}
	}

	return Plan;
}

FMGMigrationPlan UMGDataMigrationSubsystem::CreateMigrationPlanForDomain(EMGDataDomain Domain)
{
	FMGMigrationPlan Plan = CreateMigrationPlan();

	// Filter by domain
	Plan.Scripts = Plan.Scripts.FilterByPredicate([Domain](const FMGMigrationScript& Script)
	{
		return Script.Domain == Domain || Script.Domain == EMGDataDomain::All;
	});

	// Recalculate duration
	Plan.EstimatedTotalDuration = 0.0f;
	for (const FMGMigrationScript& Script : Plan.Scripts)
	{
		Plan.EstimatedTotalDuration += Script.EstimatedDurationSeconds;
	}

	return Plan;
}

TArray<FMGMigrationScript> UMGDataMigrationSubsystem::GetPendingMigrations() const
{
	TArray<FMGMigrationScript> Pending;

	for (const FMGMigrationScript& Script : RegisteredMigrations)
	{
		if (CurrentDataVersion < Script.ToVersion && Script.FromVersion <= CurrentDataVersion)
		{
			Pending.Add(Script);
		}
	}

	return Pending;
}

bool UMGDataMigrationSubsystem::ExecuteMigrationPlan(const FMGMigrationPlan& Plan)
{
	if (bMigrationInProgress)
		return false;

	if (Plan.Scripts.Num() == 0)
		return true;

	bMigrationInProgress = true;
	MigrationProgress = 0.0f;

	// Create backup first
	if (Plan.Scripts[0].bRequiresBackup)
	{
		CreateBackup(TEXT("Pre-migration backup"));
	}

	int32 CompletedScripts = 0;
	bool bAllSuccessful = true;

	for (const FMGMigrationScript& Script : Plan.Scripts)
	{
		FMGMigrationResult Result;
		bool bSuccess = ExecuteMigrationScript(Script, Result);

		MigrationHistory.Add(Result);

		if (!bSuccess)
		{
			bAllSuccessful = false;
			OnMigrationFailed.Broadcast(Result);

			if (Script.bCanRollback)
			{
				RollbackLastMigration();
			}
			break;
		}

		CompletedScripts++;
		MigrationProgress = (float)CompletedScripts / (float)Plan.Scripts.Num();
		OnMigrationProgress.Broadcast(Script.ScriptID, MigrationProgress);
	}

	if (bAllSuccessful)
	{
		CurrentDataVersion = Plan.TargetVersion;
	}

	bMigrationInProgress = false;
	MigrationProgress = bAllSuccessful ? 1.0f : MigrationProgress;

	SaveMigrationHistory();
	return bAllSuccessful;
}

bool UMGDataMigrationSubsystem::ExecuteSingleMigration(FName ScriptID)
{
	const FMGMigrationScript* Script = RegisteredMigrations.FindByPredicate(
		[ScriptID](const FMGMigrationScript& S) { return S.ScriptID == ScriptID; });

	if (!Script)
		return false;

	FMGMigrationPlan Plan;
	Plan.SourceVersion = CurrentDataVersion;
	Plan.TargetVersion = Script->ToVersion;
	Plan.Scripts.Add(*Script);

	return ExecuteMigrationPlan(Plan);
}

void UMGDataMigrationSubsystem::CancelMigration()
{
	if (bMigrationInProgress)
	{
		bMigrationInProgress = false;
		// Attempt rollback of current migration
		RollbackLastMigration();
	}
}

bool UMGDataMigrationSubsystem::RollbackLastMigration()
{
	if (MigrationHistory.Num() == 0)
		return false;

	FMGMigrationResult& LastResult = MigrationHistory.Last();
	if (LastResult.Status != EMGMigrationStatus::Completed)
		return false;

	// Find the script
	const FMGMigrationScript* Script = RegisteredMigrations.FindByPredicate(
		[&LastResult](const FMGMigrationScript& S) { return S.ScriptID == LastResult.ScriptID; });

	if (!Script || !Script->bCanRollback)
		return false;

	// Restore from last backup
	if (Backups.Num() > 0)
	{
		bool bRestored = RestoreBackup(Backups.Last().BackupID);
		if (bRestored)
		{
			LastResult.Status = EMGMigrationStatus::RolledBack;
			CurrentDataVersion = Script->FromVersion;
			return true;
		}
	}

	return false;
}

bool UMGDataMigrationSubsystem::RollbackToVersion(const FMGDataVersion& Version)
{
	// Find backup with matching version
	for (int32 i = Backups.Num() - 1; i >= 0; i--)
	{
		if (Backups[i].DataVersion == Version || Backups[i].DataVersion < Version)
		{
			bool bRestored = RestoreBackup(Backups[i].BackupID);
			if (bRestored)
			{
				CurrentDataVersion = Backups[i].DataVersion;
				return true;
			}
		}
	}

	return false;
}

bool UMGDataMigrationSubsystem::CanRollback() const
{
	if (MigrationHistory.Num() == 0)
		return false;

	const FMGMigrationResult& LastResult = MigrationHistory.Last();
	if (LastResult.Status != EMGMigrationStatus::Completed)
		return false;

	const FMGMigrationScript* Script = RegisteredMigrations.FindByPredicate(
		[&LastResult](const FMGMigrationScript& S) { return S.ScriptID == LastResult.ScriptID; });

	return Script && Script->bCanRollback && Backups.Num() > 0;
}

FMGDataBackup UMGDataMigrationSubsystem::CreateBackup(const FString& Description)
{
	FMGDataBackup Backup;
	Backup.BackupID = GenerateBackupID();
	Backup.Timestamp = FDateTime::UtcNow();
	Backup.DataVersion = CurrentDataVersion;
	Backup.Description = Description;
	Backup.bIsAutoBackup = Description.IsEmpty();

	// Would actually copy save files here
	FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames");
	Backup.BackupPath = FPaths::ProjectSavedDir() / TEXT("Backups") / Backup.BackupID;

	// Simulate backup creation
	Backup.SizeBytes = 1024 * 1024; // 1MB placeholder
	Backup.Checksum = TEXT("placeholder_checksum");

	Backups.Add(Backup);
	OnBackupCreated.Broadcast(Backup);

	// Cleanup old backups
	CleanupOldBackups(MAX_BACKUPS);

	return Backup;
}

bool UMGDataMigrationSubsystem::RestoreBackup(const FString& BackupID)
{
	const FMGDataBackup* Backup = Backups.FindByPredicate(
		[&BackupID](const FMGDataBackup& B) { return B.BackupID == BackupID; });

	if (!Backup)
		return false;

	// Would actually restore files here
	CurrentDataVersion = Backup->DataVersion;

	return true;
}

bool UMGDataMigrationSubsystem::DeleteBackup(const FString& BackupID)
{
	int32 Index = Backups.IndexOfByPredicate(
		[&BackupID](const FMGDataBackup& B) { return B.BackupID == BackupID; });

	if (Index != INDEX_NONE)
	{
		// Would delete actual backup files
		Backups.RemoveAt(Index);
		return true;
	}

	return false;
}

void UMGDataMigrationSubsystem::CleanupOldBackups(int32 MaxBackupsToKeep)
{
	while (Backups.Num() > MaxBackupsToKeep)
	{
		// Remove oldest auto-backups first
		int32 OldestAutoBackupIndex = INDEX_NONE;

		for (int32 i = 0; i < Backups.Num(); i++)
		{
			if (Backups[i].bIsAutoBackup)
			{
				OldestAutoBackupIndex = i;
				break;
			}
		}

		if (OldestAutoBackupIndex != INDEX_NONE)
		{
			DeleteBackup(Backups[OldestAutoBackupIndex].BackupID);
		}
		else
		{
			// Remove oldest manual backup
			DeleteBackup(Backups[0].BackupID);
		}
	}
}

FMGDataIntegrityReport UMGDataMigrationSubsystem::CheckDataIntegrity()
{
	FMGDataIntegrityReport Report;
	Report.CheckTimestamp = FDateTime::UtcNow();
	Report.bIsValid = true;

	// Check each domain
	TArray<EMGDataDomain> Domains = {
		EMGDataDomain::PlayerProfile,
		EMGDataDomain::Garage,
		EMGDataDomain::Career,
		EMGDataDomain::Economy,
		EMGDataDomain::Social,
		EMGDataDomain::Settings,
		EMGDataDomain::Achievements,
		EMGDataDomain::Statistics,
		EMGDataDomain::Customization
	};

	for (EMGDataDomain Domain : Domains)
	{
		FMGDataIntegrityReport DomainReport = CheckDomainIntegrity(Domain);

		Report.Errors.Append(DomainReport.Errors);
		Report.Warnings.Append(DomainReport.Warnings);

		if (!DomainReport.bIsValid)
			Report.bIsValid = false;

		for (const auto& Pair : DomainReport.RecordCounts)
		{
			Report.RecordCounts.Add(Pair.Key, Pair.Value);
		}
	}

	Report.bCanAutoRepair = Report.Errors.Num() == 0 || Report.Errors.Num() <= 5;

	OnDataIntegrityChecked.Broadcast(Report);
	return Report;
}

FMGDataIntegrityReport UMGDataMigrationSubsystem::CheckDomainIntegrity(EMGDataDomain Domain)
{
	FMGDataIntegrityReport Report;
	Report.CheckTimestamp = FDateTime::UtcNow();
	Report.bIsValid = true;

	// Would perform actual integrity checks based on domain
	// For now, simulate checks

	FString DomainName;
	switch (Domain)
	{
	case EMGDataDomain::PlayerProfile: DomainName = TEXT("PlayerProfile"); break;
	case EMGDataDomain::Garage: DomainName = TEXT("Garage"); break;
	case EMGDataDomain::Career: DomainName = TEXT("Career"); break;
	case EMGDataDomain::Economy: DomainName = TEXT("Economy"); break;
	case EMGDataDomain::Social: DomainName = TEXT("Social"); break;
	case EMGDataDomain::Settings: DomainName = TEXT("Settings"); break;
	case EMGDataDomain::Achievements: DomainName = TEXT("Achievements"); break;
	case EMGDataDomain::Statistics: DomainName = TEXT("Statistics"); break;
	case EMGDataDomain::Customization: DomainName = TEXT("Customization"); break;
	default: DomainName = TEXT("Unknown"); break;
	}

	Report.RecordCounts.Add(DomainName, 0); // Placeholder

	return Report;
}

bool UMGDataMigrationSubsystem::RepairData(const FMGDataIntegrityReport& Report)
{
	if (!Report.bCanAutoRepair)
		return false;

	// Would attempt to repair data issues
	// For now, simulate repair
	return true;
}

bool UMGDataMigrationSubsystem::ValidateChecksum(const FString& DataPath)
{
	FString FileContents;
	if (!FFileHelper::LoadFileToString(FileContents, *DataPath))
		return false;

	FString ComputedChecksum = CalculateChecksum(DataPath);
	// Would compare against stored checksum

	return true;
}

FMGMigrationResult UMGDataMigrationSubsystem::GetLastMigrationResult() const
{
	if (MigrationHistory.Num() > 0)
		return MigrationHistory.Last();
	return FMGMigrationResult();
}

void UMGDataMigrationSubsystem::RegisterBuiltInMigrations()
{
	// Migration 1.0 to 1.1 - Add vehicle insurance data
	FMGMigrationScript Migration_1_0_1_1;
	Migration_1_0_1_1.ScriptID = FName(TEXT("Migration_1_0_to_1_1"));
	Migration_1_0_1_1.DisplayName = FText::FromString(TEXT("Add Insurance Data"));
	Migration_1_0_1_1.Description = FText::FromString(TEXT("Adds vehicle insurance fields to garage data"));
	Migration_1_0_1_1.FromVersion = {1, 0, 0, 0};
	Migration_1_0_1_1.ToVersion = {1, 1, 0, 0};
	Migration_1_0_1_1.Domain = EMGDataDomain::Garage;
	Migration_1_0_1_1.Priority = EMGMigrationPriority::High;
	Migration_1_0_1_1.EstimatedDurationSeconds = 2.0f;
	RegisteredMigrations.Add(Migration_1_0_1_1);

	// Migration 1.1 to 1.2 - Profile system update
	FMGMigrationScript Migration_1_1_1_2;
	Migration_1_1_1_2.ScriptID = FName(TEXT("Migration_1_1_to_1_2"));
	Migration_1_1_1_2.DisplayName = FText::FromString(TEXT("Profile System Update"));
	Migration_1_1_1_2.Description = FText::FromString(TEXT("Updates player profile structure with new fields"));
	Migration_1_1_1_2.FromVersion = {1, 1, 0, 0};
	Migration_1_1_1_2.ToVersion = {1, 2, 0, 0};
	Migration_1_1_1_2.Domain = EMGDataDomain::PlayerProfile;
	Migration_1_1_1_2.Priority = EMGMigrationPriority::Normal;
	Migration_1_1_1_2.EstimatedDurationSeconds = 1.5f;
	RegisteredMigrations.Add(Migration_1_1_1_2);

	// Migration 1.2 to 2.0 - Major schema update
	FMGMigrationScript Migration_1_2_2_0;
	Migration_1_2_2_0.ScriptID = FName(TEXT("Migration_1_2_to_2_0"));
	Migration_1_2_2_0.DisplayName = FText::FromString(TEXT("Major Schema Update"));
	Migration_1_2_2_0.Description = FText::FromString(TEXT("Complete schema restructure for Season 2"));
	Migration_1_2_2_0.FromVersion = {1, 2, 0, 0};
	Migration_1_2_2_0.ToVersion = {2, 0, 0, 0};
	Migration_1_2_2_0.Domain = EMGDataDomain::All;
	Migration_1_2_2_0.Priority = EMGMigrationPriority::Critical;
	Migration_1_2_2_0.EstimatedDurationSeconds = 10.0f;
	Migration_1_2_2_0.bRequiresBackup = true;
	RegisteredMigrations.Add(Migration_1_2_2_0);
}

void UMGDataMigrationSubsystem::LoadMigrationHistory()
{
	// Would load from save
}

void UMGDataMigrationSubsystem::SaveMigrationHistory()
{
	// Would save to file
}

void UMGDataMigrationSubsystem::DetectDataVersion()
{
	// Would detect version from save data
	// Default to current game version for new saves
	CurrentDataVersion = GameVersion;
}

bool UMGDataMigrationSubsystem::ExecuteMigrationScript(const FMGMigrationScript& Script, FMGMigrationResult& OutResult)
{
	OutResult.ScriptID = Script.ScriptID;
	OutResult.StartTime = FDateTime::UtcNow();
	OutResult.Status = EMGMigrationStatus::InProgress;

	CurrentMigrationScript = Script.ScriptID;
	OnMigrationStarted.Broadcast(Script);

	bool bSuccess = false;

	// Execute appropriate migration
	if (Script.ScriptID == FName(TEXT("Migration_1_0_to_1_1")))
	{
		bSuccess = Migration_1_0_to_1_1(OutResult);
	}
	else if (Script.ScriptID == FName(TEXT("Migration_1_1_to_1_2")))
	{
		bSuccess = Migration_1_1_to_1_2(OutResult);
	}
	else if (Script.ScriptID == FName(TEXT("Migration_1_2_to_2_0")))
	{
		bSuccess = Migration_1_2_to_2_0(OutResult);
	}

	OutResult.EndTime = FDateTime::UtcNow();
	OutResult.DurationSeconds = (OutResult.EndTime - OutResult.StartTime).GetTotalSeconds();
	OutResult.Status = bSuccess ? EMGMigrationStatus::Completed : EMGMigrationStatus::Failed;

	if (bSuccess)
	{
		OnMigrationCompleted.Broadcast(OutResult);
	}

	return bSuccess;
}

FString UMGDataMigrationSubsystem::GenerateBackupID()
{
	return FString::Printf(TEXT("backup_%s"), *FDateTime::UtcNow().ToString(TEXT("%Y%m%d_%H%M%S")));
}

FString UMGDataMigrationSubsystem::CalculateChecksum(const FString& FilePath)
{
	TArray<uint8> FileData;
	if (FFileHelper::LoadFileToArray(FileData, *FilePath))
	{
		return FMD5::HashBytes(FileData.GetData(), FileData.Num());
	}
	return TEXT("");
}

bool UMGDataMigrationSubsystem::Migration_1_0_to_1_1(FMGMigrationResult& Result)
{
	// Add insurance data to vehicles
	Result.RecordsProcessed = 0;

	// Would iterate through vehicle data and add insurance fields
	// For now, simulate success
	Result.RecordsProcessed = 10;
	Result.Message = FText::FromString(TEXT("Added insurance data to all vehicles"));

	return true;
}

bool UMGDataMigrationSubsystem::Migration_1_1_to_1_2(FMGMigrationResult& Result)
{
	// Update profile structure
	Result.RecordsProcessed = 0;

	// Would update profile data structure
	Result.RecordsProcessed = 1;
	Result.Message = FText::FromString(TEXT("Profile data structure updated"));

	return true;
}

bool UMGDataMigrationSubsystem::Migration_1_2_to_2_0(FMGMigrationResult& Result)
{
	// Major schema update
	Result.RecordsProcessed = 0;

	// Would perform comprehensive data restructure
	Result.RecordsProcessed = 100;
	Result.Message = FText::FromString(TEXT("Season 2 schema migration complete"));

	return true;
}

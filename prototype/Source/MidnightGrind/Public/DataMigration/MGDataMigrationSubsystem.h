// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGDataMigrationSubsystem.h
 * @brief Save Data Migration and Versioning Subsystem
 *
 * ============================================================================
 * Save Data Migration and Versioning Subsystem
 * ============================================================================
 *
 * @section overview OVERVIEW FOR NEW DEVELOPERS:
 * ----------------------------
 * This file defines the Data Migration Subsystem, which handles upgrading save
 * data when the game's data format changes between versions. When you update
 * the game and the save file structure changes, this system converts old saves
 * to the new format without losing player progress.
 *
 * @section why_migration WHY IS DATA MIGRATION NECESSARY?
 * --------------------------------
 * As a game evolves, the structure of save data changes:
 *   - Version 1.0: Player has "Money" as an integer
 *   - Version 1.1: "Money" renamed to "Credits" and split into "SoftCredits" and "HardCredits"
 *   - Version 2.0: Added "CryptoWallet" for premium currency
 *
 * Without migration, old save files would be incompatible with new game versions,
 * causing players to lose all their progress. The migration system:
 *   1. Detects the save file's version
 *   2. Finds the appropriate upgrade path
 *   3. Transforms the data step by step to the current version
 *
 * @section concepts KEY CONCEPTS:
 * -------------
 * 1. DATA VERSION: A four-part version number (Major.Minor.Patch.Build)
 *    - Major: Breaking changes requiring significant migration
 *    - Minor: New features, backward-compatible
 *    - Patch: Bug fixes, data corrections
 *    - Build: Internal build number
 *
 * 2. MIGRATION SCRIPT: Code that transforms data from one version to another.
 *    Each script handles exactly one version jump (e.g., 1.0 -> 1.1).
 *
 * 3. MIGRATION PLAN: The complete sequence of scripts needed to go from the
 *    save file's version to the current game version.
 *    Example: 1.0 -> 1.1 -> 1.2 -> 2.0 (three migration scripts)
 *
 * 4. DATA DOMAIN: Categories of game data that can be migrated separately:
 *    - PlayerProfile: Basic player info
 *    - Garage: Vehicle collection and customizations
 *    - Career: Story mode progress
 *    - Economy: Currency and purchases
 *    - Social: Friends, crews, chat
 *    - Settings: Game options
 *    - Achievements: Unlocked achievements
 *    - Statistics: Gameplay stats
 *    - Customization: Visual customizations
 *
 * 5. MIGRATION STATUS:
 *    - Pending: Not yet started
 *    - InProgress: Currently running
 *    - Completed: Successfully finished
 *    - Failed: Error occurred (may need rollback)
 *    - Skipped: Not needed or conditions not met
 *    - RolledBack: Reverted due to failure
 *
 * 6. BACKUP: A snapshot of data before migration, allowing rollback if
 *    something goes wrong. ALWAYS create backups before migrating!
 *
 * @section priority MIGRATION PRIORITY:
 * -------------------
 * - Critical: Must succeed or game won't work (core data)
 * - High: Important gameplay data (progress, currency)
 * - Normal: Regular game data
 * - Low: Nice to have, can continue without
 * - Optional: Can skip if issues occur
 *
 * @section integrity DATA INTEGRITY:
 * ---------------
 * After migration, the system validates the data:
 * - Checksums verify data wasn't corrupted
 * - Required fields exist and have valid values
 * - References between data are intact
 * If validation fails, rollback to backup.
 *
 * @section flow MIGRATION FLOW:
 * ---------------
 * 1. Game starts, loads save file
 * 2. Detect save file version vs current game version
 * 3. If different, create migration plan
 * 4. Create automatic backup
 * 5. Execute migrations in sequence
 * 6. Validate data integrity
 * 7. On success: Save migrated data, update version number
 * 8. On failure: Rollback to backup, report error
 *
 * @section scripts WRITING MIGRATION SCRIPTS:
 * --------------------------
 * Migration scripts are implemented as protected member functions:
 *   - Migration_1_0_to_1_1(): Handles 1.0 -> 1.1 changes
 *   - Migration_1_1_to_1_2(): Handles 1.1 -> 1.2 changes
 * Each returns true on success, fills FMGMigrationResult with details.
 *
 * @section usage USAGE EXAMPLE:
 * --------------
 * @code
 * // Get the subsystem
 * UMGDataMigrationSubsystem* Migration = GameInstance->GetSubsystem<UMGDataMigrationSubsystem>();
 *
 * // Check if migration is needed
 * if (Migration->NeedsMigration())
 * {
 *     // Create a plan showing what will happen
 *     FMGMigrationPlan Plan = Migration->CreateMigrationPlan();
 *
 *     // Show user what's happening (especially if bRequiresUserConfirmation)
 *     ShowMigrationUI(Plan);
 *
 *     // Execute the migration
 *     if (!Migration->ExecuteMigrationPlan(Plan))
 *     {
 *         // Migration failed - show error, offer rollback
 *         FMGMigrationResult LastResult = Migration->GetLastMigrationResult();
 *         ShowError(LastResult.ErrorMessage);
 *     }
 * }
 *
 * // Create manual backup before risky operation
 * FMGDataBackup Backup = Migration->CreateBackup("Before Prestige Reset");
 *
 * // Check data integrity
 * FMGDataIntegrityReport Report = Migration->CheckDataIntegrity();
 * if (!Report.bIsValid)
 * {
 *     // Data is corrupted
 *     if (Report.bCanAutoRepair)
 *     {
 *         Migration->RepairData(Report);
 *     }
 * }
 * @endcode
 *
 * @section best_practices BEST PRACTICES:
 * ---------------
 * 1. Always create backups before migrations
 * 2. Test migrations thoroughly with old save files
 * 3. Keep migration scripts simple and focused
 * 4. Log all changes for debugging
 * 5. Handle missing/null data gracefully
 * 6. Validate data after each migration step
 *
 * @section related RELATED FILES:
 * --------------
 * - MGCrossProgressionSubsystem.h: Syncs save data across platforms
 * - SaveGame classes: Define the actual save data structure
 *
 * @see FMGDataVersion For version number structure
 * @see FMGMigrationScript For migration script definition
 * @see FMGMigrationPlan For migration execution planning
 * ============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGDataMigrationSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGMigrationStatus : uint8
{
	Pending,
	InProgress,
	Completed,
	Failed,
	Skipped,
	RolledBack
};

UENUM(BlueprintType)
enum class EMGDataDomain : uint8
{
	PlayerProfile,
	Garage,
	Career,
	Economy,
	Social,
	Settings,
	Achievements,
	Statistics,
	Customization,
	All
};

UENUM(BlueprintType)
enum class EMGMigrationPriority : uint8
{
	Critical,
	High,
	Normal,
	Low,
	Optional
};

USTRUCT(BlueprintType)
struct FMGDataVersion
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Major = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Minor = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Patch = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Build = 0;

	FString ToString() const
	{
		return FString::Printf(TEXT("%d.%d.%d.%d"), Major, Minor, Patch, Build);
	}

	bool operator<(const FMGDataVersion& Other) const
	{
		if (Major != Other.Major) return Major < Other.Major;
		if (Minor != Other.Minor) return Minor < Other.Minor;
		if (Patch != Other.Patch) return Patch < Other.Patch;
		return Build < Other.Build;
	}

	bool operator==(const FMGDataVersion& Other) const
	{
		return Major == Other.Major && Minor == Other.Minor && Patch == Other.Patch && Build == Other.Build;
	}

	bool operator<=(const FMGDataVersion& Other) const
	{
		return *this < Other || *this == Other;
	}
};

USTRUCT(BlueprintType)
struct FMGMigrationScript
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ScriptID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDataVersion FromVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDataVersion ToVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGDataDomain Domain = EMGDataDomain::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMigrationPriority Priority = EMGMigrationPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresBackup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanRollback = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedDurationSeconds = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Dependencies;
};

USTRUCT(BlueprintType)
struct FMGMigrationResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ScriptID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGMigrationStatus Status = EMGMigrationStatus::Pending;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime StartTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime EndTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ErrorMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecordsProcessed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecordsFailed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Warnings;
};

USTRUCT(BlueprintType)
struct FMGDataBackup
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BackupID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDataVersion DataVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BackupPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 SizeBytes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Checksum;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAutoBackup = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Description;
};

USTRUCT(BlueprintType)
struct FMGMigrationPlan
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDataVersion SourceVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMGDataVersion TargetVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGMigrationScript> Scripts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EstimatedTotalDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequiresUserConfirmation = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UserMessage;
};

USTRUCT(BlueprintType)
struct FMGDataIntegrityReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CheckTimestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsValid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Errors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Warnings;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, int32> RecordCounts;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanAutoRepair = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMigrationStarted, const FMGMigrationScript&, Script);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMigrationCompleted, const FMGMigrationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMigrationProgress, FName, ScriptID, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnMigrationFailed, const FMGMigrationResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBackupCreated, const FMGDataBackup&, Backup);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnDataIntegrityChecked, const FMGDataIntegrityReport&, Report);

UCLASS()
class MIDNIGHTGRIND_API UMGDataMigrationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Version Management
	UFUNCTION(BlueprintPure, Category = "DataMigration|Version")
	FMGDataVersion GetCurrentDataVersion() const { return CurrentDataVersion; }

	UFUNCTION(BlueprintPure, Category = "DataMigration|Version")
	FMGDataVersion GetGameVersion() const { return GameVersion; }

	UFUNCTION(BlueprintPure, Category = "DataMigration|Version")
	bool NeedsMigration() const;

	UFUNCTION(BlueprintPure, Category = "DataMigration|Version")
	bool IsDataVersionCompatible(const FMGDataVersion& Version) const;

	// Migration Planning
	UFUNCTION(BlueprintCallable, Category = "DataMigration|Plan")
	FMGMigrationPlan CreateMigrationPlan();

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Plan")
	FMGMigrationPlan CreateMigrationPlanForDomain(EMGDataDomain Domain);

	UFUNCTION(BlueprintPure, Category = "DataMigration|Plan")
	TArray<FMGMigrationScript> GetAvailableMigrations() const { return RegisteredMigrations; }

	UFUNCTION(BlueprintPure, Category = "DataMigration|Plan")
	TArray<FMGMigrationScript> GetPendingMigrations() const;

	// Migration Execution
	UFUNCTION(BlueprintCallable, Category = "DataMigration|Execute")
	bool ExecuteMigrationPlan(const FMGMigrationPlan& Plan);

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Execute")
	bool ExecuteSingleMigration(FName ScriptID);

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Execute")
	void CancelMigration();

	UFUNCTION(BlueprintPure, Category = "DataMigration|Execute")
	bool IsMigrationInProgress() const { return bMigrationInProgress; }

	UFUNCTION(BlueprintPure, Category = "DataMigration|Execute")
	float GetMigrationProgress() const { return MigrationProgress; }

	// Rollback
	UFUNCTION(BlueprintCallable, Category = "DataMigration|Rollback")
	bool RollbackLastMigration();

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Rollback")
	bool RollbackToVersion(const FMGDataVersion& Version);

	UFUNCTION(BlueprintPure, Category = "DataMigration|Rollback")
	bool CanRollback() const;

	// Backup Management
	UFUNCTION(BlueprintCallable, Category = "DataMigration|Backup")
	FMGDataBackup CreateBackup(const FString& Description = TEXT(""));

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Backup")
	bool RestoreBackup(const FString& BackupID);

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Backup")
	bool DeleteBackup(const FString& BackupID);

	UFUNCTION(BlueprintPure, Category = "DataMigration|Backup")
	TArray<FMGDataBackup> GetAvailableBackups() const { return Backups; }

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Backup")
	void CleanupOldBackups(int32 MaxBackupsToKeep = 5);

	// Data Integrity
	UFUNCTION(BlueprintCallable, Category = "DataMigration|Integrity")
	FMGDataIntegrityReport CheckDataIntegrity();

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Integrity")
	FMGDataIntegrityReport CheckDomainIntegrity(EMGDataDomain Domain);

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Integrity")
	bool RepairData(const FMGDataIntegrityReport& Report);

	UFUNCTION(BlueprintCallable, Category = "DataMigration|Integrity")
	bool ValidateChecksum(const FString& DataPath);

	// Migration History
	UFUNCTION(BlueprintPure, Category = "DataMigration|History")
	TArray<FMGMigrationResult> GetMigrationHistory() const { return MigrationHistory; }

	UFUNCTION(BlueprintPure, Category = "DataMigration|History")
	FMGMigrationResult GetLastMigrationResult() const;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "DataMigration|Events")
	FMGOnMigrationStarted OnMigrationStarted;

	UPROPERTY(BlueprintAssignable, Category = "DataMigration|Events")
	FMGOnMigrationCompleted OnMigrationCompleted;

	UPROPERTY(BlueprintAssignable, Category = "DataMigration|Events")
	FMGOnMigrationProgress OnMigrationProgress;

	UPROPERTY(BlueprintAssignable, Category = "DataMigration|Events")
	FMGOnMigrationFailed OnMigrationFailed;

	UPROPERTY(BlueprintAssignable, Category = "DataMigration|Events")
	FMGOnBackupCreated OnBackupCreated;

	UPROPERTY(BlueprintAssignable, Category = "DataMigration|Events")
	FMGOnDataIntegrityChecked OnDataIntegrityChecked;

protected:
	void RegisterBuiltInMigrations();
	void LoadMigrationHistory();
	void SaveMigrationHistory();
	void DetectDataVersion();
	bool ExecuteMigrationScript(const FMGMigrationScript& Script, FMGMigrationResult& OutResult);
	FString GenerateBackupID();
	FString CalculateChecksum(const FString& FilePath);

	// Migration Script Implementations
	bool Migration_1_0_to_1_1(FMGMigrationResult& Result);
	bool Migration_1_1_to_1_2(FMGMigrationResult& Result);
	bool Migration_1_2_to_2_0(FMGMigrationResult& Result);

private:
	UPROPERTY()
	FMGDataVersion CurrentDataVersion;

	UPROPERTY()
	FMGDataVersion GameVersion;

	UPROPERTY()
	TArray<FMGMigrationScript> RegisteredMigrations;

	UPROPERTY()
	TArray<FMGMigrationResult> MigrationHistory;

	UPROPERTY()
	TArray<FMGDataBackup> Backups;

	bool bMigrationInProgress = false;
	float MigrationProgress = 0.0f;
	FName CurrentMigrationScript;

	static constexpr int32 MAX_BACKUPS = 10;
};

// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGLaunchChecklistSubsystem.h
 * @brief Pre-Launch Validation and Readiness Assessment Subsystem
 *
 * This subsystem provides comprehensive launch readiness validation for
 * Midnight Grind. It automates quality checks, tracks compliance requirements,
 * and generates reports to ensure the game is ready for release.
 *
 * Key Features:
 * - Automated validation checks for all critical systems
 * - Category-based check organization (Core, Gameplay, Multiplayer, etc.)
 * - Platform-specific compliance tracking (PlayStation, Xbox, Nintendo, Steam)
 * - Launch milestone management with progress tracking
 * - Exportable reports in text and JSON formats
 *
 * The subsystem supports both automated checks (run programmatically) and
 * manual verification items (marked complete by QA team members).
 *
 * Usage Example:
 * @code
 * UMGLaunchChecklistSubsystem* Checklist = GameInstance->GetSubsystem<UMGLaunchChecklistSubsystem>();
 * Checklist->OnChecklistCompleted.AddDynamic(this, &MyClass::HandleChecklistComplete);
 * Checklist->RunAllChecks();
 *
 * if (Checklist->IsLaunchReady())
 * {
 *     UE_LOG(LogTemp, Log, TEXT("All checks passed! Ready for launch."));
 * }
 * @endcode
 *
 * @see FMGChecklistReport for report structure
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLaunchChecklistSubsystem.generated.h"

// ============================================================================
// Enumerations
// ============================================================================

/**
 * @brief Category classification for launch checks
 *
 * Groups checks by functional area for organized reporting
 * and targeted validation runs.
 */
UENUM(BlueprintType)
enum class EMGCheckCategory : uint8
{
	/** Core engine and subsystem functionality */
	Core,
	/** Gameplay mechanics and features */
	Gameplay,
	/** Online and multiplayer systems */
	Multiplayer,
	/** In-game economy and monetization */
	Economy,
	/** Player progression and unlocks */
	Progression,
	/** Social features (friends, clubs, chat) */
	Social,
	/** Audio systems and content */
	Audio,
	/** Visual quality and rendering */
	Visual,
	/** Performance targets and optimization */
	Performance,
	/** Security and anti-cheat */
	Security,
	/** Legal and regulatory compliance */
	Compliance,
	/** Content completeness and quality */
	Content,
	/** Language and region support */
	Localization,
	/** Accessibility features */
	Accessibility,
	/** Platform-specific requirements */
	Platform
};

/**
 * @brief Severity level of a check failure
 *
 * Determines the impact on launch readiness if the check fails.
 */
UENUM(BlueprintType)
enum class EMGCheckSeverity : uint8
{
	/** Failure blocks launch - must be fixed */
	Critical,
	/** Should be fixed before launch if possible */
	Major,
	/** Nice to fix but not required for launch */
	Minor,
	/** Information only - does not affect launch decision */
	Informational
};

/**
 * @brief Current status of a validation check
 *
 * Tracks the execution state and result of each check.
 */
UENUM(BlueprintType)
enum class EMGCheckStatus : uint8
{
	/** Check has not been executed */
	NotRun,
	/** Check is currently executing */
	Running,
	/** Check completed successfully */
	Passed,
	/** Check found issues that need attention */
	Failed,
	/** Check passed with minor concerns */
	Warning,
	/** Check was intentionally skipped */
	Skipped,
	/** Check encountered an error during execution */
	Error
};

/**
 * @brief Target platform for validation
 *
 * Used to filter checks and compliance items by platform.
 */
UENUM(BlueprintType)
enum class EMGPlatformTarget : uint8
{
	/** Microsoft Windows PC */
	Windows,
	/** Sony PlayStation 5 */
	PlayStation5,
	/** Microsoft Xbox Series X|S */
	XboxSeriesX,
	/** Nintendo Switch */
	NintendoSwitch,
	/** Steam store/platform requirements */
	Steam,
	/** Epic Games Store requirements */
	EpicGames,
	/** Applies to all platforms */
	All
};

// ============================================================================
// Data Structures - Checks
// ============================================================================

/**
 * @brief Definition of a single launch validation check
 *
 * Contains the check configuration, status, and results.
 */
USTRUCT(BlueprintType)
struct FMGLaunchCheck
{
	GENERATED_BODY()

	/** Unique identifier for this check */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CheckID;

	/** Display name shown in reports */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	/** Detailed description of what this check validates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Functional category for grouping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckCategory Category = EMGCheckCategory::Core;

	/** Impact level if this check fails */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckSeverity Severity = EMGCheckSeverity::Major;

	/** Current execution status */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckStatus Status = EMGCheckStatus::NotRun;

	/** Which platforms this check applies to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPlatformTarget> TargetPlatforms;

	/** Whether this check runs automatically (false = manual verification) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomated = true;

	/** Whether this check must pass for launch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequired = true;

	/** Summary result message from the check */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultMessage;

	/** Detailed findings and recommendations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Details;

	/** When this check was last executed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRunTime;

	/** How long the check took to execute in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationSeconds = 0.0f;

	/** Other check IDs that must pass before this one runs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Dependencies;
};

// ============================================================================
// Data Structures - Reports
// ============================================================================

/**
 * @brief Complete launch checklist report
 *
 * Aggregates all check results with summary statistics
 * for launch readiness assessment.
 */
USTRUCT(BlueprintType)
struct FMGChecklistReport
{
	GENERATED_BODY()

	/** When this report was generated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime GeneratedAt;

	/** Build version being validated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuildVersion;

	/** Platform this report is for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTarget Platform = EMGPlatformTarget::All;

	/** Total number of checks in the report */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalChecks = 0;

	/** Number of checks that passed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PassedChecks = 0;

	/** Number of checks that failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FailedChecks = 0;

	/** Number of checks with warnings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WarningChecks = 0;

	/** Number of checks that were skipped */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkippedChecks = 0;

	/** Count of critical severity failures */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CriticalBlockers = 0;

	/** Overall go/no-go decision for launch */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLaunchReady = false;

	/** Overall readiness score (0.0-1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverallScore = 0.0f;

	/** Complete list of check results */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLaunchCheck> Results;

	/** List of launch-blocking issues */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Blockers;

	/** Actionable recommendations for improvement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Recommendations;
};

// ============================================================================
// Data Structures - Compliance
// ============================================================================

/**
 * @brief Platform compliance requirement
 *
 * Tracks certification requirements for platform submission
 * (TRC, XR, Lotcheck, etc.).
 */
USTRUCT(BlueprintType)
struct FMGComplianceItem
{
	GENERATED_BODY()

	/** Unique identifier for this requirement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	/** The compliance requirement text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Requirement;

	/** Detailed description and implementation notes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Platform this requirement applies to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTarget Platform = EMGPlatformTarget::All;

	/** Whether this is a mandatory requirement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMandatory = true;

	/** Whether compliance has been verified */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVerified = false;

	/** Notes from the verification process */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VerificationNotes;

	/** When this item was verified */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime VerifiedAt;

	/** Name of the person who verified */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VerifiedBy;
};

// ============================================================================
// Data Structures - Milestones
// ============================================================================

/**
 * @brief Launch milestone definition
 *
 * Represents a significant checkpoint in the launch preparation
 * process (e.g., Alpha, Beta, Release Candidate).
 */
USTRUCT(BlueprintType)
struct FMGLaunchMilestone
{
	GENERATED_BODY()

	/** Unique identifier for this milestone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MilestoneID;

	/** Display name of the milestone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	/** Description of milestone requirements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** Target date for milestone completion */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime TargetDate;

	/** Whether the milestone has been achieved */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	/** Actual completion timestamp */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	/** Check IDs that must pass for this milestone */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredChecks;

	/** Current progress towards completion (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CompletionPercent = 0.0f;
};

/**
 * @brief Current build information
 *
 * Metadata about the build being validated for inclusion in reports.
 */
USTRUCT(BlueprintType)
struct FMGBuildInfo
{
	GENERATED_BODY()

	/** Semantic version string (e.g., "1.0.0") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Version;

	/** CI/CD build number */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuildNumber;

	/** Source control commit hash */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CommitHash;

	/** Source control branch name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Branch;

	/** When this build was created */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BuildTime;

	/** Build configuration (Debug, Development, Shipping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Configuration;

	/** Target platform for this build */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTarget Platform = EMGPlatformTarget::Windows;
};

// ============================================================================
// Delegate Declarations
// ============================================================================

/** Broadcast when a check begins execution */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCheckStarted, FName, CheckID);

/** Broadcast when a check completes execution */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCheckCompleted, FName, CheckID, EMGCheckStatus, Status);

/** Broadcast when all checks in a run complete */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChecklistCompleted, const FMGChecklistReport&, Report);

/** Broadcast when a milestone is achieved */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMilestoneCompleted, FName, MilestoneID, const FMGLaunchMilestone&, Milestone);

/** Broadcast when a new launch blocker is identified */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBlockerIdentified, const FString&, BlockerDescription);

// ============================================================================
// Subsystem Class
// ============================================================================

/**
 * @brief Launch Checklist Subsystem
 *
 * Manages pre-launch validation, compliance tracking, and readiness
 * assessment for Midnight Grind. Provides automated checks and manual
 * verification workflows.
 *
 * Checks are organized by category and severity, with support for
 * platform-specific filtering. Reports can be exported in multiple
 * formats for stakeholder review.
 *
 * Persists across level transitions as a GameInstance subsystem.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGLaunchChecklistSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// @brief Called when the subsystem is created. Registers default checks and milestones.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/// @brief Called when the subsystem is destroyed. Saves pending state.
	virtual void Deinitialize() override;

	// ========================================================================
	// Check Execution
	// ========================================================================

	/**
	 * @brief Run all registered validation checks
	 * Executes checks asynchronously in dependency order
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunAllChecks();

	/**
	 * @brief Run all checks in a specific category
	 * @param Category The category of checks to run
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunChecksByCategory(EMGCheckCategory Category);

	/**
	 * @brief Run a single check by ID
	 * @param CheckID The unique identifier of the check to run
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunSingleCheck(FName CheckID);

	/**
	 * @brief Run only critical severity checks
	 * Fast validation for launch-blocking issues
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunCriticalChecks();

	/**
	 * @brief Run checks specific to a target platform
	 * @param Platform The platform to validate for
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunPlatformChecks(EMGPlatformTarget Platform);

	/**
	 * @brief Check if validation is currently in progress
	 * @return True if checks are running
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Execute")
	bool IsCheckRunning() const { return bIsRunningChecks; }

	/**
	 * @brief Get current check execution progress
	 * @return Progress from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Execute")
	float GetCheckProgress() const { return CheckProgress; }

	// ========================================================================
	// Check Management
	// ========================================================================

	/**
	 * @brief Register a new validation check
	 * @param Check The check definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Checks")
	void RegisterCheck(const FMGLaunchCheck& Check);

	/**
	 * @brief Get a check by its ID
	 * @param CheckID The check identifier
	 * @return The check definition and current status
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	FMGLaunchCheck GetCheck(FName CheckID) const;

	/**
	 * @brief Get all checks in a category
	 * @param Category The category to filter by
	 * @return Array of matching checks
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	TArray<FMGLaunchCheck> GetChecksByCategory(EMGCheckCategory Category) const;

	/**
	 * @brief Get all checks that have failed
	 * @return Array of failed checks
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	TArray<FMGLaunchCheck> GetFailedChecks() const;

	/**
	 * @brief Get all checks that are blocking launch
	 * @return Array of critical failures
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	TArray<FMGLaunchCheck> GetBlockers() const;

	/**
	 * @brief Mark a manual check as verified
	 * @param CheckID The check to mark
	 * @param bPassed Whether verification passed
	 * @param Notes Verification notes
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Checks")
	void MarkCheckManuallyVerified(FName CheckID, bool bPassed, const FString& Notes);

	// ========================================================================
	// Reports
	// ========================================================================

	/**
	 * @brief Generate a complete checklist report
	 * @return The generated report with all results
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FMGChecklistReport GenerateReport();

	/**
	 * @brief Generate a report for a specific platform
	 * @param Platform The platform to report on
	 * @return Platform-filtered report
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FMGChecklistReport GeneratePlatformReport(EMGPlatformTarget Platform);

	/**
	 * @brief Get the most recently generated report
	 * @return The last generated report
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Reports")
	FMGChecklistReport GetLastReport() const { return LastReport; }

	/**
	 * @brief Export the current report to readable text format
	 * @return Formatted text report
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FString ExportReportToText();

	/**
	 * @brief Export the current report to JSON format
	 * @return JSON string representation
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FString ExportReportToJSON();

	// ========================================================================
	// Launch Readiness
	// ========================================================================

	/**
	 * @brief Check if all requirements for launch are met
	 * @return True if no critical blockers exist
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	bool IsLaunchReady() const;

	/**
	 * @brief Get the overall launch readiness score
	 * @return Score from 0.0 (not ready) to 1.0 (fully ready)
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	float GetLaunchReadinessScore() const;

	/**
	 * @brief Get the count of launch-blocking issues
	 * @return Number of critical failures
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	int32 GetBlockerCount() const;

	/**
	 * @brief Get descriptions of all launch blockers
	 * @return Array of blocker descriptions
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	TArray<FString> GetBlockerDescriptions() const;

	// ========================================================================
	// Compliance
	// ========================================================================

	/**
	 * @brief Register a platform compliance requirement
	 * @param Item The compliance item to register
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Compliance")
	void RegisterComplianceItem(const FMGComplianceItem& Item);

	/**
	 * @brief Get compliance items, optionally filtered by platform
	 * @param Platform Platform filter (All for no filter)
	 * @return Array of compliance items
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Compliance")
	TArray<FMGComplianceItem> GetComplianceItems(EMGPlatformTarget Platform = EMGPlatformTarget::All) const;

	/**
	 * @brief Mark a compliance item as verified
	 * @param ItemID The item to verify
	 * @param Notes Verification notes
	 * @param VerifiedBy Name of the verifier
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Compliance")
	void VerifyComplianceItem(FName ItemID, const FString& Notes, const FString& VerifiedBy);

	/**
	 * @brief Get compliance verification progress
	 * @param Platform Platform filter (All for overall progress)
	 * @return Progress from 0.0 to 1.0
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Compliance")
	float GetComplianceProgress(EMGPlatformTarget Platform = EMGPlatformTarget::All) const;

	// ========================================================================
	// Milestones
	// ========================================================================

	/**
	 * @brief Register a launch milestone
	 * @param Milestone The milestone definition
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Milestones")
	void RegisterMilestone(const FMGLaunchMilestone& Milestone);

	/**
	 * @brief Get all registered milestones
	 * @return Array of milestones
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Milestones")
	TArray<FMGLaunchMilestone> GetMilestones() const { return Milestones; }

	/**
	 * @brief Get the next incomplete milestone
	 * @return The upcoming milestone
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Milestones")
	FMGLaunchMilestone GetNextMilestone() const;

	/**
	 * @brief Recalculate progress for all milestones
	 */
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Milestones")
	void UpdateMilestoneProgress();

	// ========================================================================
	// Build Info
	// ========================================================================

	/**
	 * @brief Get current build information
	 * @return Build metadata
	 */
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Build")
	FMGBuildInfo GetBuildInfo() const { return CurrentBuildInfo; }

	// ========================================================================
	// Events
	// ========================================================================

	/** Broadcast when a check begins running */
	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnCheckStarted OnCheckStarted;

	/** Broadcast when a check finishes running */
	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnCheckCompleted OnCheckCompleted;

	/** Broadcast when a check run completes */
	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnChecklistCompleted OnChecklistCompleted;

	/** Broadcast when a milestone is achieved */
	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnMilestoneCompleted OnMilestoneCompleted;

	/** Broadcast when a new blocker is found */
	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnBlockerIdentified OnBlockerIdentified;

protected:
	/// @brief Register built-in default checks
	void RegisterDefaultChecks();

	/// @brief Register platform compliance requirements
	void RegisterComplianceRequirements();

	/// @brief Register default launch milestones
	void RegisterDefaultMilestones();

	/// @brief Detect and populate build information
	void DetectBuildInfo();

	// ========================================================================
	// Check Implementations
	// ========================================================================

	/// @brief Validate all core subsystems are initialized
	bool Check_CoreSubsystems(FMGLaunchCheck& Check);

	/// @brief Validate save system integrity
	bool Check_SaveSystem(FMGLaunchCheck& Check);

	/// @brief Validate network connectivity
	bool Check_NetworkConnectivity(FMGLaunchCheck& Check);

	/// @brief Validate economy balance
	bool Check_EconomyBalance(FMGLaunchCheck& Check);

	/// @brief Validate progression flow
	bool Check_ProgressionFlow(FMGLaunchCheck& Check);

	/// @brief Validate localization coverage
	bool Check_LocalizationCoverage(FMGLaunchCheck& Check);

	/// @brief Validate accessibility features
	bool Check_AccessibilityFeatures(FMGLaunchCheck& Check);

	/// @brief Validate performance targets
	bool Check_PerformanceTargets(FMGLaunchCheck& Check);

	/// @brief Validate memory budget
	bool Check_MemoryBudget(FMGLaunchCheck& Check);

	/// @brief Validate content integrity
	bool Check_ContentIntegrity(FMGLaunchCheck& Check);

	/// @brief Validate audio completeness
	bool Check_AudioComplete(FMGLaunchCheck& Check);

	/// @brief Validate multiplayer stability
	bool Check_MultiplayerStability(FMGLaunchCheck& Check);

	/// @brief Validate anti-cheat integration
	bool Check_AntiCheatIntegration(FMGLaunchCheck& Check);

	/// @brief Validate age rating compliance
	bool Check_AgeRatingCompliance(FMGLaunchCheck& Check);

	/// @brief Validate privacy compliance (GDPR, CCPA, etc.)
	bool Check_PrivacyCompliance(FMGLaunchCheck& Check);

	/// @brief Execute a single check and update its status
	/// @param Check The check to execute
	/// @return True if the check passed
	bool ExecuteCheck(FMGLaunchCheck& Check);

private:
	/** All registered validation checks */
	UPROPERTY()
	TArray<FMGLaunchCheck> Checks;

	/** All registered compliance items */
	UPROPERTY()
	TArray<FMGComplianceItem> ComplianceItems;

	/** All registered milestones */
	UPROPERTY()
	TArray<FMGLaunchMilestone> Milestones;

	/** Most recently generated report */
	UPROPERTY()
	FMGChecklistReport LastReport;

	/** Current build information */
	UPROPERTY()
	FMGBuildInfo CurrentBuildInfo;

	/** Whether checks are currently running */
	bool bIsRunningChecks = false;

	/** Current check run progress (0.0-1.0) */
	float CheckProgress = 0.0f;

	/** Index of the currently executing check */
	int32 CurrentCheckIndex = 0;
};

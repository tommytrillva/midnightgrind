// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGLaunchChecklistSubsystem.generated.h"

/**
 * Pre-Launch Validation and Readiness System
 * - Automated checks for launch readiness
 * - Validates all critical systems are functional
 * - Generates compliance and certification reports
 * - Tracks launch blockers and go/no-go criteria
 */

UENUM(BlueprintType)
enum class EMGCheckCategory : uint8
{
	Core,
	Gameplay,
	Multiplayer,
	Economy,
	Progression,
	Social,
	Audio,
	Visual,
	Performance,
	Security,
	Compliance,
	Content,
	Localization,
	Accessibility,
	Platform
};

UENUM(BlueprintType)
enum class EMGCheckSeverity : uint8
{
	Critical,      // Launch blocker
	Major,         // Should fix before launch
	Minor,         // Nice to fix
	Informational  // FYI only
};

UENUM(BlueprintType)
enum class EMGCheckStatus : uint8
{
	NotRun,
	Running,
	Passed,
	Failed,
	Warning,
	Skipped,
	Error
};

UENUM(BlueprintType)
enum class EMGPlatformTarget : uint8
{
	Windows,
	PlayStation5,
	XboxSeriesX,
	NintendoSwitch,
	Steam,
	EpicGames,
	All
};

USTRUCT(BlueprintType)
struct FMGLaunchCheck
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName CheckID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckCategory Category = EMGCheckCategory::Core;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckSeverity Severity = EMGCheckSeverity::Major;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCheckStatus Status = EMGCheckStatus::NotRun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<EMGPlatformTarget> TargetPlatforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAutomated = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bRequired = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ResultMessage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Details;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime LastRunTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Dependencies;
};

USTRUCT(BlueprintType)
struct FMGChecklistReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime GeneratedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuildVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTarget Platform = EMGPlatformTarget::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TotalChecks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PassedChecks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FailedChecks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 WarningChecks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SkippedChecks = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CriticalBlockers = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bLaunchReady = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverallScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMGLaunchCheck> Results;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Blockers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Recommendations;
};

USTRUCT(BlueprintType)
struct FMGComplianceItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Requirement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTarget Platform = EMGPlatformTarget::All;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMandatory = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bVerified = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VerificationNotes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime VerifiedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString VerifiedBy;
};

USTRUCT(BlueprintType)
struct FMGLaunchMilestone
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName MilestoneID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime TargetDate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime CompletedAt;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> RequiredChecks;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CompletionPercent = 0.0f;
};

USTRUCT(BlueprintType)
struct FMGBuildInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Version;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuildNumber;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CommitHash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Branch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime BuildTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Configuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGPlatformTarget Platform = EMGPlatformTarget::Windows;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCheckStarted, FName, CheckID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnCheckCompleted, FName, CheckID, EMGCheckStatus, Status);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnChecklistCompleted, const FMGChecklistReport&, Report);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMGOnMilestoneCompleted, FName, MilestoneID, const FMGLaunchMilestone&, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnBlockerIdentified, const FString&, BlockerDescription);

UCLASS()
class MIDNIGHTGRIND_API UMGLaunchChecklistSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Check Execution
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunAllChecks();

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunChecksByCategory(EMGCheckCategory Category);

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunSingleCheck(FName CheckID);

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunCriticalChecks();

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Execute")
	void RunPlatformChecks(EMGPlatformTarget Platform);

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Execute")
	bool IsCheckRunning() const { return bIsRunningChecks; }

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Execute")
	float GetCheckProgress() const { return CheckProgress; }

	// Check Management
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Checks")
	void RegisterCheck(const FMGLaunchCheck& Check);

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	FMGLaunchCheck GetCheck(FName CheckID) const;

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	TArray<FMGLaunchCheck> GetChecksByCategory(EMGCheckCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	TArray<FMGLaunchCheck> GetFailedChecks() const;

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Checks")
	TArray<FMGLaunchCheck> GetBlockers() const;

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Checks")
	void MarkCheckManuallyVerified(FName CheckID, bool bPassed, const FString& Notes);

	// Reports
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FMGChecklistReport GenerateReport();

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FMGChecklistReport GeneratePlatformReport(EMGPlatformTarget Platform);

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Reports")
	FMGChecklistReport GetLastReport() const { return LastReport; }

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FString ExportReportToText();

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Reports")
	FString ExportReportToJSON();

	// Launch Readiness
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	bool IsLaunchReady() const;

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	float GetLaunchReadinessScore() const;

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	int32 GetBlockerCount() const;

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Readiness")
	TArray<FString> GetBlockerDescriptions() const;

	// Compliance
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Compliance")
	void RegisterComplianceItem(const FMGComplianceItem& Item);

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Compliance")
	TArray<FMGComplianceItem> GetComplianceItems(EMGPlatformTarget Platform = EMGPlatformTarget::All) const;

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Compliance")
	void VerifyComplianceItem(FName ItemID, const FString& Notes, const FString& VerifiedBy);

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Compliance")
	float GetComplianceProgress(EMGPlatformTarget Platform = EMGPlatformTarget::All) const;

	// Milestones
	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Milestones")
	void RegisterMilestone(const FMGLaunchMilestone& Milestone);

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Milestones")
	TArray<FMGLaunchMilestone> GetMilestones() const { return Milestones; }

	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Milestones")
	FMGLaunchMilestone GetNextMilestone() const;

	UFUNCTION(BlueprintCallable, Category = "LaunchChecklist|Milestones")
	void UpdateMilestoneProgress();

	// Build Info
	UFUNCTION(BlueprintPure, Category = "LaunchChecklist|Build")
	FMGBuildInfo GetBuildInfo() const { return CurrentBuildInfo; }

	// Events
	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnCheckStarted OnCheckStarted;

	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnCheckCompleted OnCheckCompleted;

	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnChecklistCompleted OnChecklistCompleted;

	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnMilestoneCompleted OnMilestoneCompleted;

	UPROPERTY(BlueprintAssignable, Category = "LaunchChecklist|Events")
	FMGOnBlockerIdentified OnBlockerIdentified;

protected:
	void RegisterDefaultChecks();
	void RegisterComplianceRequirements();
	void RegisterDefaultMilestones();
	void DetectBuildInfo();

	// Check Implementations
	bool Check_CoreSubsystems(FMGLaunchCheck& Check);
	bool Check_SaveSystem(FMGLaunchCheck& Check);
	bool Check_NetworkConnectivity(FMGLaunchCheck& Check);
	bool Check_EconomyBalance(FMGLaunchCheck& Check);
	bool Check_ProgressionFlow(FMGLaunchCheck& Check);
	bool Check_LocalizationCoverage(FMGLaunchCheck& Check);
	bool Check_AccessibilityFeatures(FMGLaunchCheck& Check);
	bool Check_PerformanceTargets(FMGLaunchCheck& Check);
	bool Check_MemoryBudget(FMGLaunchCheck& Check);
	bool Check_ContentIntegrity(FMGLaunchCheck& Check);
	bool Check_AudioComplete(FMGLaunchCheck& Check);
	bool Check_MultiplayerStability(FMGLaunchCheck& Check);
	bool Check_AntiCheatIntegration(FMGLaunchCheck& Check);
	bool Check_AgeRatingCompliance(FMGLaunchCheck& Check);
	bool Check_PrivacyCompliance(FMGLaunchCheck& Check);

	bool ExecuteCheck(FMGLaunchCheck& Check);

private:
	UPROPERTY()
	TArray<FMGLaunchCheck> Checks;

	UPROPERTY()
	TArray<FMGComplianceItem> ComplianceItems;

	UPROPERTY()
	TArray<FMGLaunchMilestone> Milestones;

	UPROPERTY()
	FMGChecklistReport LastReport;

	UPROPERTY()
	FMGBuildInfo CurrentBuildInfo;

	bool bIsRunningChecks = false;
	float CheckProgress = 0.0f;
	int32 CurrentCheckIndex = 0;
};

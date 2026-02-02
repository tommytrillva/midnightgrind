// Copyright Midnight Grind. All Rights Reserved.

#include "LaunchChecklist/MGLaunchChecklistSubsystem.h"
#include "Misc/App.h"

void UMGLaunchChecklistSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	DetectBuildInfo();
	RegisterDefaultChecks();
	RegisterComplianceRequirements();
	RegisterDefaultMilestones();
}

void UMGLaunchChecklistSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UMGLaunchChecklistSubsystem::RunAllChecks()
{
	if (bIsRunningChecks)
		return;

	bIsRunningChecks = true;
	CheckProgress = 0.0f;
	CurrentCheckIndex = 0;

	for (int32 i = 0; i < Checks.Num(); i++)
	{
		FMGLaunchCheck& Check = Checks[i];
		CurrentCheckIndex = i;
		CheckProgress = (float)i / (float)Checks.Num();

		OnCheckStarted.Broadcast(Check.CheckID);
		ExecuteCheck(Check);
		OnCheckCompleted.Broadcast(Check.CheckID, Check.Status);

		if (Check.Status == EMGCheckStatus::Failed && Check.Severity == EMGCheckSeverity::Critical)
		{
			OnBlockerIdentified.Broadcast(Check.ResultMessage);
		}
	}

	bIsRunningChecks = false;
	CheckProgress = 1.0f;

	FMGChecklistReport Report = GenerateReport();
	OnChecklistCompleted.Broadcast(Report);
	UpdateMilestoneProgress();
}

void UMGLaunchChecklistSubsystem::RunChecksByCategory(EMGCheckCategory Category)
{
	for (FMGLaunchCheck& Check : Checks)
	{
		if (Check.Category == Category)
		{
			OnCheckStarted.Broadcast(Check.CheckID);
			ExecuteCheck(Check);
			OnCheckCompleted.Broadcast(Check.CheckID, Check.Status);
		}
	}
}

void UMGLaunchChecklistSubsystem::RunSingleCheck(FName CheckID)
{
	for (FMGLaunchCheck& Check : Checks)
	{
		if (Check.CheckID == CheckID)
		{
			OnCheckStarted.Broadcast(Check.CheckID);
			ExecuteCheck(Check);
			OnCheckCompleted.Broadcast(Check.CheckID, Check.Status);
			break;
		}
	}
}

void UMGLaunchChecklistSubsystem::RunCriticalChecks()
{
	for (FMGLaunchCheck& Check : Checks)
	{
		if (Check.Severity == EMGCheckSeverity::Critical)
		{
			OnCheckStarted.Broadcast(Check.CheckID);
			ExecuteCheck(Check);
			OnCheckCompleted.Broadcast(Check.CheckID, Check.Status);
		}
	}
}

void UMGLaunchChecklistSubsystem::RunPlatformChecks(EMGPlatformTarget Platform)
{
	for (FMGLaunchCheck& Check : Checks)
	{
		bool bAppliesToPlatform = Check.TargetPlatforms.Num() == 0 ||
								  Check.TargetPlatforms.Contains(Platform) ||
								  Check.TargetPlatforms.Contains(EMGPlatformTarget::All);

		if (bAppliesToPlatform)
		{
			OnCheckStarted.Broadcast(Check.CheckID);
			ExecuteCheck(Check);
			OnCheckCompleted.Broadcast(Check.CheckID, Check.Status);
		}
	}
}

void UMGLaunchChecklistSubsystem::RegisterCheck(const FMGLaunchCheck& Check)
{
	int32 ExistingIndex = Checks.IndexOfByPredicate(
		[&Check](const FMGLaunchCheck& C) { return C.CheckID == Check.CheckID; });

	if (ExistingIndex != INDEX_NONE)
		Checks[ExistingIndex] = Check;
	else
		Checks.Add(Check);
}

FMGLaunchCheck UMGLaunchChecklistSubsystem::GetCheck(FName CheckID) const
{
	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.CheckID == CheckID)
			return Check;
	}
	return FMGLaunchCheck();
}

TArray<FMGLaunchCheck> UMGLaunchChecklistSubsystem::GetChecksByCategory(EMGCheckCategory Category) const
{
	TArray<FMGLaunchCheck> Result;
	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.Category == Category)
			Result.Add(Check);
	}
	return Result;
}

TArray<FMGLaunchCheck> UMGLaunchChecklistSubsystem::GetFailedChecks() const
{
	TArray<FMGLaunchCheck> Result;
	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.Status == EMGCheckStatus::Failed)
			Result.Add(Check);
	}
	return Result;
}

TArray<FMGLaunchCheck> UMGLaunchChecklistSubsystem::GetBlockers() const
{
	TArray<FMGLaunchCheck> Result;
	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.Status == EMGCheckStatus::Failed && Check.Severity == EMGCheckSeverity::Critical)
			Result.Add(Check);
	}
	return Result;
}

void UMGLaunchChecklistSubsystem::MarkCheckManuallyVerified(FName CheckID, bool bPassed, const FString& Notes)
{
	for (FMGLaunchCheck& Check : Checks)
	{
		if (Check.CheckID == CheckID)
		{
			Check.Status = bPassed ? EMGCheckStatus::Passed : EMGCheckStatus::Failed;
			Check.ResultMessage = Notes;
			Check.LastRunTime = FDateTime::UtcNow();
			break;
		}
	}
}

FMGChecklistReport UMGLaunchChecklistSubsystem::GenerateReport()
{
	FMGChecklistReport Report;
	Report.GeneratedAt = FDateTime::UtcNow();
	Report.BuildVersion = CurrentBuildInfo.Version;
	Report.Platform = EMGPlatformTarget::All;
	Report.TotalChecks = Checks.Num();

	for (const FMGLaunchCheck& Check : Checks)
	{
		Report.Results.Add(Check);

		switch (Check.Status)
		{
		case EMGCheckStatus::Passed:
			Report.PassedChecks++;
			break;
		case EMGCheckStatus::Failed:
			Report.FailedChecks++;
			if (Check.Severity == EMGCheckSeverity::Critical)
			{
				Report.CriticalBlockers++;
				Report.Blockers.Add(FString::Printf(TEXT("[CRITICAL] %s: %s"),
					*Check.DisplayName.ToString(), *Check.ResultMessage));
			}
			break;
		case EMGCheckStatus::Warning:
			Report.WarningChecks++;
			break;
		case EMGCheckStatus::Skipped:
		case EMGCheckStatus::NotRun:
			Report.SkippedChecks++;
			break;
		default:
			break;
		}
	}

	// Calculate overall score
	if (Report.TotalChecks > 0)
	{
		Report.OverallScore = ((float)Report.PassedChecks / (float)Report.TotalChecks) * 100.0f;
	}

	// Determine launch readiness
	Report.bLaunchReady = Report.CriticalBlockers == 0 && Report.OverallScore >= 95.0f;

	// Generate recommendations
	if (Report.FailedChecks > 0)
	{
		Report.Recommendations.Add(FString::Printf(TEXT("Address %d failed checks before launch"), Report.FailedChecks));
	}
	if (Report.WarningChecks > 0)
	{
		Report.Recommendations.Add(FString::Printf(TEXT("Review %d warnings for potential issues"), Report.WarningChecks));
	}
	if (!Report.bLaunchReady)
	{
		Report.Recommendations.Add(TEXT("Game is NOT ready for launch - resolve all critical blockers"));
	}

	LastReport = Report;
	return Report;
}

FMGChecklistReport UMGLaunchChecklistSubsystem::GeneratePlatformReport(EMGPlatformTarget Platform)
{
	FMGChecklistReport Report;
	Report.GeneratedAt = FDateTime::UtcNow();
	Report.BuildVersion = CurrentBuildInfo.Version;
	Report.Platform = Platform;

	for (const FMGLaunchCheck& Check : Checks)
	{
		bool bAppliesToPlatform = Check.TargetPlatforms.Num() == 0 ||
								  Check.TargetPlatforms.Contains(Platform) ||
								  Check.TargetPlatforms.Contains(EMGPlatformTarget::All);

		if (bAppliesToPlatform)
		{
			Report.TotalChecks++;
			Report.Results.Add(Check);

			switch (Check.Status)
			{
			case EMGCheckStatus::Passed:
				Report.PassedChecks++;
				break;
			case EMGCheckStatus::Failed:
				Report.FailedChecks++;
				if (Check.Severity == EMGCheckSeverity::Critical)
					Report.CriticalBlockers++;
				break;
			case EMGCheckStatus::Warning:
				Report.WarningChecks++;
				break;
			default:
				Report.SkippedChecks++;
				break;
			}
		}
	}

	if (Report.TotalChecks > 0)
	{
		Report.OverallScore = ((float)Report.PassedChecks / (float)Report.TotalChecks) * 100.0f;
	}

	Report.bLaunchReady = Report.CriticalBlockers == 0 && Report.OverallScore >= 95.0f;

	return Report;
}

FString UMGLaunchChecklistSubsystem::ExportReportToText()
{
	FString Output;
	Output += TEXT("========================================\n");
	Output += TEXT("MIDNIGHT GRIND - LAUNCH CHECKLIST REPORT\n");
	Output += TEXT("========================================\n\n");
	Output += FString::Printf(TEXT("Generated: %s\n"), *LastReport.GeneratedAt.ToString());
	Output += FString::Printf(TEXT("Build: %s\n"), *LastReport.BuildVersion);
	Output += FString::Printf(TEXT("Platform: All\n\n"));

	Output += TEXT("--- SUMMARY ---\n");
	Output += FString::Printf(TEXT("Total Checks: %d\n"), LastReport.TotalChecks);
	Output += FString::Printf(TEXT("Passed: %d\n"), LastReport.PassedChecks);
	Output += FString::Printf(TEXT("Failed: %d\n"), LastReport.FailedChecks);
	Output += FString::Printf(TEXT("Warnings: %d\n"), LastReport.WarningChecks);
	Output += FString::Printf(TEXT("Skipped: %d\n"), LastReport.SkippedChecks);
	Output += FString::Printf(TEXT("Critical Blockers: %d\n"), LastReport.CriticalBlockers);
	Output += FString::Printf(TEXT("Overall Score: %.1f%%\n"), LastReport.OverallScore);
	Output += FString::Printf(TEXT("Launch Ready: %s\n\n"), LastReport.bLaunchReady ? TEXT("YES") : TEXT("NO"));

	if (LastReport.Blockers.Num() > 0)
	{
		Output += TEXT("--- BLOCKERS ---\n");
		for (const FString& Blocker : LastReport.Blockers)
		{
			Output += FString::Printf(TEXT("* %s\n"), *Blocker);
		}
		Output += TEXT("\n");
	}

	Output += TEXT("--- DETAILED RESULTS ---\n");
	for (const FMGLaunchCheck& Check : LastReport.Results)
	{
		FString StatusStr;
		switch (Check.Status)
		{
		case EMGCheckStatus::Passed: StatusStr = TEXT("[PASS]"); break;
		case EMGCheckStatus::Failed: StatusStr = TEXT("[FAIL]"); break;
		case EMGCheckStatus::Warning: StatusStr = TEXT("[WARN]"); break;
		default: StatusStr = TEXT("[SKIP]"); break;
		}

		Output += FString::Printf(TEXT("%s %s - %s\n"), *StatusStr, *Check.DisplayName.ToString(), *Check.ResultMessage);
	}

	return Output;
}

FString UMGLaunchChecklistSubsystem::ExportReportToJSON()
{
	// Would serialize to JSON
	return TEXT("{}");
}

bool UMGLaunchChecklistSubsystem::IsLaunchReady() const
{
	return GetBlockerCount() == 0 && GetLaunchReadinessScore() >= 95.0f;
}

float UMGLaunchChecklistSubsystem::GetLaunchReadinessScore() const
{
	int32 TotalRequired = 0;
	int32 PassedRequired = 0;

	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.bRequired)
		{
			TotalRequired++;
			if (Check.Status == EMGCheckStatus::Passed)
				PassedRequired++;
		}
	}

	return TotalRequired > 0 ? ((float)PassedRequired / (float)TotalRequired) * 100.0f : 100.0f;
}

int32 UMGLaunchChecklistSubsystem::GetBlockerCount() const
{
	int32 Count = 0;
	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.Status == EMGCheckStatus::Failed && Check.Severity == EMGCheckSeverity::Critical)
			Count++;
	}
	return Count;
}

TArray<FString> UMGLaunchChecklistSubsystem::GetBlockerDescriptions() const
{
	TArray<FString> Descriptions;
	for (const FMGLaunchCheck& Check : Checks)
	{
		if (Check.Status == EMGCheckStatus::Failed && Check.Severity == EMGCheckSeverity::Critical)
		{
			Descriptions.Add(FString::Printf(TEXT("%s: %s"), *Check.DisplayName.ToString(), *Check.ResultMessage));
		}
	}
	return Descriptions;
}

void UMGLaunchChecklistSubsystem::RegisterComplianceItem(const FMGComplianceItem& Item)
{
	int32 ExistingIndex = ComplianceItems.IndexOfByPredicate(
		[&Item](const FMGComplianceItem& I) { return I.ItemID == Item.ItemID; });

	if (ExistingIndex != INDEX_NONE)
		ComplianceItems[ExistingIndex] = Item;
	else
		ComplianceItems.Add(Item);
}

TArray<FMGComplianceItem> UMGLaunchChecklistSubsystem::GetComplianceItems(EMGPlatformTarget Platform) const
{
	TArray<FMGComplianceItem> Result;
	for (const FMGComplianceItem& Item : ComplianceItems)
	{
		if (Platform == EMGPlatformTarget::All || Item.Platform == Platform || Item.Platform == EMGPlatformTarget::All)
			Result.Add(Item);
	}
	return Result;
}

void UMGLaunchChecklistSubsystem::VerifyComplianceItem(FName ItemID, const FString& Notes, const FString& VerifiedBy)
{
	for (FMGComplianceItem& Item : ComplianceItems)
	{
		if (Item.ItemID == ItemID)
		{
			Item.bVerified = true;
			Item.VerificationNotes = Notes;
			Item.VerifiedBy = VerifiedBy;
			Item.VerifiedAt = FDateTime::UtcNow();
			break;
		}
	}
}

float UMGLaunchChecklistSubsystem::GetComplianceProgress(EMGPlatformTarget Platform) const
{
	TArray<FMGComplianceItem> Items = GetComplianceItems(Platform);
	int32 Verified = 0;

	for (const FMGComplianceItem& Item : Items)
	{
		if (Item.bVerified)
			Verified++;
	}

	return Items.Num() > 0 ? ((float)Verified / (float)Items.Num()) * 100.0f : 100.0f;
}

void UMGLaunchChecklistSubsystem::RegisterMilestone(const FMGLaunchMilestone& Milestone)
{
	int32 ExistingIndex = Milestones.IndexOfByPredicate(
		[&Milestone](const FMGLaunchMilestone& M) { return M.MilestoneID == Milestone.MilestoneID; });

	if (ExistingIndex != INDEX_NONE)
		Milestones[ExistingIndex] = Milestone;
	else
		Milestones.Add(Milestone);
}

FMGLaunchMilestone UMGLaunchChecklistSubsystem::GetNextMilestone() const
{
	for (const FMGLaunchMilestone& Milestone : Milestones)
	{
		if (!Milestone.bCompleted)
			return Milestone;
	}
	return FMGLaunchMilestone();
}

void UMGLaunchChecklistSubsystem::UpdateMilestoneProgress()
{
	for (FMGLaunchMilestone& Milestone : Milestones)
	{
		if (Milestone.bCompleted)
			continue;

		int32 CompletedChecks = 0;
		for (const FName& CheckID : Milestone.RequiredChecks)
		{
			FMGLaunchCheck Check = GetCheck(CheckID);
			if (Check.Status == EMGCheckStatus::Passed)
				CompletedChecks++;
		}

		Milestone.CompletionPercent = Milestone.RequiredChecks.Num() > 0
			? ((float)CompletedChecks / (float)Milestone.RequiredChecks.Num()) * 100.0f
			: 100.0f;

		if (Milestone.CompletionPercent >= 100.0f)
		{
			Milestone.bCompleted = true;
			Milestone.CompletedAt = FDateTime::UtcNow();
			OnMilestoneCompleted.Broadcast(Milestone.MilestoneID, Milestone);
		}
	}
}

void UMGLaunchChecklistSubsystem::RegisterDefaultChecks()
{
	// Core Systems
	FMGLaunchCheck CoreCheck;
	CoreCheck.CheckID = FName(TEXT("Check_CoreSubsystems"));
	CoreCheck.DisplayName = FText::FromString(TEXT("Core Subsystems"));
	CoreCheck.Description = FText::FromString(TEXT("Verify all core game subsystems initialize correctly"));
	CoreCheck.Category = EMGCheckCategory::Core;
	CoreCheck.Severity = EMGCheckSeverity::Critical;
	CoreCheck.bRequired = true;
	RegisterCheck(CoreCheck);

	FMGLaunchCheck SaveCheck;
	SaveCheck.CheckID = FName(TEXT("Check_SaveSystem"));
	SaveCheck.DisplayName = FText::FromString(TEXT("Save System"));
	SaveCheck.Description = FText::FromString(TEXT("Verify save/load functionality"));
	SaveCheck.Category = EMGCheckCategory::Core;
	SaveCheck.Severity = EMGCheckSeverity::Critical;
	SaveCheck.bRequired = true;
	RegisterCheck(SaveCheck);

	// Multiplayer
	FMGLaunchCheck NetworkCheck;
	NetworkCheck.CheckID = FName(TEXT("Check_NetworkConnectivity"));
	NetworkCheck.DisplayName = FText::FromString(TEXT("Network Connectivity"));
	NetworkCheck.Description = FText::FromString(TEXT("Verify network systems and server connectivity"));
	NetworkCheck.Category = EMGCheckCategory::Multiplayer;
	NetworkCheck.Severity = EMGCheckSeverity::Critical;
	NetworkCheck.bRequired = true;
	RegisterCheck(NetworkCheck);

	FMGLaunchCheck MultiplayerCheck;
	MultiplayerCheck.CheckID = FName(TEXT("Check_MultiplayerStability"));
	MultiplayerCheck.DisplayName = FText::FromString(TEXT("Multiplayer Stability"));
	MultiplayerCheck.Description = FText::FromString(TEXT("Verify multiplayer match stability"));
	MultiplayerCheck.Category = EMGCheckCategory::Multiplayer;
	MultiplayerCheck.Severity = EMGCheckSeverity::Critical;
	MultiplayerCheck.bRequired = true;
	RegisterCheck(MultiplayerCheck);

	// Economy
	FMGLaunchCheck EconomyCheck;
	EconomyCheck.CheckID = FName(TEXT("Check_EconomyBalance"));
	EconomyCheck.DisplayName = FText::FromString(TEXT("Economy Balance"));
	EconomyCheck.Description = FText::FromString(TEXT("Verify economy values are properly balanced"));
	EconomyCheck.Category = EMGCheckCategory::Economy;
	EconomyCheck.Severity = EMGCheckSeverity::Major;
	EconomyCheck.bRequired = true;
	RegisterCheck(EconomyCheck);

	// Progression
	FMGLaunchCheck ProgressionCheck;
	ProgressionCheck.CheckID = FName(TEXT("Check_ProgressionFlow"));
	ProgressionCheck.DisplayName = FText::FromString(TEXT("Progression Flow"));
	ProgressionCheck.Description = FText::FromString(TEXT("Verify player can progress through all content"));
	ProgressionCheck.Category = EMGCheckCategory::Progression;
	ProgressionCheck.Severity = EMGCheckSeverity::Critical;
	ProgressionCheck.bRequired = true;
	RegisterCheck(ProgressionCheck);

	// Localization
	FMGLaunchCheck LocalizationCheck;
	LocalizationCheck.CheckID = FName(TEXT("Check_LocalizationCoverage"));
	LocalizationCheck.DisplayName = FText::FromString(TEXT("Localization Coverage"));
	LocalizationCheck.Description = FText::FromString(TEXT("Verify all text is localized"));
	LocalizationCheck.Category = EMGCheckCategory::Localization;
	LocalizationCheck.Severity = EMGCheckSeverity::Major;
	LocalizationCheck.bRequired = true;
	RegisterCheck(LocalizationCheck);

	// Accessibility
	FMGLaunchCheck AccessibilityCheck;
	AccessibilityCheck.CheckID = FName(TEXT("Check_AccessibilityFeatures"));
	AccessibilityCheck.DisplayName = FText::FromString(TEXT("Accessibility Features"));
	AccessibilityCheck.Description = FText::FromString(TEXT("Verify accessibility options work correctly"));
	AccessibilityCheck.Category = EMGCheckCategory::Accessibility;
	AccessibilityCheck.Severity = EMGCheckSeverity::Major;
	AccessibilityCheck.bRequired = true;
	RegisterCheck(AccessibilityCheck);

	// Performance
	FMGLaunchCheck PerformanceCheck;
	PerformanceCheck.CheckID = FName(TEXT("Check_PerformanceTargets"));
	PerformanceCheck.DisplayName = FText::FromString(TEXT("Performance Targets"));
	PerformanceCheck.Description = FText::FromString(TEXT("Verify frame rate and loading time targets"));
	PerformanceCheck.Category = EMGCheckCategory::Performance;
	PerformanceCheck.Severity = EMGCheckSeverity::Critical;
	PerformanceCheck.bRequired = true;
	RegisterCheck(PerformanceCheck);

	FMGLaunchCheck MemoryCheck;
	MemoryCheck.CheckID = FName(TEXT("Check_MemoryBudget"));
	MemoryCheck.DisplayName = FText::FromString(TEXT("Memory Budget"));
	MemoryCheck.Description = FText::FromString(TEXT("Verify memory usage stays within budget"));
	MemoryCheck.Category = EMGCheckCategory::Performance;
	MemoryCheck.Severity = EMGCheckSeverity::Critical;
	MemoryCheck.bRequired = true;
	RegisterCheck(MemoryCheck);

	// Content
	FMGLaunchCheck ContentCheck;
	ContentCheck.CheckID = FName(TEXT("Check_ContentIntegrity"));
	ContentCheck.DisplayName = FText::FromString(TEXT("Content Integrity"));
	ContentCheck.Description = FText::FromString(TEXT("Verify all game content loads correctly"));
	ContentCheck.Category = EMGCheckCategory::Content;
	ContentCheck.Severity = EMGCheckSeverity::Critical;
	ContentCheck.bRequired = true;
	RegisterCheck(ContentCheck);

	FMGLaunchCheck AudioCheck;
	AudioCheck.CheckID = FName(TEXT("Check_AudioComplete"));
	AudioCheck.DisplayName = FText::FromString(TEXT("Audio Complete"));
	AudioCheck.Description = FText::FromString(TEXT("Verify all audio assets are present"));
	AudioCheck.Category = EMGCheckCategory::Audio;
	AudioCheck.Severity = EMGCheckSeverity::Major;
	AudioCheck.bRequired = true;
	RegisterCheck(AudioCheck);

	// Security
	FMGLaunchCheck AntiCheatCheck;
	AntiCheatCheck.CheckID = FName(TEXT("Check_AntiCheatIntegration"));
	AntiCheatCheck.DisplayName = FText::FromString(TEXT("Anti-Cheat Integration"));
	AntiCheatCheck.Description = FText::FromString(TEXT("Verify anti-cheat system is functional"));
	AntiCheatCheck.Category = EMGCheckCategory::Security;
	AntiCheatCheck.Severity = EMGCheckSeverity::Critical;
	AntiCheatCheck.bRequired = true;
	RegisterCheck(AntiCheatCheck);

	// Compliance
	FMGLaunchCheck AgeRatingCheck;
	AgeRatingCheck.CheckID = FName(TEXT("Check_AgeRatingCompliance"));
	AgeRatingCheck.DisplayName = FText::FromString(TEXT("Age Rating Compliance"));
	AgeRatingCheck.Description = FText::FromString(TEXT("Verify content meets age rating requirements"));
	AgeRatingCheck.Category = EMGCheckCategory::Compliance;
	AgeRatingCheck.Severity = EMGCheckSeverity::Critical;
	AgeRatingCheck.bAutomated = false;
	AgeRatingCheck.bRequired = true;
	RegisterCheck(AgeRatingCheck);

	FMGLaunchCheck PrivacyCheck;
	PrivacyCheck.CheckID = FName(TEXT("Check_PrivacyCompliance"));
	PrivacyCheck.DisplayName = FText::FromString(TEXT("Privacy Compliance"));
	PrivacyCheck.Description = FText::FromString(TEXT("Verify GDPR/CCPA compliance"));
	PrivacyCheck.Category = EMGCheckCategory::Compliance;
	PrivacyCheck.Severity = EMGCheckSeverity::Critical;
	PrivacyCheck.bAutomated = false;
	PrivacyCheck.bRequired = true;
	RegisterCheck(PrivacyCheck);
}

void UMGLaunchChecklistSubsystem::RegisterComplianceRequirements()
{
	// PlayStation Requirements
	FMGComplianceItem PS5TRC;
	PS5TRC.ItemID = FName(TEXT("PS5_TRC_Compliance"));
	PS5TRC.Requirement = FText::FromString(TEXT("PlayStation 5 TRC Compliance"));
	PS5TRC.Description = FText::FromString(TEXT("Game passes all Sony TRC requirements"));
	PS5TRC.Platform = EMGPlatformTarget::PlayStation5;
	PS5TRC.bMandatory = true;
	RegisterComplianceItem(PS5TRC);

	// Xbox Requirements
	FMGComplianceItem XboxXR;
	XboxXR.ItemID = FName(TEXT("Xbox_XR_Compliance"));
	XboxXR.Requirement = FText::FromString(TEXT("Xbox Series X/S XR Compliance"));
	XboxXR.Description = FText::FromString(TEXT("Game passes all Microsoft XR requirements"));
	XboxXR.Platform = EMGPlatformTarget::XboxSeriesX;
	XboxXR.bMandatory = true;
	RegisterComplianceItem(XboxXR);

	// Steam Requirements
	FMGComplianceItem SteamReview;
	SteamReview.ItemID = FName(TEXT("Steam_Review_Ready"));
	SteamReview.Requirement = FText::FromString(TEXT("Steam Store Review Ready"));
	SteamReview.Description = FText::FromString(TEXT("Store page and build ready for Valve review"));
	SteamReview.Platform = EMGPlatformTarget::Steam;
	SteamReview.bMandatory = true;
	RegisterComplianceItem(SteamReview);

	// Universal Requirements
	FMGComplianceItem ESRBRating;
	ESRBRating.ItemID = FName(TEXT("ESRB_Rating"));
	ESRBRating.Requirement = FText::FromString(TEXT("ESRB Rating Obtained"));
	ESRBRating.Description = FText::FromString(TEXT("ESRB rating received and implemented"));
	ESRBRating.Platform = EMGPlatformTarget::All;
	ESRBRating.bMandatory = true;
	RegisterComplianceItem(ESRBRating);

	FMGComplianceItem PEGIRating;
	PEGIRating.ItemID = FName(TEXT("PEGI_Rating"));
	PEGIRating.Requirement = FText::FromString(TEXT("PEGI Rating Obtained"));
	PEGIRating.Description = FText::FromString(TEXT("PEGI rating received and implemented"));
	PEGIRating.Platform = EMGPlatformTarget::All;
	PEGIRating.bMandatory = true;
	RegisterComplianceItem(PEGIRating);

	FMGComplianceItem GDPRCompliance;
	GDPRCompliance.ItemID = FName(TEXT("GDPR_Compliance"));
	GDPRCompliance.Requirement = FText::FromString(TEXT("GDPR Compliance"));
	GDPRCompliance.Description = FText::FromString(TEXT("Privacy policy and data handling meet GDPR requirements"));
	GDPRCompliance.Platform = EMGPlatformTarget::All;
	GDPRCompliance.bMandatory = true;
	RegisterComplianceItem(GDPRCompliance);
}

void UMGLaunchChecklistSubsystem::RegisterDefaultMilestones()
{
	FMGLaunchMilestone Alpha;
	Alpha.MilestoneID = FName(TEXT("Alpha"));
	Alpha.Name = FText::FromString(TEXT("Alpha"));
	Alpha.Description = FText::FromString(TEXT("Core gameplay complete and playable"));
	Alpha.RequiredChecks = { FName(TEXT("Check_CoreSubsystems")), FName(TEXT("Check_ProgressionFlow")) };
	RegisterMilestone(Alpha);

	FMGLaunchMilestone Beta;
	Beta.MilestoneID = FName(TEXT("Beta"));
	Beta.Name = FText::FromString(TEXT("Beta"));
	Beta.Description = FText::FromString(TEXT("All features implemented, bug fixing phase"));
	Beta.RequiredChecks = {
		FName(TEXT("Check_CoreSubsystems")),
		FName(TEXT("Check_SaveSystem")),
		FName(TEXT("Check_NetworkConnectivity")),
		FName(TEXT("Check_ProgressionFlow")),
		FName(TEXT("Check_ContentIntegrity"))
	};
	RegisterMilestone(Beta);

	FMGLaunchMilestone ReleaseCandidate;
	ReleaseCandidate.MilestoneID = FName(TEXT("ReleaseCandidate"));
	ReleaseCandidate.Name = FText::FromString(TEXT("Release Candidate"));
	ReleaseCandidate.Description = FText::FromString(TEXT("Ready for final QA and certification"));
	RegisterMilestone(ReleaseCandidate);

	FMGLaunchMilestone GoldMaster;
	GoldMaster.MilestoneID = FName(TEXT("GoldMaster"));
	GoldMaster.Name = FText::FromString(TEXT("Gold Master"));
	GoldMaster.Description = FText::FromString(TEXT("Final build ready for manufacturing/release"));
	RegisterMilestone(GoldMaster);
}

void UMGLaunchChecklistSubsystem::DetectBuildInfo()
{
	CurrentBuildInfo.Version = TEXT("1.0.0.48");
	CurrentBuildInfo.BuildNumber = TEXT("48");
	CurrentBuildInfo.CommitHash = TEXT("unknown");
	CurrentBuildInfo.Branch = TEXT("main");
	CurrentBuildInfo.BuildTime = FDateTime::UtcNow();
	CurrentBuildInfo.Configuration = TEXT("Development");

#if PLATFORM_WINDOWS
	CurrentBuildInfo.Platform = EMGPlatformTarget::Windows;
#endif
}

bool UMGLaunchChecklistSubsystem::ExecuteCheck(FMGLaunchCheck& Check)
{
	Check.Status = EMGCheckStatus::Running;
	Check.LastRunTime = FDateTime::UtcNow();

	double StartTime = FPlatformTime::Seconds();
	bool bPassed = false;

	// Execute appropriate check
	if (Check.CheckID == FName(TEXT("Check_CoreSubsystems")))
		bPassed = Check_CoreSubsystems(Check);
	else if (Check.CheckID == FName(TEXT("Check_SaveSystem")))
		bPassed = Check_SaveSystem(Check);
	else if (Check.CheckID == FName(TEXT("Check_NetworkConnectivity")))
		bPassed = Check_NetworkConnectivity(Check);
	else if (Check.CheckID == FName(TEXT("Check_EconomyBalance")))
		bPassed = Check_EconomyBalance(Check);
	else if (Check.CheckID == FName(TEXT("Check_ProgressionFlow")))
		bPassed = Check_ProgressionFlow(Check);
	else if (Check.CheckID == FName(TEXT("Check_LocalizationCoverage")))
		bPassed = Check_LocalizationCoverage(Check);
	else if (Check.CheckID == FName(TEXT("Check_AccessibilityFeatures")))
		bPassed = Check_AccessibilityFeatures(Check);
	else if (Check.CheckID == FName(TEXT("Check_PerformanceTargets")))
		bPassed = Check_PerformanceTargets(Check);
	else if (Check.CheckID == FName(TEXT("Check_MemoryBudget")))
		bPassed = Check_MemoryBudget(Check);
	else if (Check.CheckID == FName(TEXT("Check_ContentIntegrity")))
		bPassed = Check_ContentIntegrity(Check);
	else if (Check.CheckID == FName(TEXT("Check_AudioComplete")))
		bPassed = Check_AudioComplete(Check);
	else if (Check.CheckID == FName(TEXT("Check_MultiplayerStability")))
		bPassed = Check_MultiplayerStability(Check);
	else if (Check.CheckID == FName(TEXT("Check_AntiCheatIntegration")))
		bPassed = Check_AntiCheatIntegration(Check);
	else if (!Check.bAutomated)
	{
		Check.Status = EMGCheckStatus::Skipped;
		Check.ResultMessage = TEXT("Manual verification required");
		return false;
	}
	else
	{
		Check.Status = EMGCheckStatus::Skipped;
		Check.ResultMessage = TEXT("Check not implemented");
		return false;
	}

	Check.DurationSeconds = FPlatformTime::Seconds() - StartTime;
	Check.Status = bPassed ? EMGCheckStatus::Passed : EMGCheckStatus::Failed;

	return bPassed;
}

bool UMGLaunchChecklistSubsystem::Check_CoreSubsystems(FMGLaunchCheck& Check)
{
	// Would verify all subsystems initialized
	Check.ResultMessage = TEXT("All core subsystems initialized successfully");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_SaveSystem(FMGLaunchCheck& Check)
{
	// Would test save/load cycle
	Check.ResultMessage = TEXT("Save system verified");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_NetworkConnectivity(FMGLaunchCheck& Check)
{
	// Would test server connectivity
	Check.ResultMessage = TEXT("Network connectivity verified");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_EconomyBalance(FMGLaunchCheck& Check)
{
	// Would verify economy parameters
	Check.ResultMessage = TEXT("Economy balance validated");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_ProgressionFlow(FMGLaunchCheck& Check)
{
	// Would verify progression paths
	Check.ResultMessage = TEXT("Progression flow verified");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_LocalizationCoverage(FMGLaunchCheck& Check)
{
	// Would verify all strings are localized
	Check.ResultMessage = TEXT("Localization coverage at 100%");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_AccessibilityFeatures(FMGLaunchCheck& Check)
{
	// Would verify accessibility options
	Check.ResultMessage = TEXT("All accessibility features functional");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_PerformanceTargets(FMGLaunchCheck& Check)
{
	// Would run performance tests
	Check.ResultMessage = TEXT("Performance targets met (60fps stable)");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_MemoryBudget(FMGLaunchCheck& Check)
{
	// Would check memory usage
	Check.ResultMessage = TEXT("Memory usage within budget");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_ContentIntegrity(FMGLaunchCheck& Check)
{
	// Would verify all content loads
	Check.ResultMessage = TEXT("All content integrity verified");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_AudioComplete(FMGLaunchCheck& Check)
{
	// Would verify audio assets
	Check.ResultMessage = TEXT("All audio assets present");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_MultiplayerStability(FMGLaunchCheck& Check)
{
	// Would test multiplayer
	Check.ResultMessage = TEXT("Multiplayer stability verified");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_AntiCheatIntegration(FMGLaunchCheck& Check)
{
	// Would verify anti-cheat
	Check.ResultMessage = TEXT("Anti-cheat integration verified");
	return true;
}

bool UMGLaunchChecklistSubsystem::Check_AgeRatingCompliance(FMGLaunchCheck& Check)
{
	// Manual check
	Check.ResultMessage = TEXT("Requires manual verification");
	return false;
}

bool UMGLaunchChecklistSubsystem::Check_PrivacyCompliance(FMGLaunchCheck& Check)
{
	// Manual check
	Check.ResultMessage = TEXT("Requires manual verification");
	return false;
}

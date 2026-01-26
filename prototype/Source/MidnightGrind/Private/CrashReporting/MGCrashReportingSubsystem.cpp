// Copyright Midnight Grind. All Rights Reserved.

#include "CrashReporting/MGCrashReportingSubsystem.h"
#include "Misc/App.h"
#include "GenericPlatform/GenericPlatformCrashContext.h"

void UMGCrashReportingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BuildVersion = FApp::GetBuildVersion();
	CurrentSessionID = FGuid::NewGuid().ToString();

	if (bAutoCapture)
	{
		InstallCrashHandler();
	}

	AddBreadcrumb(TEXT("System"), TEXT("Crash reporting initialized"));
}

void UMGCrashReportingSubsystem::Deinitialize()
{
	AddBreadcrumb(TEXT("System"), TEXT("Session ending"));
	Super::Deinitialize();
}

void UMGCrashReportingSubsystem::ReportCrash(const FString& Message, EMGCrashSeverity Severity)
{
	if (!bIsEnabled)
		return;

	FMGCrashReport Report;
	Report.ReportID = FGuid::NewGuid().ToString();
	Report.Severity = Severity;
	Report.Message = Message;
	Report.StackTrace = GenerateStackTrace();
	Report.BuildVersion = BuildVersion;
	Report.Platform = FPlatformProperties::IniPlatformName();
	Report.Timestamp = FDateTime::UtcNow();
	Report.SessionID = CurrentSessionID;
	Report.ReplayID = CurrentReplayID;

	for (const auto& Tag : Tags)
	{
		Report.Metadata.Add(Tag.Key, Tag.Value);
	}

	Report.Metadata.Add(TEXT("UserID"), CurrentUserID);

	UploadReport(Report);
	OnCrashReported.Broadcast(Report);
}

void UMGCrashReportingSubsystem::ReportException(const FString& ExceptionType, const FString& Message)
{
	AddBreadcrumb(TEXT("Exception"), FString::Printf(TEXT("%s: %s"), *ExceptionType, *Message));
	ReportCrash(FString::Printf(TEXT("[%s] %s"), *ExceptionType, *Message), EMGCrashSeverity::Error);
}

void UMGCrashReportingSubsystem::ReportError(const FString& ErrorCode, const FString& Message)
{
	AddBreadcrumb(TEXT("Error"), FString::Printf(TEXT("%s: %s"), *ErrorCode, *Message));
	ReportCrash(FString::Printf(TEXT("Error %s: %s"), *ErrorCode, *Message), EMGCrashSeverity::Error);
}

void UMGCrashReportingSubsystem::AddBreadcrumb(const FString& Category, const FString& Message)
{
	TMap<FString, FString> EmptyData;
	AddBreadcrumbWithData(Category, Message, EmptyData);
}

void UMGCrashReportingSubsystem::AddBreadcrumbWithData(const FString& Category, const FString& Message, const TMap<FString, FString>& Data)
{
	FMGBreadcrumb Crumb;
	Crumb.Category = Category;
	Crumb.Message = Message;
	Crumb.Timestamp = FDateTime::UtcNow();
	Crumb.Data = Data;

	Breadcrumbs.Add(Crumb);

	// Keep breadcrumbs within limit
	while (Breadcrumbs.Num() > MaxBreadcrumbs)
	{
		Breadcrumbs.RemoveAt(0);
	}
}

void UMGCrashReportingSubsystem::ClearBreadcrumbs()
{
	Breadcrumbs.Empty();
}

void UMGCrashReportingSubsystem::SetUserID(const FString& UserID)
{
	CurrentUserID = UserID;
	AddBreadcrumb(TEXT("User"), FString::Printf(TEXT("User identified: %s"), *UserID));
}

void UMGCrashReportingSubsystem::SetSessionID(const FString& SessionID)
{
	CurrentSessionID = SessionID;
}

void UMGCrashReportingSubsystem::AddTag(const FString& Key, const FString& Value)
{
	Tags.Add(Key, Value);
}

void UMGCrashReportingSubsystem::SetCurrentReplayID(const FString& ReplayID)
{
	CurrentReplayID = ReplayID;
	AddBreadcrumb(TEXT("Replay"), FString::Printf(TEXT("Replay started: %s"), *ReplayID));
}

void UMGCrashReportingSubsystem::SetEnabled(bool bEnabled)
{
	bIsEnabled = bEnabled;
}

void UMGCrashReportingSubsystem::SetAutoCapture(bool bAuto)
{
	bAutoCapture = bAuto;
	if (bAuto)
	{
		InstallCrashHandler();
	}
}

void UMGCrashReportingSubsystem::InstallCrashHandler()
{
	// Would install platform-specific crash handlers
	// FCoreDelegates::OnHandleSystemError would be used
}

void UMGCrashReportingSubsystem::UploadReport(const FMGCrashReport& Report)
{
	// Would upload to crash reporting service
	// Include breadcrumbs, metadata, stack trace
	// Include replay ID for bug reproduction

	UE_LOG(LogTemp, Warning, TEXT("Crash Report: %s - %s"), *Report.ReportID, *Report.Message);
}

FString UMGCrashReportingSubsystem::GenerateStackTrace()
{
	// Would capture actual stack trace
	TArray<FProgramCounterSymbolInfo> StackFrames;
	// FPlatformStackWalk::CaptureStackBackTrace would be used

	return TEXT("Stack trace capture placeholder");
}

void UMGCrashReportingSubsystem::OnEngineCrash()
{
	ReportCrash(TEXT("Engine crash detected"), EMGCrashSeverity::Fatal);
}

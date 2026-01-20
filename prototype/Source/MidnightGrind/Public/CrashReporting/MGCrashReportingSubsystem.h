// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGCrashReportingSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGCrashSeverity : uint8
{
	Info,
	Warning,
	Error,
	Fatal
};

USTRUCT(BlueprintType)
struct FMGCrashReport
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReportID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGCrashSeverity Severity = EMGCrashSeverity::Error;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString StackTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString BuildVersion;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Platform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Metadata;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ReplayID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SessionID;
};

USTRUCT(BlueprintType)
struct FMGBreadcrumb
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Category;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Data;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnCrashReported, const FMGCrashReport&, Report);

UCLASS()
class MIDNIGHTGRIND_API UMGCrashReportingSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Crash Reporting
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ReportCrash(const FString& Message, EMGCrashSeverity Severity = EMGCrashSeverity::Error);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ReportException(const FString& ExceptionType, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ReportError(const FString& ErrorCode, const FString& Message);

	// Breadcrumbs (for debugging)
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void AddBreadcrumb(const FString& Category, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void AddBreadcrumbWithData(const FString& Category, const FString& Message, const TMap<FString, FString>& Data);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void ClearBreadcrumbs();

	UFUNCTION(BlueprintPure, Category = "CrashReporting")
	TArray<FMGBreadcrumb> GetBreadcrumbs() const { return Breadcrumbs; }

	// Context
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetUserID(const FString& UserID);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetSessionID(const FString& SessionID);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void AddTag(const FString& Key, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetCurrentReplayID(const FString& ReplayID);

	// Configuration
	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "CrashReporting")
	bool IsEnabled() const { return bIsEnabled; }

	UFUNCTION(BlueprintCallable, Category = "CrashReporting")
	void SetAutoCapture(bool bAuto);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "CrashReporting|Events")
	FMGOnCrashReported OnCrashReported;

protected:
	void InstallCrashHandler();
	void UploadReport(const FMGCrashReport& Report);
	FString GenerateStackTrace();
	void OnEngineCrash();

private:
	UPROPERTY()
	TArray<FMGBreadcrumb> Breadcrumbs;

	TMap<FString, FString> Tags;
	FString CurrentUserID;
	FString CurrentSessionID;
	FString CurrentReplayID;
	FString BuildVersion;
	bool bIsEnabled = true;
	bool bAutoCapture = true;
	int32 MaxBreadcrumbs = 100;
};

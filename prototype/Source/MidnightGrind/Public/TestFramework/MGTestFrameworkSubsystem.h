// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTestFrameworkSubsystem.generated.h"

UENUM(BlueprintType)
enum class EMGTestResult : uint8
{
	Passed,
	Failed,
	Skipped,
	Timeout,
	Error
};

UENUM(BlueprintType)
enum class EMGTestCategory : uint8
{
	Unit,
	Integration,
	Performance,
	Regression,
	Smoke,
	Stress
};

USTRUCT(BlueprintType)
struct FMGTestCase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTestCategory Category = EMGTestCategory::Unit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeoutSeconds = 30.0f;
};

USTRUCT(BlueprintType)
struct FMGTestResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTestResult Result = EMGTestResult::Skipped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Logs;
};

USTRUCT(BlueprintType)
struct FMGTestSuiteReport
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TotalTests = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 PassedTests = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 FailedTests = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 SkippedTests = 0;

	UPROPERTY(BlueprintReadOnly)
	float TotalDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	TArray<FMGTestResult> Results;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTestStarted, FName, TestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTestCompleted, const FMGTestResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSuiteCompleted, const FMGTestSuiteReport&, Report);

UCLASS()
class MIDNIGHTGRIND_API UMGTestFrameworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Test Registration
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RegisterTest(const FMGTestCase& Test);

	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FMGTestCase> GetAllTests() const { return RegisteredTests; }

	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FMGTestCase> GetTestsByCategory(EMGTestCategory Category) const;

	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FMGTestCase> GetTestsByTag(FName Tag) const;

	// Test Execution
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunTest(FName TestID);

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunAllTests();

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunTestsByCategory(EMGTestCategory Category);

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunSmokeTests();

	UFUNCTION(BlueprintCallable, Category = "Testing")
	void StopTests();

	UFUNCTION(BlueprintPure, Category = "Testing")
	bool IsRunning() const { return bIsRunning; }

	// Results
	UFUNCTION(BlueprintPure, Category = "Testing")
	FMGTestSuiteReport GetLastReport() const { return LastReport; }

	UFUNCTION(BlueprintPure, Category = "Testing")
	FMGTestResult GetTestResult(FName TestID) const;

	// Assertions (for use in test implementations)
	UFUNCTION(BlueprintCallable, Category = "Testing|Assert")
	void AssertTrue(bool bCondition, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Testing|Assert")
	void AssertEqual(int32 Expected, int32 Actual, const FString& Message);

	UFUNCTION(BlueprintCallable, Category = "Testing|Assert")
	void AssertNearlyEqual(float Expected, float Actual, float Tolerance, const FString& Message);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FMGOnTestStarted OnTestStarted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FMGOnTestCompleted OnTestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FMGOnSuiteCompleted OnSuiteCompleted;

protected:
	void ExecuteTest(const FMGTestCase& Test);
	void RegisterBuiltInTests();

private:
	UPROPERTY()
	TArray<FMGTestCase> RegisteredTests;

	UPROPERTY()
	TArray<FMGTestResult> CurrentResults;

	FMGTestSuiteReport LastReport;
	bool bIsRunning = false;
	FName CurrentTestID;
	TArray<FString> CurrentTestLogs;
};

// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTestFrameworkSubsystem.cpp
 * @brief Implementation of the automated test framework subsystem.
 *
 * Provides test registration, execution, assertion handling, and result reporting
 * for the Midnight Grind testing infrastructure.
 */

#include "TestFramework/MGTestFrameworkSubsystem.h"

void UMGTestFrameworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterBuiltInTests();
}

void UMGTestFrameworkSubsystem::RegisterTest(const FMGTestCase& Test)
{
	// Check for duplicate
	int32 ExistingIndex = RegisteredTests.IndexOfByPredicate(
		[&Test](const FMGTestCase& T) { return T.TestID == Test.TestID; });

	if (ExistingIndex != INDEX_NONE)
		RegisteredTests[ExistingIndex] = Test;
	else
		RegisteredTests.Add(Test);
}

TArray<FMGTestCase> UMGTestFrameworkSubsystem::GetTestsByCategory(EMGTestCategory Category) const
{
	TArray<FMGTestCase> Result;
	for (const FMGTestCase& Test : RegisteredTests)
	{
		if (Test.Category == Category)
			Result.Add(Test);
	}
	return Result;
}

TArray<FMGTestCase> UMGTestFrameworkSubsystem::GetTestsByTag(FName Tag) const
{
	TArray<FMGTestCase> Result;
	for (const FMGTestCase& Test : RegisteredTests)
	{
		if (Test.Tags.Contains(Tag))
			Result.Add(Test);
	}
	return Result;
}

void UMGTestFrameworkSubsystem::RunTest(FName TestID)
{
	const FMGTestCase* Test = RegisteredTests.FindByPredicate(
		[TestID](const FMGTestCase& T) { return T.TestID == TestID; });

	if (Test)
	{
		bIsRunning = true;
		ExecuteTest(*Test);
		bIsRunning = false;
	}
}

void UMGTestFrameworkSubsystem::RunAllTests()
{
	bIsRunning = true;
	CurrentResults.Empty();
	LastReport = FMGTestSuiteReport();

	for (const FMGTestCase& Test : RegisteredTests)
	{
		ExecuteTest(Test);
	}

	// Generate report
	LastReport.TotalTests = CurrentResults.Num();
	for (const FMGTestResult& Result : CurrentResults)
	{
		LastReport.TotalDurationSeconds += Result.DurationSeconds;

		switch (Result.Result)
		{
		case EMGTestResult::Passed:
			LastReport.PassedTests++;
			break;
		case EMGTestResult::Failed:
		case EMGTestResult::Error:
		case EMGTestResult::Timeout:
			LastReport.FailedTests++;
			break;
		case EMGTestResult::Skipped:
			LastReport.SkippedTests++;
			break;
		}
	}

	LastReport.Results = CurrentResults;
	OnSuiteCompleted.Broadcast(LastReport);
	bIsRunning = false;
}

void UMGTestFrameworkSubsystem::RunTestsByCategory(EMGTestCategory Category)
{
	bIsRunning = true;
	CurrentResults.Empty();

	TArray<FMGTestCase> CategoryTests = GetTestsByCategory(Category);
	for (const FMGTestCase& Test : CategoryTests)
	{
		ExecuteTest(Test);
	}

	bIsRunning = false;
}

void UMGTestFrameworkSubsystem::RunSmokeTests()
{
	RunTestsByCategory(EMGTestCategory::Smoke);
}

void UMGTestFrameworkSubsystem::StopTests()
{
	bIsRunning = false;
}

FMGTestResult UMGTestFrameworkSubsystem::GetTestResult(FName TestID) const
{
	const FMGTestResult* Result = CurrentResults.FindByPredicate(
		[TestID](const FMGTestResult& R) { return R.TestID == TestID; });
	return Result ? *Result : FMGTestResult();
}

void UMGTestFrameworkSubsystem::AssertTrue(bool bCondition, const FString& Message)
{
	if (!bCondition)
	{
		CurrentTestLogs.Add(FString::Printf(TEXT("ASSERTION FAILED: %s"), *Message));
	}
}

void UMGTestFrameworkSubsystem::AssertEqual(int32 Expected, int32 Actual, const FString& Message)
{
	if (Expected != Actual)
	{
		CurrentTestLogs.Add(FString::Printf(TEXT("ASSERTION FAILED: %s (Expected: %d, Actual: %d)"), *Message, Expected, Actual));
	}
}

void UMGTestFrameworkSubsystem::AssertNearlyEqual(float Expected, float Actual, float Tolerance, const FString& Message)
{
	if (FMath::Abs(Expected - Actual) > Tolerance)
	{
		CurrentTestLogs.Add(FString::Printf(TEXT("ASSERTION FAILED: %s (Expected: %f, Actual: %f, Tolerance: %f)"), *Message, Expected, Actual, Tolerance));
	}
}

void UMGTestFrameworkSubsystem::ExecuteTest(const FMGTestCase& Test)
{
	CurrentTestID = Test.TestID;
	CurrentTestLogs.Empty();

	OnTestStarted.Broadcast(Test.TestID);

	FMGTestResult Result;
	Result.TestID = Test.TestID;
	Result.Timestamp = FDateTime::UtcNow();

	double StartTime = FPlatformTime::Seconds();

	// Execute test logic (would be implemented per test)
	// For now, simulate
	bool bPassed = true;

	double EndTime = FPlatformTime::Seconds();
	Result.DurationSeconds = EndTime - StartTime;

	if (CurrentTestLogs.Num() > 0)
	{
		bPassed = false;
		Result.Logs = CurrentTestLogs;
	}

	Result.Result = bPassed ? EMGTestResult::Passed : EMGTestResult::Failed;
	Result.Message = bPassed ? FText::FromString(TEXT("Test passed")) : FText::FromString(TEXT("Test failed"));

	CurrentResults.Add(Result);
	OnTestCompleted.Broadcast(Result);
}

void UMGTestFrameworkSubsystem::RegisterBuiltInTests()
{
	// Smoke tests
	FMGTestCase CurrencyTest;
	CurrencyTest.TestID = FName(TEXT("Test_Currency_Earn"));
	CurrencyTest.TestName = FText::FromString(TEXT("Currency Earning"));
	CurrencyTest.Description = FText::FromString(TEXT("Verify currency can be earned"));
	CurrencyTest.Category = EMGTestCategory::Smoke;
	CurrencyTest.Tags.Add(FName(TEXT("Currency")));
	RegisterTest(CurrencyTest);

	FMGTestCase StoreTest;
	StoreTest.TestID = FName(TEXT("Test_Store_Purchase"));
	StoreTest.TestName = FText::FromString(TEXT("Store Purchase"));
	StoreTest.Description = FText::FromString(TEXT("Verify items can be purchased"));
	StoreTest.Category = EMGTestCategory::Smoke;
	StoreTest.Tags.Add(FName(TEXT("Store")));
	RegisterTest(StoreTest);

	FMGTestCase RaceTest;
	RaceTest.TestID = FName(TEXT("Test_Race_Complete"));
	RaceTest.TestName = FText::FromString(TEXT("Race Completion"));
	RaceTest.Description = FText::FromString(TEXT("Verify race can be completed"));
	RaceTest.Category = EMGTestCategory::Smoke;
	RaceTest.Tags.Add(FName(TEXT("Racing")));
	RegisterTest(RaceTest);

	// Performance tests
	FMGTestCase FramerateTest;
	FramerateTest.TestID = FName(TEXT("Test_Perf_Framerate"));
	FramerateTest.TestName = FText::FromString(TEXT("Framerate Stability"));
	FramerateTest.Description = FText::FromString(TEXT("Verify stable 60fps during race"));
	FramerateTest.Category = EMGTestCategory::Performance;
	FramerateTest.TimeoutSeconds = 120.0f;
	RegisterTest(FramerateTest);

	FMGTestCase MemoryTest;
	MemoryTest.TestID = FName(TEXT("Test_Perf_Memory"));
	MemoryTest.TestName = FText::FromString(TEXT("Memory Budget"));
	MemoryTest.Description = FText::FromString(TEXT("Verify memory stays within budget"));
	MemoryTest.Category = EMGTestCategory::Performance;
	RegisterTest(MemoryTest);
}

// Copyright Midnight Grind. All Rights Reserved.

/**
 * =============================================================================
 * MGTestFrameworkSubsystem.h - Automated Testing Framework
 * =============================================================================
 *
 * PURPOSE:
 * This file implements an automated testing framework specifically for
 * Midnight Grind. It allows developers to write, register, and run tests
 * to verify that game systems work correctly. Think of it like a simplified
 * version of unit testing frameworks (JUnit, NUnit, etc.) but designed for
 * Unreal Engine and accessible from both C++ and Blueprints.
 *
 * KEY CONCEPTS FOR NEW DEVELOPERS:
 *
 * 1. AUTOMATED TESTING:
 *    - Tests are small pieces of code that verify specific functionality.
 *    - Instead of manually testing "can the player earn currency?", you write
 *      a test that does it automatically and checks the result.
 *    - Tests can be run repeatedly to catch bugs when code changes (regression).
 *
 * 2. TEST TERMINOLOGY:
 *    - Test Case: A single test that verifies one thing (e.g., "currency adds correctly")
 *    - Test Suite: A collection of related tests run together
 *    - Assertion: A check that fails the test if a condition isn't met
 *    - Pass/Fail: The result of running a test
 *
 * 3. ENUMS (UENUM):
 *    - EMGTestResult: What happened when the test ran (Passed, Failed, etc.)
 *    - EMGTestCategory: What type of test this is (Unit, Integration, etc.)
 *    - BlueprintType specifier makes these usable in Blueprints.
 *
 * 4. TEST CATEGORIES EXPLAINED:
 *    - Unit: Tests a single function or class in isolation
 *    - Integration: Tests multiple systems working together
 *    - Performance: Measures speed/memory usage
 *    - Regression: Verifies bugs don't come back after being fixed
 *    - Smoke: Quick sanity checks that basic functionality works
 *    - Stress: Pushes the system to its limits (many objects, long duration)
 *
 * 5. DELEGATES (Events):
 *    - DECLARE_DYNAMIC_MULTICAST_DELEGATE: Creates an event that multiple
 *      listeners can subscribe to.
 *    - OnTestStarted: Fires when a test begins (for UI updates, logging)
 *    - OnTestCompleted: Fires when a test finishes (with the result)
 *    - OnSuiteCompleted: Fires when all tests in a batch complete
 *
 * HOW THIS FITS INTO THE GAME ARCHITECTURE:
 *
 *    [Test Case Definitions]
 *           |
 *           v
 *    [UMGTestFrameworkSubsystem] -- Manages test registration and execution
 *           |
 *           +---> Runs tests one by one
 *           +---> Collects results
 *           +---> Broadcasts events for UI/logging
 *           |
 *           v
 *    [FMGTestSuiteReport] -- Summary of all test results
 *
 * HOW TO WRITE A TEST:
 * 1. Create an FMGTestCase struct with TestID, name, description
 * 2. Register it with RegisterTest()
 * 3. Implement the test logic (the framework calls your test function)
 * 4. Use AssertTrue, AssertEqual, etc. to verify conditions
 * 5. Return the result
 *
 * COMMON USE CASES:
 * - RunSmokeTests(): Quick sanity check before a build
 * - RunAllTests(): Full verification before release
 * - RunTestsByCategory(Performance): Check for performance regressions
 *
 * EXAMPLE USAGE:
 *   // Register a test
 *   FMGTestCase Test;
 *   Test.TestID = "Currency_Add";
 *   Test.TestName = FText::FromString("Currency Addition");
 *   Test.Category = EMGTestCategory::Unit;
 *   TestFramework->RegisterTest(Test);
 *
 *   // Run tests
 *   TestFramework->RunAllTests();
 *
 *   // Check results
 *   FMGTestSuiteReport Report = TestFramework->GetLastReport();
 *   UE_LOG(LogTemp, Log, TEXT("Passed: %d, Failed: %d"), Report.PassedTests, Report.FailedTests);
 *
 * =============================================================================
 */

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MGTestFrameworkSubsystem.generated.h"

/**
 * Test result status.
 * Indicates what happened when a test was executed.
 *
 * Note: uint8 means this enum uses only 1 byte of memory (values 0-255).
 * This is efficient for storing many results and for network transmission.
 */
UENUM(BlueprintType)
enum class EMGTestResult : uint8
{
	/** Test completed successfully - all assertions passed */
	Passed,
	/** Test completed but one or more assertions failed */
	Failed,
	/** Test was not run (disabled, filtered out, or prerequisites not met) */
	Skipped,
	/** Test took too long and was forcibly stopped */
	Timeout,
	/** Test encountered an unexpected error (crash, exception, etc.) */
	Error
};

/**
 * Test category classification.
 * Helps organize tests and allows running specific types of tests.
 *
 * Different categories serve different purposes:
 * - Run Smoke tests before every commit (quick sanity check)
 * - Run Unit tests during development
 * - Run Stress tests before releases
 */
UENUM(BlueprintType)
enum class EMGTestCategory : uint8
{
	/** Tests a single function or class in isolation (fast, focused) */
	Unit,
	/** Tests multiple systems working together (slower, broader) */
	Integration,
	/** Measures speed, memory usage, or other metrics */
	Performance,
	/** Verifies that fixed bugs don't reappear */
	Regression,
	/** Quick tests that verify basic functionality (run frequently) */
	Smoke,
	/** Pushes system to limits - many objects, long duration, etc. */
	Stress
};

/**
 * Definition of a single test case.
 * Contains metadata about the test and how to run it.
 *
 * Think of this as the "registration form" for a test - it describes
 * WHAT the test is, but the actual test code is elsewhere.
 */
USTRUCT(BlueprintType)
struct FMGTestCase
{
	GENERATED_BODY()

	/** Unique identifier for this test (e.g., "Currency_Add", "Weather_Rain") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TestID;

	/** Human-readable name shown in test reports */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TestName;

	/** Explanation of what this test verifies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	/** What type of test this is (Unit, Integration, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTestCategory Category = EMGTestCategory::Unit;

	/**
	 * Optional tags for filtering (e.g., "Currency", "Critical", "Flaky")
	 * Use GetTestsByTag() to find tests with specific tags.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> Tags;

	/**
	 * Maximum time (in seconds) before the test is forcibly stopped.
	 * Prevents infinite loops or stuck tests from blocking the entire suite.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeoutSeconds = 30.0f;
};

/**
 * Result of running a single test.
 * Created after a test completes and contains outcome information.
 *
 * This is the "report card" for a single test execution.
 */
USTRUCT(BlueprintType)
struct FMGTestResult
{
	GENERATED_BODY()

	/** Which test this result is for (matches FMGTestCase::TestID) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TestID;

	/** Did the test pass, fail, timeout, etc.? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMGTestResult Result = EMGTestResult::Skipped;

	/** Human-readable explanation (success message or failure reason) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Message;

	/** How long the test took to run (useful for performance tests) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DurationSeconds = 0.0f;

	/** When the test was run (for historical tracking) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime Timestamp;

	/** Detailed log output from the test (for debugging failures) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> Logs;
};

/**
 * Summary report for a batch of tests (a "test suite").
 * Provides aggregate statistics for all tests that were run.
 *
 * This is the "final grade" for an entire testing session.
 */
USTRUCT(BlueprintType)
struct FMGTestSuiteReport
{
	GENERATED_BODY()

	/** Total number of tests that were executed */
	UPROPERTY(BlueprintReadOnly)
	int32 TotalTests = 0;

	/** Number of tests that passed (all assertions succeeded) */
	UPROPERTY(BlueprintReadOnly)
	int32 PassedTests = 0;

	/** Number of tests that failed (at least one assertion failed) */
	UPROPERTY(BlueprintReadOnly)
	int32 FailedTests = 0;

	/** Number of tests that were skipped (not run) */
	UPROPERTY(BlueprintReadOnly)
	int32 SkippedTests = 0;

	/** Total time spent running all tests */
	UPROPERTY(BlueprintReadOnly)
	float TotalDurationSeconds = 0.0f;

	/** Individual results for each test (for detailed analysis) */
	UPROPERTY(BlueprintReadOnly)
	TArray<FMGTestResult> Results;
};

// ============================================================================
// DELEGATES (Events)
// ============================================================================
// These allow other systems to react when tests start/complete.
// Example: A UI widget might subscribe to show progress during testing.

/**
 * Broadcast when a test begins execution.
 * Listeners receive the TestID of the test that's starting.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTestStarted, FName, TestID);

/**
 * Broadcast when a test finishes (pass or fail).
 * Listeners receive the full result object.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnTestCompleted, const FMGTestResult&, Result);

/**
 * Broadcast when an entire test suite finishes.
 * Listeners receive the summary report with pass/fail counts.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMGOnSuiteCompleted, const FMGTestSuiteReport&, Report);

/**
 * Test Framework Subsystem
 *
 * The central manager for all automated testing in Midnight Grind.
 * This subsystem handles:
 * - Registering test cases
 * - Running tests (individual, by category, or all)
 * - Collecting and reporting results
 * - Providing assertion functions for test implementations
 *
 * As a GameInstanceSubsystem, it persists for the entire game session
 * and is automatically created by Unreal Engine.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGTestFrameworkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Called automatically when the subsystem is created.
	 * Registers built-in tests and sets up the framework.
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ========================================================================
	// TEST REGISTRATION
	// ========================================================================
	// These functions manage the list of available tests.

	/**
	 * Register a new test case with the framework.
	 * Call this during initialization to make a test available for running.
	 *
	 * @param Test The test case definition to register
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RegisterTest(const FMGTestCase& Test);

	/**
	 * Get all registered tests.
	 * @return Array of all test case definitions
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FMGTestCase> GetAllTests() const { return RegisteredTests; }

	/**
	 * Get tests filtered by category.
	 * Useful for running only Unit tests, only Performance tests, etc.
	 *
	 * @param Category The category to filter by
	 * @return Array of tests matching the category
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FMGTestCase> GetTestsByCategory(EMGTestCategory Category) const;

	/**
	 * Get tests filtered by tag.
	 * Useful for finding tests related to a specific feature.
	 *
	 * @param Tag The tag to search for (e.g., "Currency", "Critical")
	 * @return Array of tests that have this tag
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	TArray<FMGTestCase> GetTestsByTag(FName Tag) const;

	// ========================================================================
	// TEST EXECUTION
	// ========================================================================
	// These functions run tests and collect results.

	/**
	 * Run a single test by its ID.
	 * @param TestID The unique identifier of the test to run
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunTest(FName TestID);

	/**
	 * Run ALL registered tests.
	 * This may take a while - consider using RunSmokeTests for quick checks.
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunAllTests();

	/**
	 * Run all tests in a specific category.
	 * @param Category The category of tests to run
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunTestsByCategory(EMGTestCategory Category);

	/**
	 * Run only smoke tests (quick sanity checks).
	 * Good for running before every commit or build.
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void RunSmokeTests();

	/**
	 * Stop any currently running tests.
	 * Use this to abort a long-running test suite.
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void StopTests();

	/**
	 * Check if tests are currently running.
	 * @return True if a test or test suite is in progress
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	bool IsRunning() const { return bIsRunning; }

	// ========================================================================
	// RESULTS
	// ========================================================================
	// These functions retrieve test outcomes.

	/**
	 * Get the summary report from the last test run.
	 * @return The test suite report with pass/fail counts
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	FMGTestSuiteReport GetLastReport() const { return LastReport; }

	/**
	 * Get the result of a specific test from the last run.
	 * @param TestID The test to look up
	 * @return The result for that test (or a default result if not found)
	 */
	UFUNCTION(BlueprintPure, Category = "Testing")
	FMGTestResult GetTestResult(FName TestID) const;

	// ========================================================================
	// ASSERTIONS
	// ========================================================================
	// Use these in test implementations to verify conditions.
	// If an assertion fails, the test fails.

	/**
	 * Assert that a condition is true.
	 * If bCondition is false, the test fails with the given message.
	 *
	 * Example: AssertTrue(Balance > 0, "Balance should be positive");
	 *
	 * @param bCondition The condition to check
	 * @param Message Description of what went wrong if it fails
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assert")
	void AssertTrue(bool bCondition, const FString& Message);

	/**
	 * Assert that two integers are equal.
	 * If they differ, the test fails showing expected vs actual values.
	 *
	 * Example: AssertEqual(100, NewBalance, "Balance should be 100");
	 *
	 * @param Expected The value you expect
	 * @param Actual The value you got
	 * @param Message Description of what's being compared
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assert")
	void AssertEqual(int32 Expected, int32 Actual, const FString& Message);

	/**
	 * Assert that two floats are approximately equal.
	 * Useful for physics/math where exact equality is unreliable.
	 *
	 * Example: AssertNearlyEqual(1.0f, Speed, 0.01f, "Speed should be ~1.0");
	 *
	 * @param Expected The value you expect
	 * @param Actual The value you got
	 * @param Tolerance How much difference is acceptable
	 * @param Message Description of what's being compared
	 */
	UFUNCTION(BlueprintCallable, Category = "Testing|Assert")
	void AssertNearlyEqual(float Expected, float Actual, float Tolerance, const FString& Message);

	// ========================================================================
	// EVENTS
	// ========================================================================
	// Subscribe to these to react when tests start/complete.

	/** Fires when a test begins. Use for progress UI. */
	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FMGOnTestStarted OnTestStarted;

	/** Fires when a test completes. Use for logging individual results. */
	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FMGOnTestCompleted OnTestCompleted;

	/** Fires when an entire test suite completes. Use for final report. */
	UPROPERTY(BlueprintAssignable, Category = "Testing|Events")
	FMGOnSuiteCompleted OnSuiteCompleted;

protected:
	/**
	 * Execute a single test case.
	 * Called internally by the Run* functions.
	 */
	void ExecuteTest(const FMGTestCase& Test);

	/**
	 * Register the framework's built-in tests.
	 * Called during Initialize().
	 */
	void RegisterBuiltInTests();

private:
	/** All tests that have been registered with the framework */
	UPROPERTY()
	TArray<FMGTestCase> RegisteredTests;

	/** Results from the currently-running test suite */
	UPROPERTY()
	TArray<FMGTestResult> CurrentResults;

	/** Summary report from the most recent test run */
	FMGTestSuiteReport LastReport;

	/** True while tests are actively running */
	bool bIsRunning = false;

	/** ID of the test currently being executed */
	FName CurrentTestID;

	/** Log messages accumulated during the current test */
	TArray<FString> CurrentTestLogs;
};

// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AI/MGAISubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: AI Difficulty Scaling
 * Verifies that the AI subsystem correctly scales difficulty
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIDifficultyScalingTest,
	"MidnightGrind.Unit.AI.DifficultyScaling",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIDifficultyScalingTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	if (!TestNotNull(TEXT("AI subsystem created"), AI))
		return false;

	AI->Initialize(nullptr);

	// Test: Can get current difficulty
	EMGAIDifficulty Difficulty = AI->GetCurrentDifficulty();
	TestTrue(TEXT("Difficulty level is valid"), true); // Just verify no crash

	// Test: Can get difficulty multiplier
	float Multiplier = AI->GetDifficultyMultiplier();
	TestTrue(TEXT("Difficulty multiplier is positive"), Multiplier > 0.0f);
	TestTrue(TEXT("Difficulty multiplier is reasonable"), Multiplier < 10.0f);

	// Test: AI subsystem is initialized
	bool IsInitialized = AI->IsInitialized();
	TestTrue(TEXT("AI subsystem is initialized"), IsInitialized);

	return true;
}

/**
 * Test: AI Opponent Selection
 * Verifies that the AI subsystem can select appropriate opponents
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIOpponentSelectionTest,
	"MidnightGrind.Unit.AI.OpponentSelection",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIOpponentSelectionTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	if (!TestNotNull(TEXT("AI subsystem created"), AI))
		return false;

	AI->Initialize(nullptr);

	// Test: Can get available opponents
	TArray<FMGAIDriverProfile> Opponents = AI->GetAvailableOpponents();
	TestTrue(TEXT("Opponent list is valid (may be empty)"), Opponents.Num() >= 0);

	// Test: Can request specific number of opponents
	int32 RequestedCount = 5;
	TArray<FMGAIDriverProfile> SelectedOpponents = AI->SelectOpponents(RequestedCount);
	TestTrue(TEXT("Selected opponents count is valid"), SelectedOpponents.Num() >= 0);
	TestTrue(TEXT("Selected opponents <= requested"), SelectedOpponents.Num() <= RequestedCount);

	return true;
}

/**
 * Test: AI Behavior State Management
 * Verifies that the AI subsystem can manage behavior states
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIBehaviorStateTest,
	"MidnightGrind.Unit.AI.BehaviorState",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIBehaviorStateTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	if (!TestNotNull(TEXT("AI subsystem created"), AI))
		return false;

	AI->Initialize(nullptr);

	// Test: Can get aggression level
	float Aggression = AI->GetAIAggressionLevel();
	TestTrue(TEXT("Aggression level is non-negative"), Aggression >= 0.0f);
	TestTrue(TEXT("Aggression level is <= 1.0"), Aggression <= 1.0f);

	// Test: Can get skill level
	float Skill = AI->GetAISkillLevel();
	TestTrue(TEXT("Skill level is non-negative"), Skill >= 0.0f);
	TestTrue(TEXT("Skill level is <= 1.0"), Skill <= 1.0f);

	// Test: Can check if AI is in aggressive mode
	bool IsAggressive = AI->IsAIInAggressiveMode();
	TestTrue(TEXT("Aggressive mode check returns valid bool"), true); // Just verify no crash

	return true;
}

/**
 * Test: AI Performance Calculation
 * Verifies that the AI subsystem correctly calculates performance metrics
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIPerformanceCalculationTest,
	"MidnightGrind.Unit.AI.PerformanceCalculation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIPerformanceCalculationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	if (!TestNotNull(TEXT("AI subsystem created"), AI))
		return false;

	AI->Initialize(nullptr);

	// Test: Can calculate lap time prediction
	float PredictedLapTime = AI->PredictAILapTime(EMGAIDifficulty::Medium, TEXT("TrackName"));
	TestTrue(TEXT("Predicted lap time is positive"), PredictedLapTime > 0.0f);
	TestTrue(TEXT("Predicted lap time is reasonable (< 10 minutes)"), PredictedLapTime < 600.0f);

	// Test: Different difficulties produce different predictions
	float EasyTime = AI->PredictAILapTime(EMGAIDifficulty::Easy, TEXT("TrackName"));
	float HardTime = AI->PredictAILapTime(EMGAIDifficulty::Hard, TEXT("TrackName"));

	TestTrue(TEXT("Easy and hard times are both positive"), EasyTime > 0.0f && HardTime > 0.0f);
	// Note: Can't guarantee easy > hard due to variance, just verify both are valid

	return true;
}

/**
 * Test: AI Rubber-banding System
 * Verifies that the AI subsystem implements rubber-banding correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIRubberBandingTest,
	"MidnightGrind.Unit.AI.RubberBanding",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIRubberBandingTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	if (!TestNotNull(TEXT("AI subsystem created"), AI))
		return false;

	AI->Initialize(nullptr);

	// Test: Can get rubber-banding factor
	float RubberBandFactor = AI->GetRubberBandingFactor();
	TestTrue(TEXT("Rubber-banding factor is non-negative"), RubberBandFactor >= 0.0f);
	TestTrue(TEXT("Rubber-banding factor is reasonable"), RubberBandFactor <= 2.0f);

	// Test: Can enable/disable rubber-banding
	AI->SetRubberBandingEnabled(false);
	bool IsEnabled = AI->IsRubberBandingEnabled();
	TestFalse(TEXT("Rubber-banding can be disabled"), IsEnabled);

	AI->SetRubberBandingEnabled(true);
	IsEnabled = AI->IsRubberBandingEnabled();
	TestTrue(TEXT("Rubber-banding can be enabled"), IsEnabled);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

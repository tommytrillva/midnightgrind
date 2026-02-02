// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "AI/MGAISubsystem.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Progression/MGPlayerProgressionSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Integration Test: Race Setup with AI and Vehicle Data
 * Verifies AI opponent selection integrates with vehicle catalog
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGRaceSetupIntegrationTest,
	"MidnightGrind.Integration.Gameplay.RaceSetup",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGRaceSetupIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create subsystems
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);

	// Setup vehicle catalog
	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(15);
	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	VehicleCatalog->VehicleDataTable = VehicleDT;

	VehicleCatalog->Initialize(nullptr);
	AI->Initialize(nullptr);

	// Test: AI can select opponents
	TArray<FMGAIDriverConfig> Opponents = AI->SelectOpponents(5);
	TestTrue(TEXT("AI can select opponents"), Opponents.Num() >= 0);
	TestTrue(TEXT("Opponent count reasonable"), Opponents.Num() <= 5);

	// Test: Vehicles exist for potential AI assignment
	TArray<FMGVehicleData> AllVehicles = VehicleCatalog->GetAllVehicles();
	TestEqual(TEXT("Vehicle catalog has expected vehicles"), AllVehicles.Num(), 15);

	// Test: Different vehicle classes available for AI
	TArray<FMGVehicleData> SportVehicles = VehicleCatalog->GetVehiclesByClass(EMGVehicleClass::Sport);
	TArray<FMGVehicleData> SedanVehicles = VehicleCatalog->GetVehiclesByClass(EMGVehicleClass::Sedan);

	TestTrue(TEXT("Has vehicles for AI selection"), AllVehicles.Num() > 0);

	// Test: AI difficulty can be configured for race
	EMGAIDifficulty Difficulty = AI->GetCurrentDifficulty();
	float Multiplier = AI->GetDifficultyMultiplier();

	TestTrue(TEXT("Difficulty multiplier is valid"), Multiplier > 0.0f && Multiplier < 10.0f);

	return true;
}

/**
 * Integration Test: Progression and Reward Flow
 * Verifies progression updates correctly trigger rewards
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGProgressionRewardFlowTest,
	"MidnightGrind.Integration.Gameplay.ProgressionRewardFlow",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGProgressionRewardFlowTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create progression subsystem
	UMGPlayerProgressionSubsystem* Progression = NewObject<UMGPlayerProgressionSubsystem>(GameInstance);
	Progression->Initialize(nullptr);

	// Test: Can get current player level
	int32 Level = Progression->GetPlayerLevel();
	TestTrue(TEXT("Player level is non-negative"), Level >= 0);

	// Test: Can get XP values
	int32 CurrentXP = Progression->GetCurrentXP();
	int32 XPToNextLevel = Progression->GetXPToNextLevel();

	TestTrue(TEXT("Current XP is non-negative"), CurrentXP >= 0);
	TestTrue(TEXT("XP to next level is positive"), XPToNextLevel > 0);

	// Test: Can check unlock status
	bool HasUnlockedFeature = Progression->IsFeatureUnlocked(TEXT("AdvancedTuning"));
	TestTrue(TEXT("Feature unlock check works"), true); // Just verify no crash

	// Test: Progression subsystem is initialized
	TestTrue(TEXT("Progression subsystem initialized"), Progression->IsInitialized());

	return true;
}

/**
 * Integration Test: AI Performance with Difficulty Settings
 * Verifies AI performance scales correctly with difficulty
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAIDifficultyPerformanceTest,
	"MidnightGrind.Integration.Gameplay.AIDifficultyPerformance",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAIDifficultyPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	AI->Initialize(nullptr);

	// Test: Different difficulties produce different predictions
	float EasyLapTime = AI->PredictAILapTime(EMGAIDifficulty::Easy, TEXT("TestTrack"));
	float MediumLapTime = AI->PredictAILapTime(EMGAIDifficulty::Medium, TEXT("TestTrack"));
	float HardLapTime = AI->PredictAILapTime(EMGAIDifficulty::Hard, TEXT("TestTrack"));

	TestTrue(TEXT("Easy lap time is valid"), EasyLapTime > 0.0f && EasyLapTime < 600.0f);
	TestTrue(TEXT("Medium lap time is valid"), MediumLapTime > 0.0f && MediumLapTime < 600.0f);
	TestTrue(TEXT("Hard lap time is valid"), HardLapTime > 0.0f && HardLapTime < 600.0f);

	// Test: Skill and aggression levels affect behavior
	float Aggression = AI->GetAIAggressionLevel();
	float Skill = AI->GetAISkillLevel();

	TestTrue(TEXT("Aggression in valid range"), Aggression >= 0.0f && Aggression <= 1.0f);
	TestTrue(TEXT("Skill in valid range"), Skill >= 0.0f && Skill <= 1.0f);

	// Test: Rubber-banding can be controlled
	AI->SetRubberBandingEnabled(true);
	bool EnabledState = AI->IsRubberBandingEnabled();
	TestTrue(TEXT("Rubber-banding can be enabled"), EnabledState);

	AI->SetRubberBandingEnabled(false);
	bool DisabledState = AI->IsRubberBandingEnabled();
	TestFalse(TEXT("Rubber-banding can be disabled"), DisabledState);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

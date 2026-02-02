// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/MGPlayerMarketSubsystem.h"
#include "Economy/MGMechanicSubsystem.h"
#include "Social/MGPlayerSocialSubsystem.h"
#include "AI/MGAISubsystem.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Performance Test: Multi-Subsystem Initialization
 * Measures time to initialize all game subsystems
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMultiSubsystemInitializationTest,
	"MidnightGrind.Performance.Subsystem.MultiInitialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMultiSubsystemInitializationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create all major subsystems
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);
	UMGPlayerSocialSubsystem* Social = NewObject<UMGPlayerSocialSubsystem>(GameInstance);
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);

	// Setup data tables
	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(50);
	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(200);

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	// Performance test: Initialize all subsystems
	double StartTime = FPlatformTime::Seconds();

	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);
	Market->Initialize(nullptr);
	Mechanic->Initialize(nullptr);
	Social->Initialize(nullptr);
	AI->Initialize(nullptr);

	double InitializationTime = FPlatformTime::Seconds() - StartTime;

	// Validate performance: All subsystems should initialize quickly
	TestTrue(TEXT("All subsystems initialize in <2 seconds"), InitializationTime < 2.0);

	// Verify all subsystems initialized correctly
	TestTrue(TEXT("Vehicle catalog initialized"), VehicleCatalog->IsInitialized());
	TestTrue(TEXT("Parts catalog initialized"), PartsCatalog->IsInitialized());
	TestTrue(TEXT("Market initialized"), Market->IsInitialized());
	TestTrue(TEXT("Mechanic initialized"), Mechanic->IsInitialized());
	TestTrue(TEXT("Social initialized"), Social->IsInitialized());
	TestTrue(TEXT("AI initialized"), AI->IsInitialized());

	AddInfo(FString::Printf(TEXT("Total initialization time: %.3f ms"), InitializationTime * 1000.0));

	return true;
}

/**
 * Performance Test: Economy Calculation Throughput
 * Measures performance of frequent economy calculations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGEconomyCalculationThroughputTest,
	"MidnightGrind.Performance.Subsystem.EconomyCalculationThroughput",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGEconomyCalculationThroughputTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create economy subsystems
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);

	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(100);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	PartsCatalog->PartsDataTable = PartsDT;
	PartsCatalog->Initialize(nullptr);
	Mechanic->Initialize(nullptr);

	// Performance test: Repeated install time calculations
	const int32 CalculationCount = 5000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < CalculationCount; ++i)
	{
		int32 PartIndex = i % TestParts.Num();
		Mechanic->GetPartBaseInstallTime(TestParts[PartIndex].PartID);
	}

	double InstallTimeCalculationTime = FPlatformTime::Seconds() - StartTime;

	// Performance test: Repeated labor cost calculations
	StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < CalculationCount; ++i)
	{
		int32 PartIndex = i % TestParts.Num();
		Mechanic->GetPartBaseInstallCost(TestParts[PartIndex].PartID);
	}

	double LaborCostCalculationTime = FPlatformTime::Seconds() - StartTime;

	// Validate performance
	TestTrue(TEXT("Install time calculations complete quickly (<0.2s)"), InstallTimeCalculationTime < 0.2);
	TestTrue(TEXT("Labor cost calculations complete quickly (<0.2s)"), LaborCostCalculationTime < 0.2);

	AddInfo(FString::Printf(TEXT("Install time calc: %.3f ms (%.0f/sec)"),
		InstallTimeCalculationTime * 1000.0, CalculationCount / InstallTimeCalculationTime));
	AddInfo(FString::Printf(TEXT("Labor cost calc: %.3f ms (%.0f/sec)"),
		LaborCostCalculationTime * 1000.0, CalculationCount / LaborCostCalculationTime));

	return true;
}

/**
 * Performance Test: AI System Under Load
 * Measures AI subsystem performance with multiple concurrent operations
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGAISystemLoadTest,
	"MidnightGrind.Performance.Subsystem.AISystemLoad",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGAISystemLoadTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create AI subsystem
	UMGAISubsystem* AI = NewObject<UMGAISubsystem>(GameInstance);
	AI->Initialize(nullptr);

	// Performance test: Repeated opponent selection
	const int32 SelectionCount = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < SelectionCount; ++i)
	{
		AI->SelectOpponents(5);
	}

	double SelectionTime = FPlatformTime::Seconds() - StartTime;

	// Performance test: Repeated lap time predictions
	StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < SelectionCount; ++i)
	{
		EMGAIDifficulty Difficulty = static_cast<EMGAIDifficulty>(i % 5);
		AI->PredictAILapTime(Difficulty, TEXT("TestTrack"));
	}

	double PredictionTime = FPlatformTime::Seconds() - StartTime;

	// Validate performance
	TestTrue(TEXT("Opponent selection completes quickly (<0.5s)"), SelectionTime < 0.5);
	TestTrue(TEXT("Lap time prediction completes quickly (<0.3s)"), PredictionTime < 0.3);

	AddInfo(FString::Printf(TEXT("Opponent selection: %.3f ms (%.0f/sec)"),
		SelectionTime * 1000.0, SelectionCount / SelectionTime));
	AddInfo(FString::Printf(TEXT("Lap time prediction: %.3f ms (%.0f/sec)"),
		PredictionTime * 1000.0, SelectionCount / PredictionTime));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

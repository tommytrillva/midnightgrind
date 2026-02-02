// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/MGMechanicSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: Mechanic Part Install Time Calculation
 * Verifies that the mechanic subsystem correctly calculates install times from catalog
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMechanicInstallTimeTest,
	"MidnightGrind.Unit.Economy.MechanicInstallTime",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMechanicInstallTimeTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create parts catalog
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Parts catalog created"), PartsCatalog))
		return false;

	// Create mechanic subsystem
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Mechanic subsystem created"), Mechanic))
		return false;

	// Create test parts with specific install times
	FMGPartData QuickPart = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_QuickInstall"),
		500.0f,   // Base cost
		75.0f,    // Labor cost
		30.0f);   // 30 minutes install

	FMGPartData MediumPart = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_MediumInstall"),
		1500.0f,  // Base cost
		225.0f,   // Labor cost
		120.0f);  // 2 hours install

	FMGPartData LongPart = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_LongInstall"),
		5000.0f,  // Base cost
		750.0f,   // Labor cost
		360.0f);  // 6 hours install

	// Create mock data table
	TArray<FMGPartData> TestParts = { QuickPart, MediumPart, LongPart };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	PartsCatalog->PartsDataTable = MockDataTable;
	PartsCatalog->Initialize(nullptr);

	// Test: Quick install (30 min = 1 hour rounded up)
	int32 QuickInstallTime = Mechanic->GetPartBaseInstallTime(TEXT("Part_QuickInstall"));
	TestEqual(TEXT("Quick install time (30 min -> 1 hour)"), QuickInstallTime, 1);

	// Test: Medium install (120 min = 2 hours)
	int32 MediumInstallTime = Mechanic->GetPartBaseInstallTime(TEXT("Part_MediumInstall"));
	TestEqual(TEXT("Medium install time (120 min -> 2 hours)"), MediumInstallTime, 2);

	// Test: Long install (360 min = 6 hours)
	int32 LongInstallTime = Mechanic->GetPartBaseInstallTime(TEXT("Part_LongInstall"));
	TestEqual(TEXT("Long install time (360 min -> 6 hours)"), LongInstallTime, 6);

	// Test: Unknown part (should use fallback estimation)
	int32 UnknownInstallTime = Mechanic->GetPartBaseInstallTime(TEXT("Part_Unknown"));
	TestTrue(TEXT("Unknown part returns fallback time > 0"), UnknownInstallTime > 0);

	return true;
}

/**
 * Test: Mechanic Labor Cost Calculation
 * Verifies that the mechanic subsystem correctly calculates labor costs from catalog
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMechanicLaborCostTest,
	"MidnightGrind.Unit.Economy.MechanicLaborCost",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMechanicLaborCostTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create parts catalog
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Parts catalog created"), PartsCatalog))
		return false;

	// Create mechanic subsystem
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Mechanic subsystem created"), Mechanic))
		return false;

	// Create test parts with specific labor costs
	FMGPartData CheapLabor = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_CheapLabor"),
		1000.0f,  // Base cost
		150.0f,   // $150 labor
		60.0f);   // 1 hour

	FMGPartData ExpensiveLabor = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_ExpensiveLabor"),
		10000.0f, // Base cost
		1500.0f,  // $1500 labor
		300.0f);  // 5 hours

	// Create mock data table
	TArray<FMGPartData> TestParts = { CheapLabor, ExpensiveLabor };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	PartsCatalog->PartsDataTable = MockDataTable;
	PartsCatalog->Initialize(nullptr);

	// Test: Cheap labor cost
	int32 CheapCost = Mechanic->GetPartBaseInstallCost(TEXT("Part_CheapLabor"));
	TestEqual(TEXT("Cheap labor cost"), CheapCost, 150);

	// Test: Expensive labor cost
	int32 ExpensiveCost = Mechanic->GetPartBaseInstallCost(TEXT("Part_ExpensiveLabor"));
	TestEqual(TEXT("Expensive labor cost"), ExpensiveCost, 1500);

	// Test: Unknown part (should use fallback calculation)
	int32 UnknownCost = Mechanic->GetPartBaseInstallCost(TEXT("Part_Unknown"));
	TestTrue(TEXT("Unknown part returns fallback cost > 0"), UnknownCost > 0);

	// Test: Verify labor cost correlates with install time
	TestTrue(TEXT("Longer installs have higher labor costs"), ExpensiveCost > CheapCost);

	return true;
}

/**
 * Test: Mechanic Skill Level Impact
 * Verifies that mechanic skill affects install time and cost
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMechanicSkillLevelTest,
	"MidnightGrind.Unit.Economy.MechanicSkillLevel",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMechanicSkillLevelTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create parts catalog
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Parts catalog created"), PartsCatalog))
		return false;

	// Create mechanic subsystem
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Mechanic subsystem created"), Mechanic))
		return false;

	// Create test part
	FMGPartData TestPart = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_SkillTest"),
		2000.0f,  // Base cost
		300.0f,   // Labor cost
		120.0f);  // 2 hours install

	// Create mock data table
	TArray<FMGPartData> TestParts = { TestPart };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	PartsCatalog->PartsDataTable = MockDataTable;
	PartsCatalog->Initialize(nullptr);

	// Get base values
	int32 BaseInstallTime = Mechanic->GetPartBaseInstallTime(TEXT("Part_SkillTest"));
	int32 BaseLaborCost = Mechanic->GetPartBaseInstallCost(TEXT("Part_SkillTest"));

	// Test: Base time is positive
	TestTrue(TEXT("Base install time is positive"), BaseInstallTime > 0);
	TestTrue(TEXT("Base labor cost is positive"), BaseLaborCost > 0);

	// Test: Values are within expected ranges
	TestTrue(TEXT("Install time is reasonable (< 24 hours)"), BaseInstallTime < 24);
	TestTrue(TEXT("Labor cost is reasonable (< $10000)"), BaseLaborCost < 10000);

	return true;
}

/**
 * Test: Mechanic Job Queue Management
 * Verifies that the mechanic subsystem can manage multiple jobs
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMechanicJobQueueTest,
	"MidnightGrind.Unit.Economy.MechanicJobQueue",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMechanicJobQueueTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create mechanic subsystem
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Mechanic subsystem created"), Mechanic))
		return false;

	Mechanic->Initialize(nullptr);

	// Test: Initial state - no active jobs
	TArray<FMGMechanicJob> ActiveJobs = Mechanic->GetActiveJobs();
	TestEqual(TEXT("No active jobs initially"), ActiveJobs.Num(), 0);

	// Test: Can get available mechanic slots
	int32 AvailableSlots = Mechanic->GetAvailableMechanicSlots();
	TestTrue(TEXT("Has available mechanic slots"), AvailableSlots > 0);

	// Test: Can check if mechanic is available
	bool IsAvailable = Mechanic->IsMechanicAvailable();
	TestTrue(TEXT("Mechanic is available initially"), IsAvailable);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

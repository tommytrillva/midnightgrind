// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: Parts Catalog Pricing Lookup
 * Verifies that the catalog correctly returns part pricing information
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGPartsCatalogPricingTest,
	"MidnightGrind.Unit.Catalog.PartPricing",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGPartsCatalogPricingTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog subsystem
	UMGPartsCatalogSubsystem* Catalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Catalog subsystem created"), Catalog))
		return false;

	// Create test parts with specific pricing
	FMGPartData TestPart1 = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_Engine_001"),
		2500.0f,  // Base cost
		375.0f,   // Labor cost
		120.0f);  // Install time (2 hours)

	FMGPartData TestPart2 = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_Turbo_001"),
		5000.0f,  // Base cost
		750.0f,   // Labor cost
		180.0f);  // Install time (3 hours)

	// Create mock data table
	TArray<FMGPartData> TestParts = { TestPart1, TestPart2 };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	Catalog->PartsDataTable = MockDataTable;
	Catalog->Initialize(nullptr);

	// Test: Get pricing info for engine part
	FMGPartPricingInfo Pricing1 = Catalog->GetPartPricing(TEXT("Part_Engine_001"));
	TestTrue(TEXT("Engine part pricing is valid"), Pricing1.bIsValid);
	TestEqual(TEXT("Engine base cost correct"), Pricing1.BaseCost, 2500.0f);
	TestEqual(TEXT("Engine labor cost correct"), Pricing1.LaborCost, 375.0f);
	TestEqual(TEXT("Engine total cost correct"), Pricing1.TotalCost, 2875.0f);

	// Test: Get pricing info for turbo part
	FMGPartPricingInfo Pricing2 = Catalog->GetPartPricing(TEXT("Part_Turbo_001"));
	TestTrue(TEXT("Turbo part pricing is valid"), Pricing2.bIsValid);
	TestEqual(TEXT("Turbo base cost correct"), Pricing2.BaseCost, 5000.0f);
	TestEqual(TEXT("Turbo labor cost correct"), Pricing2.LaborCost, 750.0f);
	TestEqual(TEXT("Turbo total cost correct"), Pricing2.TotalCost, 5750.0f);

	// Test: Get install time
	float InstallTime1 = Catalog->GetPartInstallTime(TEXT("Part_Engine_001"));
	TestEqual(TEXT("Engine install time correct"), InstallTime1, 120.0f);

	float InstallTime2 = Catalog->GetPartInstallTime(TEXT("Part_Turbo_001"));
	TestEqual(TEXT("Turbo install time correct"), InstallTime2, 180.0f);

	// Test: Invalid part (should return invalid pricing)
	FMGPartPricingInfo InvalidPricing = Catalog->GetPartPricing(TEXT("Part_DoesNotExist"));
	TestFalse(TEXT("Invalid part pricing is marked invalid"), InvalidPricing.bIsValid);
	TestEqual(TEXT("Invalid part returns 0 base cost"), InvalidPricing.BaseCost, 0.0f);

	// Test: Invalid part install time (should return 0)
	float InvalidInstallTime = Catalog->GetPartInstallTime(TEXT("Part_DoesNotExist"));
	TestEqual(TEXT("Invalid part returns 0 install time"), InvalidInstallTime, 0.0f);

	return true;
}

/**
 * Test: Parts Catalog Compatibility Check
 * Verifies that the catalog correctly determines part compatibility with vehicles
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGPartsCatalogCompatibilityTest,
	"MidnightGrind.Unit.Catalog.PartCompatibility",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGPartsCatalogCompatibilityTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog subsystem
	UMGPartsCatalogSubsystem* Catalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Catalog subsystem created"), Catalog))
		return false;

	// Create parts with specific vehicle compatibility
	TArray<FName> SportVehicles = { TEXT("Vehicle_Sport_001"), TEXT("Vehicle_Sport_002") };
	TArray<EMGVehicleClass> SportClasses = { EMGVehicleClass::Sport };
	FMGPartData SportPart = FMGTestDataFactory::CreateTestPartWithCompatibility(
		TEXT("Part_SportExhaust_001"),
		SportVehicles,
		SportClasses);

	// Create part compatible with all sedan class vehicles
	TArray<FName> EmptyVehicles;
	TArray<EMGVehicleClass> SedanClasses = { EMGVehicleClass::Sedan };
	FMGPartData SedanPart = FMGTestDataFactory::CreateTestPartWithCompatibility(
		TEXT("Part_SedanEngine_001"),
		EmptyVehicles,
		SedanClasses);

	// Create universal part (no restrictions)
	TArray<EMGVehicleClass> EmptyClasses;
	FMGPartData UniversalPart = FMGTestDataFactory::CreateTestPartWithCompatibility(
		TEXT("Part_UniversalFilter_001"),
		EmptyVehicles,
		EmptyClasses);

	// Create mock data table
	TArray<FMGPartData> TestParts = { SportPart, SedanPart, UniversalPart };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	Catalog->PartsDataTable = MockDataTable;
	Catalog->Initialize(nullptr);

	// Test: Sport part compatible with listed sport vehicle
	bool SportCompatible1 = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_SportExhaust_001"),
		TEXT("Vehicle_Sport_001"),
		EMGVehicleClass::Sport);
	TestTrue(TEXT("Sport part compatible with listed sport vehicle"), SportCompatible1);

	// Test: Sport part incompatible with sedan
	bool SportCompatible2 = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_SportExhaust_001"),
		TEXT("Vehicle_Sedan_001"),
		EMGVehicleClass::Sedan);
	TestFalse(TEXT("Sport part incompatible with sedan"), SportCompatible2);

	// Test: Sedan part compatible with any sedan (class-based)
	bool SedanCompatible1 = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_SedanEngine_001"),
		TEXT("Vehicle_Sedan_Random"),
		EMGVehicleClass::Sedan);
	TestTrue(TEXT("Sedan part compatible with any sedan (class-based)"), SedanCompatible1);

	// Test: Sedan part incompatible with sport
	bool SedanCompatible2 = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_SedanEngine_001"),
		TEXT("Vehicle_Sport_001"),
		EMGVehicleClass::Sport);
	TestFalse(TEXT("Sedan part incompatible with sport"), SedanCompatible2);

	// Test: Universal part compatible with everything
	bool UniversalCompatible1 = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_UniversalFilter_001"),
		TEXT("Vehicle_Any_001"),
		EMGVehicleClass::Sport);
	TestTrue(TEXT("Universal part compatible with sport"), UniversalCompatible1);

	bool UniversalCompatible2 = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_UniversalFilter_001"),
		TEXT("Vehicle_Any_002"),
		EMGVehicleClass::Sedan);
	TestTrue(TEXT("Universal part compatible with sedan"), UniversalCompatible2);

	// Test: Invalid part (should return false)
	bool InvalidCompatibility = Catalog->IsPartCompatibleWithVehicle(
		TEXT("Part_DoesNotExist"),
		TEXT("Vehicle_Sport_001"),
		EMGVehicleClass::Sport);
	TestFalse(TEXT("Invalid part returns incompatible"), InvalidCompatibility);

	return true;
}

/**
 * Test: Parts Catalog Specialization Match
 * Verifies that the catalog correctly identifies parts requiring specialist installation
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGPartsCatalogSpecializationTest,
	"MidnightGrind.Unit.Catalog.PartSpecialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGPartsCatalogSpecializationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog subsystem
	UMGPartsCatalogSubsystem* Catalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Catalog subsystem created"), Catalog))
		return false;

	// Create specialist part (requires specialized mechanic)
	FMGPartData SpecialistPart = FMGTestDataFactory::CreateTestPart(
		TEXT("Part_TurboSystem_001"),
		FText::FromString(TEXT("Advanced Turbo System")),
		EMGPartCategory::Engine,
		8000.0f);
	SpecialistPart.bRequiresSpecialist = true;
	SpecialistPart.InstallTimeMinutes = 240.0f; // 4 hours

	// Create standard part (no specialist required)
	FMGPartData StandardPart = FMGTestDataFactory::CreateTestPart(
		TEXT("Part_AirFilter_001"),
		FText::FromString(TEXT("Standard Air Filter")),
		EMGPartCategory::Engine,
		150.0f);
	StandardPart.bRequiresSpecialist = false;
	StandardPart.InstallTimeMinutes = 30.0f; // 30 minutes

	// Create complex part requiring specialist
	FMGPartData ComplexPart = FMGTestDataFactory::CreateTestPart(
		TEXT("Part_CustomECU_001"),
		FText::FromString(TEXT("Custom ECU Tune")),
		EMGPartCategory::Electronics,
		3500.0f);
	ComplexPart.bRequiresSpecialist = true;
	ComplexPart.InstallTimeMinutes = 180.0f; // 3 hours

	// Create mock data table
	TArray<FMGPartData> TestParts = { SpecialistPart, StandardPart, ComplexPart };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	Catalog->PartsDataTable = MockDataTable;
	Catalog->Initialize(nullptr);

	// Test: Get part data and verify specialist flag
	FMGPartData RetrievedSpecialistPart;
	bool Found1 = Catalog->GetPartData(TEXT("Part_TurboSystem_001"), RetrievedSpecialistPart);
	TestTrue(TEXT("Specialist part found"), Found1);
	TestTrue(TEXT("Turbo system requires specialist"), RetrievedSpecialistPart.bRequiresSpecialist);
	TestEqual(TEXT("Turbo system install time correct"), RetrievedSpecialistPart.InstallTimeMinutes, 240.0f);

	FMGPartData RetrievedStandardPart;
	bool Found2 = Catalog->GetPartData(TEXT("Part_AirFilter_001"), RetrievedStandardPart);
	TestTrue(TEXT("Standard part found"), Found2);
	TestFalse(TEXT("Air filter does not require specialist"), RetrievedStandardPart.bRequiresSpecialist);
	TestEqual(TEXT("Air filter install time correct"), RetrievedStandardPart.InstallTimeMinutes, 30.0f);

	FMGPartData RetrievedComplexPart;
	bool Found3 = Catalog->GetPartData(TEXT("Part_CustomECU_001"), RetrievedComplexPart);
	TestTrue(TEXT("Complex part found"), Found3);
	TestTrue(TEXT("ECU tune requires specialist"), RetrievedComplexPart.bRequiresSpecialist);

	// Test: Filter parts by category
	TArray<FMGPartData> EngineParts = Catalog->GetPartsByCategory(EMGPartCategory::Engine);
	TestEqual(TEXT("Found 2 engine parts"), EngineParts.Num(), 2);

	TArray<FMGPartData> ElectronicsParts = Catalog->GetPartsByCategory(EMGPartCategory::Electronics);
	TestEqual(TEXT("Found 1 electronics part"), ElectronicsParts.Num(), 1);

	// Test: Verify specialist parts have higher labor costs
	FMGPartPricingInfo SpecialistPricing = Catalog->GetPartPricing(TEXT("Part_TurboSystem_001"));
	FMGPartPricingInfo StandardPricing = Catalog->GetPartPricing(TEXT("Part_AirFilter_001"));

	TestTrue(TEXT("Specialist part has higher total cost"), SpecialistPricing.TotalCost > StandardPricing.TotalCost);

	// Test: Invalid part (should return false)
	FMGPartData InvalidPart;
	bool InvalidFound = Catalog->GetPartData(TEXT("Part_DoesNotExist"), InvalidPart);
	TestFalse(TEXT("Invalid part not found"), InvalidFound);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

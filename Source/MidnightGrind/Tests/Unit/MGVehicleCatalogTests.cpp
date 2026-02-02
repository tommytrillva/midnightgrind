// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: Vehicle Catalog Pricing Lookup
 * Verifies that the catalog correctly returns vehicle pricing information
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGVehicleCatalogPricingTest,
	"MidnightGrind.Unit.Catalog.VehiclePricing",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGVehicleCatalogPricingTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog subsystem
	UMGVehicleCatalogSubsystem* Catalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Catalog subsystem created"), Catalog))
		return false;

	// Create test vehicle data
	FMGVehicleData TestVehicle1 = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_PricingTest_001"),
		FText::FromString(TEXT("Test Sports Car")),
		35000.0f,
		EMGVehicleClass::Sport);

	FMGVehicleData TestVehicle2 = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_PricingTest_002"),
		FText::FromString(TEXT("Test Sedan")),
		22000.0f,
		EMGVehicleClass::Sedan);

	// Create mock data table
	TArray<FMGVehicleData> TestVehicles = { TestVehicle1, TestVehicle2 };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);

	// Manually set the data table (simulating what would happen in Initialize)
	// Note: This accesses private members for testing - in production, Initialize() would be called
	Catalog->VehicleDataTable = MockDataTable;
	Catalog->Initialize(nullptr);

	// Test: Get base price for known vehicle
	float Price1 = Catalog->GetVehicleBasePrice(TEXT("Vehicle_PricingTest_001"));
	TestEqual(TEXT("Sports car has correct price"), Price1, 35000.0f);

	float Price2 = Catalog->GetVehicleBasePrice(TEXT("Vehicle_PricingTest_002"));
	TestEqual(TEXT("Sedan has correct price"), Price2, 22000.0f);

	// Test: Get price for unknown vehicle (should return 0)
	float InvalidPrice = Catalog->GetVehicleBasePrice(TEXT("Vehicle_DoesNotExist"));
	TestEqual(TEXT("Unknown vehicle returns 0 price"), InvalidPrice, 0.0f);

	// Test: Verify cached lookups (second call should be faster)
	float CachedPrice = Catalog->GetVehicleBasePrice(TEXT("Vehicle_PricingTest_001"));
	TestEqual(TEXT("Cached price matches original"), CachedPrice, 35000.0f);

	return true;
}

/**
 * Test: Vehicle Catalog Class Filtering
 * Verifies that the catalog correctly filters vehicles by class
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGVehicleCatalogClassFilterTest,
	"MidnightGrind.Unit.Catalog.VehicleClassFilter",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGVehicleCatalogClassFilterTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog subsystem
	UMGVehicleCatalogSubsystem* Catalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Catalog subsystem created"), Catalog))
		return false;

	// Create test vehicles of different classes
	TArray<FMGVehicleData> TestVehicles;

	// Create 3 sport vehicles
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Sport, 3));

	// Create 2 sedan vehicles
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Sedan, 2));

	// Create 1 super vehicle
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Super, 1));

	// Create mock data table
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);

	// Set data table and initialize
	Catalog->VehicleDataTable = MockDataTable;
	Catalog->Initialize(nullptr);

	// Test: Get all sport vehicles
	TArray<FMGVehicleData> SportVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::Sport);
	TestEqual(TEXT("Found 3 sport vehicles"), SportVehicles.Num(), 3);

	for (const FMGVehicleData& Vehicle : SportVehicles)
	{
		TestEqual(TEXT("All sport vehicles have correct class"), Vehicle.VehicleClass, EMGVehicleClass::Sport);
	}

	// Test: Get all sedan vehicles
	TArray<FMGVehicleData> SedanVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::Sedan);
	TestEqual(TEXT("Found 2 sedan vehicles"), SedanVehicles.Num(), 2);

	// Test: Get all super vehicles
	TArray<FMGVehicleData> SuperVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::Super);
	TestEqual(TEXT("Found 1 super vehicle"), SuperVehicles.Num(), 1);

	// Test: Get vehicles of class with no entries (should return empty array)
	TArray<FMGVehicleData> OffRoadVehicles = Catalog->GetVehiclesByClass(EMGVehicleClass::OffRoad);
	TestEqual(TEXT("No off-road vehicles found"), OffRoadVehicles.Num(), 0);

	// Test: Verify total vehicle count
	TArray<FMGVehicleData> AllVehicles = Catalog->GetAllVehicles();
	TestEqual(TEXT("Total vehicle count is correct"), AllVehicles.Num(), 6);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

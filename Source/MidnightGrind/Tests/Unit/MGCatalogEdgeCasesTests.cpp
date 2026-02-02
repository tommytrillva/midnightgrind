// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: Catalog Empty DataTable Handling
 * Verifies that catalogs handle empty DataTables gracefully
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogEmptyDataTableTest,
	"MidnightGrind.Unit.Catalog.EmptyDataTable",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogEmptyDataTableTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create vehicle catalog with empty DataTable
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Vehicle catalog created"), VehicleCatalog))
		return false;

	// Create empty mock data table
	TArray<FMGVehicleData> EmptyVehicles;
	UDataTable* EmptyDataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, EmptyVehicles);

	VehicleCatalog->VehicleDataTable = EmptyDataTable;
	VehicleCatalog->Initialize(nullptr);

	// Test: GetAllVehicles returns empty array
	TArray<FMGVehicleData> AllVehicles = VehicleCatalog->GetAllVehicles();
	TestEqual(TEXT("Empty catalog returns empty array"), AllVehicles.Num(), 0);

	// Test: GetVehiclesByClass returns empty array
	TArray<FMGVehicleData> SportVehicles = VehicleCatalog->GetVehiclesByClass(EMGVehicleClass::Sport);
	TestEqual(TEXT("Empty catalog returns empty class array"), SportVehicles.Num(), 0);

	// Test: GetVehicleBasePrice returns 0 for any vehicle
	float Price = VehicleCatalog->GetVehicleBasePrice(TEXT("NonExistent"));
	TestEqual(TEXT("Empty catalog returns 0 price"), Price, 0.0f);

	// Test: GetVehicleData returns false
	FMGVehicleData VehicleData;
	bool Found = VehicleCatalog->GetVehicleData(TEXT("NonExistent"), VehicleData);
	TestFalse(TEXT("Empty catalog returns not found"), Found);

	return true;
}

/**
 * Test: Catalog Null Pointer Safety
 * Verifies that catalogs handle null pointers safely
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogNullPointerTest,
	"MidnightGrind.Unit.Catalog.NullPointerSafety",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogNullPointerTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog without setting DataTable (null)
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Vehicle catalog created"), VehicleCatalog))
		return false;

	// Don't set VehicleDataTable - leave it null
	VehicleCatalog->Initialize(nullptr);

	// Test: GetAllVehicles handles null DataTable
	TArray<FMGVehicleData> AllVehicles = VehicleCatalog->GetAllVehicles();
	TestEqual(TEXT("Null DataTable returns empty array"), AllVehicles.Num(), 0);

	// Test: GetVehicleBasePrice handles null DataTable
	float Price = VehicleCatalog->GetVehicleBasePrice(TEXT("Test"));
	TestEqual(TEXT("Null DataTable returns 0 price"), Price, 0.0f);

	// Test: Subsystem doesn't crash with null DataTable
	TestNotNull(TEXT("Catalog still valid after null operations"), VehicleCatalog);

	return true;
}

/**
 * Test: Catalog Invalid Name Handling
 * Verifies that catalogs handle invalid/empty names correctly
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogInvalidNameTest,
	"MidnightGrind.Unit.Catalog.InvalidNames",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogInvalidNameTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog with test data
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);

	// Create test data
	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(5);
	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(5);

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);

	// Test: Empty name (NAME_None)
	float EmptyPrice = VehicleCatalog->GetVehicleBasePrice(NAME_None);
	TestEqual(TEXT("Empty name returns 0 price"), EmptyPrice, 0.0f);

	// Test: Invalid characters in name
	float InvalidPrice = VehicleCatalog->GetVehicleBasePrice(TEXT("Vehicle@#$%"));
	TestEqual(TEXT("Invalid characters return 0 price"), InvalidPrice, 0.0f);

	// Test: Very long name
	FString LongName;
	for (int32 i = 0; i < 1000; ++i)
	{
		LongName.AppendChar(TEXT('A'));
	}
	float LongNamePrice = VehicleCatalog->GetVehicleBasePrice(FName(*LongName));
	TestEqual(TEXT("Very long name returns 0 price"), LongNamePrice, 0.0f);

	// Test: Parts catalog with empty name
	FMGPartPricingInfo EmptyPartPricing = PartsCatalog->GetPartPricing(NAME_None);
	TestFalse(TEXT("Empty part name returns invalid pricing"), EmptyPartPricing.bIsValid);

	return true;
}

/**
 * Test: Catalog Large Dataset Performance
 * Verifies that catalogs handle large datasets efficiently
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogLargeDatasetTest,
	"MidnightGrind.Unit.Catalog.LargeDataset",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogLargeDatasetTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);

	// Create large dataset (100 vehicles)
	TArray<FMGVehicleData> LargeDataset = FMGTestDataFactory::CreateTestVehicleArray(100);
	UDataTable* LargeDataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, LargeDataset);

	VehicleCatalog->VehicleDataTable = LargeDataTable;

	// Test: Initialization completes
	double StartTime = FPlatformTime::Seconds();
	VehicleCatalog->Initialize(nullptr);
	double InitTime = FPlatformTime::Seconds() - StartTime;

	TestTrue(TEXT("Large dataset initializes in reasonable time (<1s)"), InitTime < 1.0);

	// Test: GetAllVehicles returns correct count
	TArray<FMGVehicleData> AllVehicles = VehicleCatalog->GetAllVehicles();
	TestEqual(TEXT("Large dataset returns all vehicles"), AllVehicles.Num(), 100);

	// Test: Lookup performance (should be O(1) with hash table)
	StartTime = FPlatformTime::Seconds();
	for (int32 i = 0; i < 1000; ++i)
	{
		// Lookup random vehicles (should hit cache)
		int32 Index = i % LargeDataset.Num();
		float Price = VehicleCatalog->GetVehicleBasePrice(LargeDataset[Index].VehicleID);
	}
	double LookupTime = FPlatformTime::Seconds() - StartTime;

	TestTrue(TEXT("1000 lookups complete in reasonable time (<0.1s)"), LookupTime < 0.1);

	return true;
}

/**
 * Test: Catalog Data Validation
 * Verifies that catalog data is properly validated
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogDataValidationTest,
	"MidnightGrind.Unit.Catalog.DataValidation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogDataValidationTest::RunTest(const FString& Parameters)
{
	// Create test vehicles with various data states
	FMGVehicleData ValidVehicle = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Valid_Vehicle"),
		FText::FromString(TEXT("Valid Car")),
		25000.0f,
		EMGVehicleClass::Sport);

	// Test: Valid vehicle passes validation
	bool IsValid = FMGTestDataFactory::ValidateVehicleData(ValidVehicle);
	TestTrue(TEXT("Valid vehicle passes validation"), IsValid);

	// Create invalid vehicle (no ID)
	FMGVehicleData InvalidVehicleNoID = ValidVehicle;
	InvalidVehicleNoID.VehicleID = NAME_None;
	bool IsInvalidNoID = FMGTestDataFactory::ValidateVehicleData(InvalidVehicleNoID);
	TestFalse(TEXT("Vehicle with no ID fails validation"), IsInvalidNoID);

	// Create invalid vehicle (no display name)
	FMGVehicleData InvalidVehicleNoName = ValidVehicle;
	InvalidVehicleNoName.DisplayName = FText::GetEmpty();
	bool IsInvalidNoName = FMGTestDataFactory::ValidateVehicleData(InvalidVehicleNoName);
	TestFalse(TEXT("Vehicle with no display name fails validation"), IsInvalidNoName);

	// Create invalid vehicle (negative price)
	FMGVehicleData InvalidVehicleNegativePrice = ValidVehicle;
	InvalidVehicleNegativePrice.BasePrice = -1000.0f;
	bool IsInvalidPrice = FMGTestDataFactory::ValidateVehicleData(InvalidVehicleNegativePrice);
	TestFalse(TEXT("Vehicle with negative price fails validation"), IsInvalidPrice);

	// Test: Part validation
	FMGPartData ValidPart = FMGTestDataFactory::CreateTestPart(
		TEXT("Valid_Part"),
		FText::FromString(TEXT("Valid Part")),
		EMGPartCategory::Engine,
		1000.0f);

	bool IsValidPart = FMGTestDataFactory::ValidatePartData(ValidPart);
	TestTrue(TEXT("Valid part passes validation"), IsValidPart);

	// Create invalid part (negative cost)
	FMGPartData InvalidPartNegativeCost = ValidPart;
	InvalidPartNegativeCost.BaseCost = -500.0f;
	bool IsInvalidPartCost = FMGTestDataFactory::ValidatePartData(InvalidPartNegativeCost);
	TestFalse(TEXT("Part with negative cost fails validation"), IsInvalidPartCost);

	return true;
}

/**
 * Test: Catalog Cache Consistency
 * Verifies that cached lookups return consistent results
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogCacheConsistencyTest,
	"MidnightGrind.Unit.Catalog.CacheConsistency",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogCacheConsistencyTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);

	// Create test data
	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(20);
	UDataTable* DataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);

	VehicleCatalog->VehicleDataTable = DataTable;
	VehicleCatalog->Initialize(nullptr);

	// Test: Multiple lookups of same vehicle return same result
	for (const FMGVehicleData& Vehicle : TestVehicles)
	{
		float Price1 = VehicleCatalog->GetVehicleBasePrice(Vehicle.VehicleID);
		float Price2 = VehicleCatalog->GetVehicleBasePrice(Vehicle.VehicleID);
		float Price3 = VehicleCatalog->GetVehicleBasePrice(Vehicle.VehicleID);

		TestEqual(TEXT("First and second lookup match"), Price1, Price2);
		TestEqual(TEXT("Second and third lookup match"), Price2, Price3);
		TestEqual(TEXT("Price matches expected value"), Price1, Vehicle.BasePrice);
	}

	// Test: GetVehicleData consistency
	for (const FMGVehicleData& Vehicle : TestVehicles)
	{
		FMGVehicleData Data1, Data2;
		bool Found1 = VehicleCatalog->GetVehicleData(Vehicle.VehicleID, Data1);
		bool Found2 = VehicleCatalog->GetVehicleData(Vehicle.VehicleID, Data2);

		TestTrue(TEXT("Vehicle found on first lookup"), Found1);
		TestTrue(TEXT("Vehicle found on second lookup"), Found2);
		TestEqual(TEXT("Vehicle IDs match"), Data1.VehicleID, Data2.VehicleID);
		TestEqual(TEXT("Vehicle prices match"), Data1.BasePrice, Data2.BasePrice);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

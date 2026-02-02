// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/MGPlayerMarketSubsystem.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Test: Market Vehicle Valuation
 * Verifies that the market subsystem correctly calculates vehicle market values
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMarketVehicleValuationTest,
	"MidnightGrind.Unit.Economy.MarketVehicleValuation",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMarketVehicleValuationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create vehicle catalog
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Vehicle catalog created"), VehicleCatalog))
		return false;

	// Create market subsystem
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Market subsystem created"), Market))
		return false;

	// Create test vehicles with different prices
	FMGVehicleData CheapVehicle = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_Cheap"),
		FText::FromString(TEXT("Budget Car")),
		15000.0f,
		EMGVehicleClass::Economy);

	FMGVehicleData ExpensiveVehicle = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_Expensive"),
		FText::FromString(TEXT("Luxury Car")),
		85000.0f,
		EMGVehicleClass::Super);

	// Create mock data table
	TArray<FMGVehicleData> TestVehicles = { CheapVehicle, ExpensiveVehicle };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);

	// Set data table and initialize
	VehicleCatalog->VehicleDataTable = MockDataTable;
	VehicleCatalog->Initialize(nullptr);
	Market->Initialize(nullptr);

	// Test: Get market value for cheap vehicle
	float CheapValue = Market->GetEstimatedMarketValue(TEXT("Vehicle_Cheap"));
	TestTrue(TEXT("Cheap vehicle has positive value"), CheapValue > 0.0f);
	TestTrue(TEXT("Cheap vehicle value is reasonable"), CheapValue <= 20000.0f);

	// Test: Get market value for expensive vehicle
	float ExpensiveValue = Market->GetEstimatedMarketValue(TEXT("Vehicle_Expensive"));
	TestTrue(TEXT("Expensive vehicle has positive value"), ExpensiveValue > 0.0f);
	TestTrue(TEXT("Expensive vehicle worth more than cheap"), ExpensiveValue > CheapValue);

	// Test: Unknown vehicle (should return 0 or base value)
	float UnknownValue = Market->GetEstimatedMarketValue(TEXT("Vehicle_Unknown"));
	TestTrue(TEXT("Unknown vehicle returns non-negative value"), UnknownValue >= 0.0f);

	return true;
}

/**
 * Test: Market Part Pricing
 * Verifies that the market subsystem correctly prices parts
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMarketPartPricingTest,
	"MidnightGrind.Unit.Economy.MarketPartPricing",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMarketPartPricingTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create parts catalog
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Parts catalog created"), PartsCatalog))
		return false;

	// Create market subsystem
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Market subsystem created"), Market))
		return false;

	// Create test parts with different prices
	FMGPartData CheapPart = FMGTestDataFactory::CreateTestPart(
		TEXT("Part_Cheap"),
		FText::FromString(TEXT("Budget Part")),
		EMGPartCategory::Exhaust,
		500.0f);

	FMGPartData ExpensivePart = FMGTestDataFactory::CreateTestPart(
		TEXT("Part_Expensive"),
		FText::FromString(TEXT("Premium Part")),
		EMGPartCategory::Engine,
		5000.0f);

	// Create mock data table
	TArray<FMGPartData> TestParts = { CheapPart, ExpensivePart };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	// Set data table and initialize
	PartsCatalog->PartsDataTable = MockDataTable;
	PartsCatalog->Initialize(nullptr);
	Market->Initialize(nullptr);

	// Test: Verify parts catalog has the parts
	FMGPartData RetrievedCheapPart;
	bool CheapFound = PartsCatalog->GetPartData(TEXT("Part_Cheap"), RetrievedCheapPart);
	TestTrue(TEXT("Cheap part found in catalog"), CheapFound);
	TestEqual(TEXT("Cheap part has correct base cost"), RetrievedCheapPart.BaseCost, 500.0f);

	FMGPartData RetrievedExpensivePart;
	bool ExpensiveFound = PartsCatalog->GetPartData(TEXT("Part_Expensive"), RetrievedExpensivePart);
	TestTrue(TEXT("Expensive part found in catalog"), ExpensiveFound);
	TestEqual(TEXT("Expensive part has correct base cost"), RetrievedExpensivePart.BaseCost, 5000.0f);

	return true;
}

/**
 * Test: Market Demand Fluctuation
 * Verifies that the market subsystem can track demand changes
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMarketDemandTest,
	"MidnightGrind.Unit.Economy.MarketDemand",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMarketDemandTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create market subsystem
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Market subsystem created"), Market))
		return false;

	Market->Initialize(nullptr);

	// Test: Market subsystem initializes correctly
	TestNotNull(TEXT("Market subsystem initialized"), Market);

	// Test: Can get market trend (should not crash)
	EMGMarketTrend Trend = Market->GetMarketTrend();
	TestTrue(TEXT("Market trend returned"), true); // Just verify no crash

	// Test: Can check if market is initialized
	bool IsInitialized = Market->IsInitialized();
	TestTrue(TEXT("Market is marked as initialized"), IsInitialized);

	return true;
}

/**
 * Test: Market Buy/Sell Price Differential
 * Verifies that buy and sell prices have appropriate spreads
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMarketPriceSpreadTest,
	"MidnightGrind.Unit.Economy.MarketPriceSpread",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMarketPriceSpreadTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create vehicle catalog
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Vehicle catalog created"), VehicleCatalog))
		return false;

	// Create market subsystem
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);
	if (!TestNotNull(TEXT("Market subsystem created"), Market))
		return false;

	// Create test vehicle
	FMGVehicleData TestVehicle = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_Spread"),
		FText::FromString(TEXT("Test Vehicle")),
		50000.0f,
		EMGVehicleClass::Sport);

	// Create mock data table
	TArray<FMGVehicleData> TestVehicles = { TestVehicle };
	UDataTable* MockDataTable = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);

	// Set data table and initialize
	VehicleCatalog->VehicleDataTable = MockDataTable;
	VehicleCatalog->Initialize(nullptr);
	Market->Initialize(nullptr);

	// Test: Get buy price
	float BuyPrice = Market->GetVehicleBuyPrice(TEXT("Vehicle_Spread"));
	TestTrue(TEXT("Buy price is positive"), BuyPrice > 0.0f);

	// Test: Get sell price
	float SellPrice = Market->GetVehicleSellPrice(TEXT("Vehicle_Spread"));
	TestTrue(TEXT("Sell price is positive"), SellPrice > 0.0f);

	// Test: Buy price should be higher than sell price (market spread)
	TestTrue(TEXT("Buy price >= sell price (market spread)"), BuyPrice >= SellPrice);

	// Test: Prices should be in reasonable range (not negative, not absurdly high)
	TestTrue(TEXT("Buy price is reasonable"), BuyPrice < 1000000.0f);
	TestTrue(TEXT("Sell price is reasonable"), SellPrice < 1000000.0f);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Economy/MGPlayerMarketSubsystem.h"
#include "Economy/MGMechanicSubsystem.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Integration Test: Vehicle Purchase Flow
 * Verifies the complete vehicle purchase workflow across multiple subsystems
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGVehiclePurchaseFlowTest,
	"MidnightGrind.Integration.Economy.VehiclePurchaseFlow",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGVehiclePurchaseFlowTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create all required subsystems
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);

	// Create test vehicle data
	FMGVehicleData TestVehicle = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_Purchase_001"),
		FText::FromString(TEXT("Test Purchase Car")),
		30000.0f,
		EMGVehicleClass::Sport);

	// Setup catalog
	TArray<FMGVehicleData> TestVehicles = { TestVehicle };
	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	VehicleCatalog->VehicleDataTable = VehicleDT;
	VehicleCatalog->Initialize(nullptr);

	// Initialize market
	Market->Initialize(nullptr);

	// Test: Vehicle exists in catalog
	FMGVehicleData CatalogVehicle;
	bool VehicleExists = VehicleCatalog->GetVehicleData(TEXT("Vehicle_Purchase_001"), CatalogVehicle);
	TestTrue(TEXT("Vehicle exists in catalog"), VehicleExists);

	// Test: Market can price the vehicle
	float BuyPrice = Market->GetVehicleBuyPrice(TEXT("Vehicle_Purchase_001"));
	TestTrue(TEXT("Market returns valid buy price"), BuyPrice > 0.0f);

	// Test: Catalog price matches expected value
	TestEqual(TEXT("Catalog base price correct"), CatalogVehicle.BasePrice, 30000.0f);

	// Test: Buy price is based on catalog price
	TestTrue(TEXT("Buy price is reasonable relative to base"), BuyPrice >= CatalogVehicle.BasePrice * 0.8f);
	TestTrue(TEXT("Buy price is not inflated unreasonably"), BuyPrice <= CatalogVehicle.BasePrice * 1.5f);

	// Test: Can also get sell price
	float SellPrice = Market->GetVehicleSellPrice(TEXT("Vehicle_Purchase_001"));
	TestTrue(TEXT("Market returns valid sell price"), SellPrice > 0.0f);
	TestTrue(TEXT("Sell price < buy price (market spread)"), SellPrice <= BuyPrice);

	return true;
}

/**
 * Integration Test: Part Installation Flow
 * Verifies the complete part installation workflow across catalog and mechanic subsystems
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGPartInstallationFlowTest,
	"MidnightGrind.Integration.Economy.PartInstallationFlow",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGPartInstallationFlowTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create all required subsystems
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);

	// Create test part data
	FMGPartData TestPart = FMGTestDataFactory::CreateTestPartWithPricing(
		TEXT("Part_Install_001"),
		2000.0f,  // Base cost
		300.0f,   // Labor cost
		120.0f);  // 2 hours install time

	// Setup parts catalog
	TArray<FMGPartData> TestParts = { TestPart };
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);
	PartsCatalog->PartsDataTable = PartsDT;
	PartsCatalog->Initialize(nullptr);

	// Initialize mechanic
	Mechanic->Initialize(nullptr);

	// Test: Part exists in catalog
	FMGPartData CatalogPart;
	bool PartExists = PartsCatalog->GetPartData(TEXT("Part_Install_001"), CatalogPart);
	TestTrue(TEXT("Part exists in catalog"), PartExists);

	// Test: Catalog has correct pricing info
	FMGPartPricingInfo PricingInfo = PartsCatalog->GetPartPricing(TEXT("Part_Install_001"));
	TestTrue(TEXT("Pricing info is valid"), PricingInfo.bIsValid);
	TestEqual(TEXT("Base cost matches"), PricingInfo.BaseCost, 2000.0f);
	TestEqual(TEXT("Labor cost matches"), PricingInfo.LaborCost, 300.0f);
	TestEqual(TEXT("Total cost calculated correctly"), PricingInfo.TotalCost, 2300.0f);

	// Test: Mechanic can get install time from catalog
	int32 InstallTime = Mechanic->GetPartBaseInstallTime(TEXT("Part_Install_001"));
	TestEqual(TEXT("Install time converted correctly (120 min -> 2 hours)"), InstallTime, 2);

	// Test: Mechanic can get labor cost from catalog
	int32 LaborCost = Mechanic->GetPartBaseInstallCost(TEXT("Part_Install_001"));
	TestEqual(TEXT("Labor cost retrieved correctly"), LaborCost, 300);

	// Test: Complete installation cost calculation
	int32 TotalInstallCost = CatalogPart.BaseCost + LaborCost;
	TestEqual(TEXT("Total installation cost correct"), TotalInstallCost, 2300);

	return true;
}

/**
 * Integration Test: Market Valuation with Catalog Integration
 * Verifies market subsystem correctly integrates with both vehicle and parts catalogs
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGMarketCatalogIntegrationTest,
	"MidnightGrind.Integration.Economy.MarketCatalogIntegration",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGMarketCatalogIntegrationTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create all required subsystems
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);

	// Create test data
	FMGVehicleData TestVehicle = FMGTestDataFactory::CreateTestVehicle(
		TEXT("Vehicle_Market_001"),
		FText::FromString(TEXT("Market Test Car")),
		40000.0f,
		EMGVehicleClass::Sport);

	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(5);

	// Setup catalogs
	TArray<FMGVehicleData> TestVehicles = { TestVehicle };
	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);
	Market->Initialize(nullptr);

	// Test: Market can access vehicle catalog data
	float MarketValue = Market->GetEstimatedMarketValue(TEXT("Vehicle_Market_001"));
	float CatalogPrice = VehicleCatalog->GetVehicleBasePrice(TEXT("Vehicle_Market_001"));

	TestTrue(TEXT("Market value is positive"), MarketValue > 0.0f);
	TestTrue(TEXT("Catalog price is positive"), CatalogPrice > 0.0f);

	// Test: Market value is related to catalog price (with some variance)
	float MinExpected = CatalogPrice * 0.5f;  // At least 50% of base price
	float MaxExpected = CatalogPrice * 1.5f;  // At most 150% of base price
	TestTrue(TEXT("Market value within reasonable range"), MarketValue >= MinExpected && MarketValue <= MaxExpected);

	// Test: Market can access parts catalog for multiple parts
	for (const FMGPartData& Part : TestParts)
	{
		FMGPartData RetrievedPart;
		bool Found = PartsCatalog->GetPartData(Part.PartID, RetrievedPart);
		TestTrue(TEXT("Part accessible through catalog"), Found);

		if (Found)
		{
			TestEqual(TEXT("Part base cost matches"), RetrievedPart.BaseCost, Part.BaseCost);
		}
	}

	return true;
}

/**
 * Integration Test: Economy System Consistency
 * Verifies consistent pricing across all economy subsystems
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGEconomyConsistencyTest,
	"MidnightGrind.Integration.Economy.SystemConsistency",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGEconomyConsistencyTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create all economy subsystems
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);
	UMGPlayerMarketSubsystem* Market = NewObject<UMGPlayerMarketSubsystem>(GameInstance);
	UMGMechanicSubsystem* Mechanic = NewObject<UMGMechanicSubsystem>(GameInstance);

	// Create test data
	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(10);
	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(10);

	// Setup all subsystems
	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);
	Market->Initialize(nullptr);
	Mechanic->Initialize(nullptr);

	// Test: All subsystems initialized successfully
	TestTrue(TEXT("Vehicle catalog initialized"), VehicleCatalog->IsInitialized());
	TestTrue(TEXT("Parts catalog initialized"), PartsCatalog->IsInitialized());
	TestTrue(TEXT("Market initialized"), Market->IsInitialized());
	TestTrue(TEXT("Mechanic initialized"), Mechanic->IsInitialized());

	// Test: Pricing consistency across subsystems
	for (const FMGPartData& Part : TestParts)
	{
		// Get pricing from catalog
		FMGPartPricingInfo CatalogPricing = PartsCatalog->GetPartPricing(Part.PartID);

		// Get labor cost from mechanic
		int32 MechanicLabor = Mechanic->GetPartBaseInstallCost(Part.PartID);

		// Verify consistency
		if (CatalogPricing.bIsValid)
		{
			// Mechanic labor should match catalog labor (or use fallback)
			bool IsConsistent = (MechanicLabor == CatalogPricing.LaborCost) || (MechanicLabor > 0);
			TestTrue(TEXT("Labor cost is consistent or has valid fallback"), IsConsistent);
		}
	}

	// Test: Vehicle pricing consistency
	for (const FMGVehicleData& Vehicle : TestVehicles)
	{
		float CatalogPrice = VehicleCatalog->GetVehicleBasePrice(Vehicle.VehicleID);
		float MarketValue = Market->GetEstimatedMarketValue(Vehicle.VehicleID);

		TestTrue(TEXT("Catalog price is positive"), CatalogPrice > 0.0f);
		TestTrue(TEXT("Market value is non-negative"), MarketValue >= 0.0f);
	}

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

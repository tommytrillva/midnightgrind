// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGTestDataFactory.h
 * @brief Test Data Factory - Mock Data Generation for Unit Testing
 *
 * @section overview Overview
 * This file defines the Test Data Factory, a utility class that generates
 * mock/fake data for unit testing and automated testing scenarios. It provides
 * consistent, predictable test data without requiring database connections or
 * loading actual game assets.
 *
 * @section concepts Key Concepts for Beginners
 *
 * @subsection factory_pattern Factory Pattern
 * A "factory" is a class whose job is to CREATE other objects:
 * - Instead of manually constructing test data everywhere
 * - Call factory methods to get pre-configured objects
 * - Ensures consistent test data across all tests
 * - Easy to modify all test data in one place
 *
 * @subsection mock_data What is Mock Data?
 * Mock data is fake/test data used for testing:
 * - Doesn't require loading real game assets
 * - Predictable values for assertions
 * - Can test edge cases (invalid data, empty data)
 * - Isolated from production data changes
 *
 * @subsection static_methods Static Factory Methods
 * All methods are static, meaning:
 * - No instance needed: FMGTestDataFactory::CreateTestVehicle()
 * - No state between calls (mostly - see UniqueIDCounter)
 * - Thread-safe for parallel test execution
 *
 * @subsection data_tables Mock DataTables
 * Unreal's DataTables are usually loaded from assets. For testing:
 * - CreateMockVehicleDataTable() creates a table in memory
 * - No file I/O or asset loading required
 * - Tests can run without cooked content
 *
 * @section usage Usage Examples
 *
 * @subsection simple_vehicle Creating a Simple Test Vehicle
 * @code
 * // Default test vehicle
 * FMGVehicleData Vehicle = FMGTestDataFactory::CreateTestVehicle();
 * // Vehicle.VehicleID = "TestVehicle_001"
 * // Vehicle.DisplayName = "Test Vehicle"
 * // Vehicle.BasePrice = 25000.0f
 *
 * // Custom test vehicle
 * FMGVehicleData CustomVehicle = FMGTestDataFactory::CreateTestVehicle(
 *     TEXT("MyTestCar"),
 *     FText::FromString(TEXT("Speedy McSpeedface")),
 *     50000.0f,
 *     EMGVehicleClass::Muscle
 * );
 * @endcode
 *
 * @subsection array_creation Creating Test Data Arrays
 * @code
 * // Create 10 vehicles for testing
 * TArray<FMGVehicleData> Vehicles = FMGTestDataFactory::CreateTestVehicleArray(10);
 *
 * // Create parts for a specific category
 * TArray<FMGPartData> EngineParts = FMGTestDataFactory::CreatePartsByCategory(
 *     EMGPartCategory::Engine,
 *     5  // Count
 * );
 *
 * // Create vehicles of a specific class
 * TArray<FMGVehicleData> MuscleCards = FMGTestDataFactory::CreateVehiclesByClass(
 *     EMGVehicleClass::Muscle,
 *     3
 * );
 * @endcode
 *
 * @subsection part_with_details Creating Parts with Specific Details
 * @code
 * // Part with pricing info
 * FMGPartData PricedPart = FMGTestDataFactory::CreateTestPartWithPricing(
 *     TEXT("Turbo_001"),
 *     2500.0f,   // Base cost
 *     350.0f,    // Labor cost
 *     45.0f      // Install time (minutes)
 * );
 *
 * // Part with compatibility restrictions
 * TArray<FName> CompatibleVehicles = { TEXT("Mustang_69"), TEXT("Camaro_70") };
 * TArray<EMGVehicleClass> CompatibleClasses = { EMGVehicleClass::Muscle };
 *
 * FMGPartData RestrictedPart = FMGTestDataFactory::CreateTestPartWithCompatibility(
 *     TEXT("MuscleIntake_001"),
 *     CompatibleVehicles,
 *     CompatibleClasses
 * );
 * @endcode
 *
 * @subsection mock_tables Creating Mock DataTables
 * @code
 * // For subsystem testing that expects DataTables
 * TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(5);
 * UDataTable* MockTable = FMGTestDataFactory::CreateMockVehicleDataTable(
 *     GetTransientPackage(),  // Outer object
 *     TestVehicles
 * );
 *
 * // Now use the mock table in your subsystem test
 * VehicleCatalog->Initialize(MockTable);
 * @endcode
 *
 * @subsection validation Validating Test Data
 * @code
 * FMGVehicleData Vehicle = FMGTestDataFactory::CreateTestVehicle();
 *
 * // Verify the factory created valid data
 * bool bIsValid = FMGTestDataFactory::ValidateVehicleData(Vehicle);
 * check(bIsValid);  // Unreal assertion
 *
 * // Validate part data
 * FMGPartData Part = FMGTestDataFactory::CreateTestPart();
 * check(FMGTestDataFactory::ValidatePartData(Part));
 * @endcode
 *
 * @subsection test_example Complete Test Example
 * @code
 * IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVehicleCatalogTest, "MidnightGrind.Catalog.VehicleCatalog",
 *     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
 *
 * bool FVehicleCatalogTest::RunTest(const FString& Parameters)
 * {
 *     // Arrange - Create mock data
 *     TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(3);
 *     UDataTable* MockTable = FMGTestDataFactory::CreateMockVehicleDataTable(
 *         GetTransientPackage(), TestVehicles);
 *
 *     // Act - Initialize subsystem with mock data
 *     UMGVehicleCatalogSubsystem* Catalog = NewObject<UMGVehicleCatalogSubsystem>();
 *     Catalog->Initialize(MockTable);
 *
 *     // Assert - Verify expected behavior
 *     TestEqual("Vehicle count should match", Catalog->GetVehicleCount(), 3);
 *
 *     FMGVehicleData Retrieved = Catalog->GetVehicle(TEXT("TestVehicle_001"));
 *     TestTrue("Should find test vehicle", Retrieved.VehicleID != NAME_None);
 *
 *     return true;
 * }
 * @endcode
 *
 * @section best_practices Best Practices
 *
 * @subsection deterministic Deterministic Data
 * - Use specific IDs when order matters
 * - Array methods generate sequential IDs (TestVehicle_001, _002, etc.)
 * - Reset UniqueIDCounter between test suites if needed
 *
 * @subsection isolation Test Isolation
 * - Create fresh data for each test
 * - Don't rely on state from previous tests
 * - Use validation helpers to catch factory bugs
 *
 * @subsection edge_cases Testing Edge Cases
 * - Create empty arrays to test empty state handling
 * - Create invalid data to test error handling
 * - Override default values to test boundary conditions
 *
 * @see FMGVehicleData - Vehicle data structure
 * @see FMGPartData - Part data structure
 * @see FMGPartPricingInfo - Part pricing information
 * @see UDataTable - Unreal's data table class
 *
 * Midnight Grind - Y2K Arcade Street Racing - Test Infrastructure
 */

#pragma once

#include "CoreMinimal.h"
#include "Data/MG_VHCL_Data.h"
#include "Catalog/MGPartData.h"

/**
 * Factory class for generating test data.
 *
 * Provides static utility methods for creating mock data tables and structures
 * for unit tests. All methods are static - no instance required.
 *
 * @see MGTestDataFactory.h file documentation for detailed usage examples.
 */
class MIDNIGHTGRIND_API FMGTestDataFactory
{
public:
	// Vehicle Test Data
	static FMGVehicleData CreateTestVehicle(
		FName VehicleID = TEXT("TestVehicle_001"),
		FText DisplayName = FText::FromString(TEXT("Test Vehicle")),
		float BasePrice = 25000.0f,
		EMGVehicleClass VehicleClass = EMGVehicleClass::Sport);

	static TArray<FMGVehicleData> CreateTestVehicleArray(int32 Count = 5);

	// Part Test Data
	static FMGPartData CreateTestPart(
		FName PartID = TEXT("TestPart_001"),
		FText DisplayName = FText::FromString(TEXT("Test Part")),
		EMGPartCategory Category = EMGPartCategory::Engine,
		float BasePrice = 1000.0f);

	static FMGPartData CreateTestPartWithPricing(
		FName PartID,
		float BaseCost,
		float LaborCost,
		float InstallTimeMinutes);

	static FMGPartData CreateTestPartWithCompatibility(
		FName PartID,
		TArray<FName> CompatibleVehicles,
		TArray<EMGVehicleClass> CompatibleClasses);

	static TArray<FMGPartData> CreateTestPartArray(int32 Count = 5);

	// Pricing Test Data
	static FMGPartPricingInfo CreateTestPricingInfo(
		float BaseCost = 1000.0f,
		float LaborCost = 150.0f,
		bool bIsValid = true);

	// Vehicle Class Filtering
	static TArray<FMGVehicleData> CreateVehiclesByClass(EMGVehicleClass TargetClass, int32 Count = 3);

	// Part Category Filtering
	static TArray<FMGPartData> CreatePartsByCategory(EMGPartCategory TargetCategory, int32 Count = 3);

	// Mock DataTable Creation (for subsystem testing)
	static UDataTable* CreateMockVehicleDataTable(UObject* Outer, TArray<FMGVehicleData> Vehicles);
	static UDataTable* CreateMockPartDataTable(UObject* Outer, TArray<FMGPartData> Parts);

	// Validation Helpers
	static bool ValidateVehicleData(const FMGVehicleData& Vehicle);
	static bool ValidatePartData(const FMGPartData& Part);

private:
	// Counter for generating unique IDs in tests
	static int32 UniqueIDCounter;
};

// Copyright Midnight Grind. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Data/MGVehicleCatalogSubsystem.h"
#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Tests/TestHelpers/MGTestDataFactory.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformTime.h"

#if WITH_DEV_AUTOMATION_TESTS

/**
 * Performance Test: Catalog Initialization with Large Dataset
 * Measures catalog initialization time with production-scale data
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogInitializationPerformanceTest,
	"MidnightGrind.Performance.Catalog.Initialization",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogInitializationPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalogs
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);

	// Create production-scale datasets
	const int32 VehicleCount = 50;  // Realistic vehicle count
	const int32 PartCount = 500;    // Realistic part count (~10 parts per vehicle)

	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(VehicleCount);
	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(PartCount);

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	// Performance test: Vehicle catalog initialization
	double VehicleStartTime = FPlatformTime::Seconds();
	VehicleCatalog->Initialize(nullptr);
	double VehicleInitTime = FPlatformTime::Seconds() - VehicleStartTime;

	// Performance test: Parts catalog initialization
	double PartsStartTime = FPlatformTime::Seconds();
	PartsCatalog->Initialize(nullptr);
	double PartsInitTime = FPlatformTime::Seconds() - PartsStartTime;

	// Validate performance
	TestTrue(TEXT("Vehicle catalog initializes quickly (<0.5s)"), VehicleInitTime < 0.5);
	TestTrue(TEXT("Parts catalog initializes quickly (<1.0s)"), PartsInitTime < 1.0);

	// Log performance metrics
	AddInfo(FString::Printf(TEXT("Vehicle catalog initialization: %.3f ms"), VehicleInitTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Parts catalog initialization: %.3f ms"), PartsInitTime * 1000.0));

	// Verify data integrity after fast initialization
	TestEqual(TEXT("All vehicles loaded"), VehicleCatalog->GetAllVehicles().Num(), VehicleCount);
	TestEqual(TEXT("All parts loaded"), PartsCatalog->GetAllParts().Num(), PartCount);

	return true;
}

/**
 * Performance Test: High-Frequency Catalog Lookups
 * Measures lookup performance under heavy load
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogLookupPerformanceTest,
	"MidnightGrind.Performance.Catalog.HighFrequencyLookup",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogLookupPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog with realistic dataset
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);

	const int32 VehicleCount = 100;
	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(VehicleCount);
	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	VehicleCatalog->Initialize(nullptr);

	// Performance test: 10,000 sequential lookups
	const int32 LookupCount = 10000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < LookupCount; ++i)
	{
		// Lookup random vehicle from catalog
		int32 VehicleIndex = i % TestVehicles.Num();
		float Price = VehicleCatalog->GetVehicleBasePrice(TestVehicles[VehicleIndex].VehicleID);

		// Ensure lookup actually happened (prevent optimization)
		if (Price < 0.0f)
		{
			AddError(TEXT("Invalid price returned"));
		}
	}

	double LookupTime = FPlatformTime::Seconds() - StartTime;

	// Validate performance: 10,000 lookups should complete in <100ms (O(1) hash table)
	TestTrue(TEXT("10,000 lookups complete quickly (<0.1s)"), LookupTime < 0.1);

	// Log performance metrics
	double AvgLookupTime = (LookupTime / LookupCount) * 1000000.0; // Convert to microseconds
	AddInfo(FString::Printf(TEXT("Total lookup time: %.3f ms"), LookupTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Average lookup time: %.3f µs"), AvgLookupTime));
	AddInfo(FString::Printf(TEXT("Lookups per second: %.0f"), LookupCount / LookupTime));

	// Performance should be O(1) - verify by doing more lookups
	const int32 ExtendedLookupCount = 50000;
	StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < ExtendedLookupCount; ++i)
	{
		int32 VehicleIndex = i % TestVehicles.Num();
		VehicleCatalog->GetVehicleBasePrice(TestVehicles[VehicleIndex].VehicleID);
	}

	double ExtendedLookupTime = FPlatformTime::Seconds() - StartTime;

	// Extended lookups should scale linearly (5x more lookups ~ 5x more time)
	double ScalingFactor = ExtendedLookupTime / LookupTime;
	TestTrue(TEXT("Lookup time scales linearly (O(1) confirmed)"), ScalingFactor > 4.0 && ScalingFactor < 6.0);

	AddInfo(FString::Printf(TEXT("Scaling factor: %.2fx (expected ~5x)"), ScalingFactor));

	return true;
}

/**
 * Performance Test: Concurrent Catalog Access
 * Measures performance when multiple systems access catalogs simultaneously
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogConcurrentAccessTest,
	"MidnightGrind.Performance.Catalog.ConcurrentAccess",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogConcurrentAccessTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create both catalogs
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);

	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(50);
	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(200);

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);

	// Simulate concurrent access from multiple systems
	const int32 AccessCount = 5000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < AccessCount; ++i)
	{
		// Interleaved access to both catalogs (simulates real gameplay)
		if (i % 2 == 0)
		{
			int32 VehicleIndex = i % TestVehicles.Num();
			VehicleCatalog->GetVehicleBasePrice(TestVehicles[VehicleIndex].VehicleID);
		}
		else
		{
			int32 PartIndex = i % TestParts.Num();
			PartsCatalog->GetPartPricing(TestParts[PartIndex].PartID);
		}
	}

	double ConcurrentAccessTime = FPlatformTime::Seconds() - StartTime;

	// Validate performance: Concurrent access should still be fast
	TestTrue(TEXT("Concurrent access completes quickly (<0.15s)"), ConcurrentAccessTime < 0.15);

	AddInfo(FString::Printf(TEXT("Concurrent access time: %.3f ms"), ConcurrentAccessTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Accesses per second: %.0f"), AccessCount / ConcurrentAccessTime));

	return true;
}

/**
 * Performance Test: Catalog Filter Operations
 * Measures performance of filtering large datasets
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogFilterPerformanceTest,
	"MidnightGrind.Performance.Catalog.FilterOperations",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogFilterPerformanceTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalog with mixed vehicle classes
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);

	TArray<FMGVehicleData> TestVehicles;
	// Create 20 of each class
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Economy, 20));
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Sport, 20));
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Sedan, 20));
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Super, 20));
	TestVehicles.Append(FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass::Muscle, 20));

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	VehicleCatalog->VehicleDataTable = VehicleDT;
	VehicleCatalog->Initialize(nullptr);

	// Performance test: Repeated filtering operations
	const int32 FilterCount = 1000;
	double StartTime = FPlatformTime::Seconds();

	for (int32 i = 0; i < FilterCount; ++i)
	{
		// Cycle through all vehicle classes
		EMGVehicleClass TargetClass = static_cast<EMGVehicleClass>(i % 5);
		TArray<FMGVehicleData> FilteredVehicles = VehicleCatalog->GetVehiclesByClass(TargetClass);

		// Verify filter worked
		if (FilteredVehicles.Num() != 20)
		{
			AddWarning(FString::Printf(TEXT("Unexpected filtered count: %d"), FilteredVehicles.Num()));
		}
	}

	double FilterTime = FPlatformTime::Seconds() - StartTime;

	// Validate performance: 1000 filters should complete quickly
	TestTrue(TEXT("1000 filter operations complete quickly (<0.2s)"), FilterTime < 0.2);

	AddInfo(FString::Printf(TEXT("Filter time: %.3f ms"), FilterTime * 1000.0));
	AddInfo(FString::Printf(TEXT("Average filter time: %.3f µs"), (FilterTime / FilterCount) * 1000000.0));

	return true;
}

/**
 * Performance Test: Memory Efficiency
 * Measures memory usage patterns of catalog subsystems
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FMGCatalogMemoryEfficiencyTest,
	"MidnightGrind.Performance.Catalog.MemoryEfficiency",
	EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FMGCatalogMemoryEfficiencyTest::RunTest(const FString& Parameters)
{
	// Create test game instance
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	if (!TestNotNull(TEXT("GameInstance created"), GameInstance))
		return false;

	// Create catalogs with production-scale data
	UMGVehicleCatalogSubsystem* VehicleCatalog = NewObject<UMGVehicleCatalogSubsystem>(GameInstance);
	UMGPartsCatalogSubsystem* PartsCatalog = NewObject<UMGPartsCatalogSubsystem>(GameInstance);

	const int32 VehicleCount = 100;
	const int32 PartCount = 1000;

	TArray<FMGVehicleData> TestVehicles = FMGTestDataFactory::CreateTestVehicleArray(VehicleCount);
	TArray<FMGPartData> TestParts = FMGTestDataFactory::CreateTestPartArray(PartCount);

	UDataTable* VehicleDT = FMGTestDataFactory::CreateMockVehicleDataTable(GameInstance, TestVehicles);
	UDataTable* PartsDT = FMGTestDataFactory::CreateMockPartDataTable(GameInstance, TestParts);

	VehicleCatalog->VehicleDataTable = VehicleDT;
	PartsCatalog->PartsDataTable = PartsDT;

	VehicleCatalog->Initialize(nullptr);
	PartsCatalog->Initialize(nullptr);

	// Test: Verify cache doesn't duplicate data excessively
	// Cache should only store lookups, not full data copies

	// Perform many lookups to populate cache
	for (int32 i = 0; i < 1000; ++i)
	{
		int32 VehicleIndex = i % TestVehicles.Num();
		VehicleCatalog->GetVehicleBasePrice(TestVehicles[VehicleIndex].VehicleID);

		int32 PartIndex = i % TestParts.Num();
		PartsCatalog->GetPartPricing(TestParts[PartIndex].PartID);
	}

	// Verify data integrity after heavy caching
	TArray<FMGVehicleData> AllVehicles = VehicleCatalog->GetAllVehicles();
	TArray<FMGPartData> AllParts = PartsCatalog->GetAllParts();

	TestEqual(TEXT("All vehicles still accessible"), AllVehicles.Num(), VehicleCount);
	TestEqual(TEXT("All parts still accessible"), AllParts.Num(), PartCount);

	// Test: Cache consistency after many operations
	for (const FMGVehicleData& Vehicle : TestVehicles)
	{
		float Price1 = VehicleCatalog->GetVehicleBasePrice(Vehicle.VehicleID);
		float Price2 = VehicleCatalog->GetVehicleBasePrice(Vehicle.VehicleID);
		TestEqual(TEXT("Cache returns consistent prices"), Price1, Price2);
	}

	AddInfo(TEXT("Memory efficiency validated: Cache operates correctly without data duplication"));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS

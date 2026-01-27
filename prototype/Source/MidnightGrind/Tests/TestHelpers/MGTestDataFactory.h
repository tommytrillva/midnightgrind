// Copyright Midnight Grind. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Data/MGVehicleData.h"
#include "Catalog/MGPartData.h"

/**
 * Factory class for generating test data
 * Provides utilities for creating mock data tables and structures for unit tests
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

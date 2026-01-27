// Copyright Midnight Grind. All Rights Reserved.

#include "MGTestDataFactory.h"
#include "Engine/DataTable.h"

int32 FMGTestDataFactory::UniqueIDCounter = 0;

FMGVehicleData FMGTestDataFactory::CreateTestVehicle(FName VehicleID, FText DisplayName, float BasePrice, EMGVehicleClass VehicleClass)
{
	FMGVehicleData Vehicle;
	Vehicle.VehicleID = VehicleID;
	Vehicle.DisplayName = DisplayName;
	Vehicle.BasePrice = BasePrice;
	Vehicle.VehicleClass = VehicleClass;
	Vehicle.Manufacturer = FText::FromString(TEXT("Test Manufacturer"));
	Vehicle.Year = 2023;
	Vehicle.bIsUnlocked = true;

	// Set reasonable test stats
	Vehicle.BaseStats.TopSpeed = 180.0f;
	Vehicle.BaseStats.Acceleration = 7.5f;
	Vehicle.BaseStats.Handling = 7.0f;
	Vehicle.BaseStats.Braking = 8.0f;
	Vehicle.BaseStats.Weight = 1500.0f;

	return Vehicle;
}

TArray<FMGVehicleData> FMGTestDataFactory::CreateTestVehicleArray(int32 Count)
{
	TArray<FMGVehicleData> Vehicles;

	for (int32 i = 0; i < Count; ++i)
	{
		FName VehicleID = FName(*FString::Printf(TEXT("TestVehicle_%03d"), UniqueIDCounter++));
		FText DisplayName = FText::FromString(FString::Printf(TEXT("Test Vehicle %d"), i + 1));
		float BasePrice = 20000.0f + (i * 5000.0f);

		// Cycle through vehicle classes
		EMGVehicleClass VehicleClass = static_cast<EMGVehicleClass>(i % 6);

		Vehicles.Add(CreateTestVehicle(VehicleID, DisplayName, BasePrice, VehicleClass));
	}

	return Vehicles;
}

FMGPartData FMGTestDataFactory::CreateTestPart(FName PartID, FText DisplayName, EMGPartCategory Category, float BasePrice)
{
	FMGPartData Part;
	Part.PartID = PartID;
	Part.DisplayName = DisplayName;
	Part.Category = Category;
	Part.BaseCost = BasePrice;
	Part.LaborCost = BasePrice * 0.15f; // 15% of part cost for labor
	Part.InstallTimeMinutes = 60.0f; // 1 hour default
	Part.Manufacturer = FText::FromString(TEXT("Test Parts Inc."));
	Part.bRequiresSpecialist = false;

	// Set performance impact
	Part.PerformanceImpact.TopSpeed = 5.0f;
	Part.PerformanceImpact.Acceleration = 0.5f;
	Part.PerformanceImpact.Handling = 0.3f;
	Part.PerformanceImpact.Braking = 0.2f;

	return Part;
}

FMGPartData FMGTestDataFactory::CreateTestPartWithPricing(FName PartID, float BaseCost, float LaborCost, float InstallTimeMinutes)
{
	FMGPartData Part = CreateTestPart(PartID);
	Part.BaseCost = BaseCost;
	Part.LaborCost = LaborCost;
	Part.InstallTimeMinutes = InstallTimeMinutes;
	return Part;
}

FMGPartData FMGTestDataFactory::CreateTestPartWithCompatibility(FName PartID, TArray<FName> CompatibleVehicles, TArray<EMGVehicleClass> CompatibleClasses)
{
	FMGPartData Part = CreateTestPart(PartID);
	Part.CompatibleVehicles = CompatibleVehicles;
	Part.CompatibleVehicleClasses = CompatibleClasses;
	return Part;
}

TArray<FMGPartData> FMGTestDataFactory::CreateTestPartArray(int32 Count)
{
	TArray<FMGPartData> Parts;

	for (int32 i = 0; i < Count; ++i)
	{
		FName PartID = FName(*FString::Printf(TEXT("TestPart_%03d"), UniqueIDCounter++));
		FText DisplayName = FText::FromString(FString::Printf(TEXT("Test Part %d"), i + 1));
		float BasePrice = 500.0f + (i * 250.0f);

		// Cycle through part categories
		EMGPartCategory Category = static_cast<EMGPartCategory>(i % 8);

		Parts.Add(CreateTestPart(PartID, DisplayName, Category, BasePrice));
	}

	return Parts;
}

FMGPartPricingInfo FMGTestDataFactory::CreateTestPricingInfo(float BaseCost, float LaborCost, bool bIsValid)
{
	FMGPartPricingInfo PricingInfo;
	PricingInfo.BaseCost = BaseCost;
	PricingInfo.LaborCost = LaborCost;
	PricingInfo.TotalCost = BaseCost + LaborCost;
	PricingInfo.bIsValid = bIsValid;
	return PricingInfo;
}

TArray<FMGVehicleData> FMGTestDataFactory::CreateVehiclesByClass(EMGVehicleClass TargetClass, int32 Count)
{
	TArray<FMGVehicleData> Vehicles;

	for (int32 i = 0; i < Count; ++i)
	{
		FName VehicleID = FName(*FString::Printf(TEXT("TestVehicle_%s_%03d"),
			*UEnum::GetValueAsString(TargetClass), UniqueIDCounter++));
		FText DisplayName = FText::FromString(FString::Printf(TEXT("Test %s Vehicle %d"),
			*UEnum::GetValueAsString(TargetClass), i + 1));
		float BasePrice = 25000.0f + (i * 5000.0f);

		Vehicles.Add(CreateTestVehicle(VehicleID, DisplayName, BasePrice, TargetClass));
	}

	return Vehicles;
}

TArray<FMGPartData> FMGTestDataFactory::CreatePartsByCategory(EMGPartCategory TargetCategory, int32 Count)
{
	TArray<FMGPartData> Parts;

	for (int32 i = 0; i < Count; ++i)
	{
		FName PartID = FName(*FString::Printf(TEXT("TestPart_%s_%03d"),
			*UEnum::GetValueAsString(TargetCategory), UniqueIDCounter++));
		FText DisplayName = FText::FromString(FString::Printf(TEXT("Test %s Part %d"),
			*UEnum::GetValueAsString(TargetCategory), i + 1));
		float BasePrice = 1000.0f + (i * 500.0f);

		Parts.Add(CreateTestPart(PartID, DisplayName, TargetCategory, BasePrice));
	}

	return Parts;
}

UDataTable* FMGTestDataFactory::CreateMockVehicleDataTable(UObject* Outer, TArray<FMGVehicleData> Vehicles)
{
	UDataTable* DataTable = NewObject<UDataTable>(Outer);
	DataTable->RowStruct = FMGVehicleData::StaticStruct();

	for (const FMGVehicleData& Vehicle : Vehicles)
	{
		FMGVehicleData* NewRow = reinterpret_cast<FMGVehicleData*>(DataTable->FindOrAddRow(Vehicle.VehicleID, TEXT("Test")));
		*NewRow = Vehicle;
	}

	return DataTable;
}

UDataTable* FMGTestDataFactory::CreateMockPartDataTable(UObject* Outer, TArray<FMGPartData> Parts)
{
	UDataTable* DataTable = NewObject<UDataTable>(Outer);
	DataTable->RowStruct = FMGPartData::StaticStruct();

	for (const FMGPartData& Part : Parts)
	{
		FMGPartData* NewRow = reinterpret_cast<FMGPartData*>(DataTable->FindOrAddRow(Part.PartID, TEXT("Test")));
		*NewRow = Part;
	}

	return DataTable;
}

bool FMGTestDataFactory::ValidateVehicleData(const FMGVehicleData& Vehicle)
{
	if (Vehicle.VehicleID.IsNone())
		return false;

	if (Vehicle.DisplayName.IsEmpty())
		return false;

	if (Vehicle.BasePrice <= 0.0f)
		return false;

	if (Vehicle.BaseStats.TopSpeed <= 0.0f || Vehicle.BaseStats.Weight <= 0.0f)
		return false;

	return true;
}

bool FMGTestDataFactory::ValidatePartData(const FMGPartData& Part)
{
	if (Part.PartID.IsNone())
		return false;

	if (Part.DisplayName.IsEmpty())
		return false;

	if (Part.BaseCost < 0.0f || Part.LaborCost < 0.0f)
		return false;

	if (Part.InstallTimeMinutes < 0.0f)
		return false;

	return true;
}

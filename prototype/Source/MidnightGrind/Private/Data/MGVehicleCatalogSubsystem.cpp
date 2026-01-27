// Copyright Midnight Grind. All Rights Reserved.

#include "Data/MGVehicleCatalogSubsystem.h"
#include "Engine/DataTable.h"

// ==========================================
// SUBSYSTEM LIFECYCLE
// ==========================================

void UMGVehicleCatalogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Load DataTable if path is set
	if (!VehicleCatalogTable.IsNull())
	{
		LoadedCatalogTable = VehicleCatalogTable.LoadSynchronous();

		if (LoadedCatalogTable)
		{
			BuildCache();
			UE_LOG(LogTemp, Log, TEXT("Vehicle Catalog loaded: %d vehicles"), VehicleCache.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load Vehicle Catalog DataTable"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Vehicle Catalog DataTable path not set"));
	}
}

void UMGVehicleCatalogSubsystem::Deinitialize()
{
	VehicleCache.Empty();
	LoadedCatalogTable = nullptr;

	Super::Deinitialize();
}

// ==========================================
// PRICING LOOKUPS
// ==========================================

FMGVehiclePricingInfo UMGVehicleCatalogSubsystem::GetVehiclePricing(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = GetCatalogRow(VehicleID);

	if (Row)
	{
		return Row->Pricing;
	}

	// Return default pricing if not found
	FMGVehiclePricingInfo DefaultPricing;
	DefaultPricing.BasePurchasePrice = 25000;
	DefaultPricing.StreetValue = 30000;
	DefaultPricing.LegendaryValue = 60000;
	DefaultPricing.MaintenanceCostMultiplier = 1.0f;
	DefaultPricing.PartsPriceMultiplier = 1.0f;
	DefaultPricing.InsuranceClass = "C";

	return DefaultPricing;
}

int32 UMGVehicleCatalogSubsystem::GetBasePurchasePrice(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = GetCatalogRow(VehicleID);
	return Row ? Row->Pricing.BasePurchasePrice : 0;
}

int32 UMGVehicleCatalogSubsystem::GetStreetValue(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = GetCatalogRow(VehicleID);
	return Row ? Row->Pricing.StreetValue : 0;
}

// ==========================================
// PERFORMANCE LOOKUPS
// ==========================================

FMGVehiclePerformanceInfo UMGVehicleCatalogSubsystem::GetVehiclePerformance(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = GetCatalogRow(VehicleID);

	if (Row)
	{
		return Row->Performance;
	}

	// Return default performance if not found
	FMGVehiclePerformanceInfo DefaultPerf;
	DefaultPerf.BasePI = 420;
	DefaultPerf.PerformanceClass = "C";
	DefaultPerf.MaxPIPotential = 750;
	DefaultPerf.BaseHorsepower = 200;
	DefaultPerf.BaseTorque = 150;
	DefaultPerf.BaseWeight = 2800;
	DefaultPerf.Drivetrain = "RWD";

	return DefaultPerf;
}

FString UMGVehicleCatalogSubsystem::GetPerformanceClass(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = GetCatalogRow(VehicleID);
	return Row ? Row->Performance.PerformanceClass : FString("C");
}

// ==========================================
// CATALOG QUERIES
// ==========================================

bool UMGVehicleCatalogSubsystem::GetVehicleData(FName VehicleID, FMGVehicleCatalogRow& OutData) const
{
	const FMGVehicleCatalogRow* Row = GetCatalogRow(VehicleID);

	if (Row)
	{
		OutData = *Row;
		return true;
	}

	return false;
}

TArray<FMGVehicleCatalogRow> UMGVehicleCatalogSubsystem::GetVehiclesByClass(const FString& PerformanceClass) const
{
	TArray<FMGVehicleCatalogRow> Results;

	for (const auto& Pair : VehicleCache)
	{
		if (Pair.Value && Pair.Value->Performance.PerformanceClass == PerformanceClass)
		{
			Results.Add(*Pair.Value);
		}
	}

	return Results;
}

TArray<FMGVehicleCatalogRow> UMGVehicleCatalogSubsystem::GetVehiclesByTag(const FString& Tag) const
{
	TArray<FMGVehicleCatalogRow> Results;

	for (const auto& Pair : VehicleCache)
	{
		if (Pair.Value && Pair.Value->Tags.Contains(Tag))
		{
			Results.Add(*Pair.Value);
		}
	}

	return Results;
}

bool UMGVehicleCatalogSubsystem::IsVehicleInCatalog(FName VehicleID) const
{
	return VehicleCache.Contains(VehicleID);
}

TArray<FName> UMGVehicleCatalogSubsystem::GetAllVehicleIDs() const
{
	TArray<FName> VehicleIDs;
	VehicleCache.GetKeys(VehicleIDs);
	return VehicleIDs;
}

// ==========================================
// INTERNAL HELPERS
// ==========================================

void UMGVehicleCatalogSubsystem::BuildCache()
{
	VehicleCache.Empty();

	if (!LoadedCatalogTable)
	{
		return;
	}

	// Get all row names from DataTable
	TArray<FName> RowNames = LoadedCatalogTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		// Get row data
		FMGVehicleCatalogRow* Row = LoadedCatalogTable->FindRow<FMGVehicleCatalogRow>(RowName, TEXT("VehicleCatalogCache"));

		if (Row)
		{
			// Cache using VehicleID from row data
			FName VehicleID = FName(*Row->VehicleID);
			VehicleCache.Add(VehicleID, Row);
		}
	}
}

const FMGVehicleCatalogRow* UMGVehicleCatalogSubsystem::GetCatalogRow(FName VehicleID) const
{
	if (FMGVehicleCatalogRow* const* RowPtr = VehicleCache.Find(VehicleID))
	{
		return *RowPtr;
	}

	return nullptr;
}

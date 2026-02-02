// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGVehicleCatalogSubsystem.cpp
 * @brief Implementation of the Vehicle Catalog Subsystem for vehicle data access,
 *        pricing lookups, and filtering operations.
 */

#include "Catalog/MGVehicleCatalogSubsystem.h"
#include "Engine/DataTable.h"

// Initialize static member
const FMGVehiclePricingInfo UMGVehicleCatalogSubsystem::InvalidPricingInfo = FMGVehiclePricingInfo();

void UMGVehicleCatalogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Attempt to load DataTable if soft reference is set
	if (!VehicleCatalogTableRef.IsNull())
	{
		VehicleCatalogTable = VehicleCatalogTableRef.LoadSynchronous();
		if (VehicleCatalogTable)
		{
			BuildCache();
			UE_LOG(LogTemp, Log, TEXT("MGVehicleCatalogSubsystem: Loaded %d vehicles from catalog"), VehicleCache.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MGVehicleCatalogSubsystem: Failed to load VehicleCatalogTable"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MGVehicleCatalogSubsystem: No VehicleCatalogTable configured. Set VehicleCatalogTableRef in Blueprint."));
	}
}

void UMGVehicleCatalogSubsystem::Deinitialize()
{
	ClearCache();
	Super::Deinitialize();
}

void UMGVehicleCatalogSubsystem::BuildCache()
{
	ClearCache();

	if (!VehicleCatalogTable)
	{
		return;
	}

	// Iterate all rows and build cache
	const FString ContextString(TEXT("VehicleCatalogBuildCache"));
	TArray<FName> RowNames = VehicleCatalogTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FMGVehicleCatalogRow* Row = VehicleCatalogTable->FindRow<FMGVehicleCatalogRow>(RowName, ContextString);
		if (Row)
		{
			// Use VehicleID as the key if set, otherwise use row name
			FName CacheKey = Row->VehicleID.IsNone() ? RowName : Row->VehicleID;
			VehicleCache.Add(CacheKey, *Row);
		}
	}

	bCacheBuilt = true;
}

void UMGVehicleCatalogSubsystem::ClearCache()
{
	VehicleCache.Empty();
	bCacheBuilt = false;
}

// ========== Pricing Lookups ==========

FMGVehiclePricingInfo UMGVehicleCatalogSubsystem::GetVehiclePricing(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = VehicleCache.Find(VehicleID);
	if (Row)
	{
		FMGVehiclePricingInfo Info;
		Info.BasePurchasePrice = Row->Economy.BasePurchasePrice;
		Info.StreetValue = Row->Economy.StreetValue;
		Info.LegendaryValue = Row->Economy.LegendaryValue;
		Info.MaintenanceCostMultiplier = Row->Economy.MaintenanceCostMultiplier;
		Info.PartsPriceMultiplier = Row->Economy.PartsPriceMultiplier;
		Info.bIsValid = true;
		return Info;
	}

	// Return invalid info with default values
	return InvalidPricingInfo;
}

int32 UMGVehicleCatalogSubsystem::GetEstimatedValue(FName VehicleID, float Condition) const
{
	const FMGVehicleCatalogRow* Row = VehicleCache.Find(VehicleID);
	if (Row)
	{
		// Clamp condition to valid range
		Condition = FMath::Clamp(Condition, 0.5f, 1.0f);

		// Base value is street value adjusted by condition
		int32 BaseValue = Row->Economy.StreetValue;
		return FMath::RoundToInt(BaseValue * Condition);
	}

	return -1;
}

int32 UMGVehicleCatalogSubsystem::GetBasePurchasePrice(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = VehicleCache.Find(VehicleID);
	if (Row)
	{
		return Row->Economy.BasePurchasePrice;
	}

	return -1;
}

// ========== Data Lookups ==========

bool UMGVehicleCatalogSubsystem::GetVehicleData(FName VehicleID, FMGVehicleCatalogRow& OutData) const
{
	const FMGVehicleCatalogRow* Row = VehicleCache.Find(VehicleID);
	if (Row)
	{
		OutData = *Row;
		return true;
	}

	return false;
}

bool UMGVehicleCatalogSubsystem::VehicleExists(FName VehicleID) const
{
	return VehicleCache.Contains(VehicleID);
}

FText UMGVehicleCatalogSubsystem::GetVehicleDisplayName(FName VehicleID) const
{
	const FMGVehicleCatalogRow* Row = VehicleCache.Find(VehicleID);
	if (Row)
	{
		return Row->DisplayName;
	}

	return FText::FromString(TEXT("Unknown Vehicle"));
}

// ========== Filtering ==========

TArray<FMGVehicleCatalogRow> UMGVehicleCatalogSubsystem::GetVehiclesByClass(EMGPerformanceClass PerformanceClass) const
{
	TArray<FMGVehicleCatalogRow> Results;

	for (const auto& Pair : VehicleCache)
	{
		if (Pair.Value.PerformanceIndex.Class == PerformanceClass)
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

TArray<FMGVehicleCatalogRow> UMGVehicleCatalogSubsystem::GetVehiclesByCategory(EMGVehicleCategory Category) const
{
	TArray<FMGVehicleCatalogRow> Results;

	for (const auto& Pair : VehicleCache)
	{
		if (Pair.Value.Category == Category)
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

TArray<FMGVehicleCatalogRow> UMGVehicleCatalogSubsystem::GetVehiclesInPriceRange(int32 MinPrice, int32 MaxPrice) const
{
	TArray<FMGVehicleCatalogRow> Results;

	for (const auto& Pair : VehicleCache)
	{
		int32 Price = Pair.Value.Economy.BasePurchasePrice;
		if (Price >= MinPrice && Price <= MaxPrice)
		{
			Results.Add(Pair.Value);
		}
	}

	// Sort by price ascending
	Results.Sort([](const FMGVehicleCatalogRow& A, const FMGVehicleCatalogRow& B) {
		return A.Economy.BasePurchasePrice < B.Economy.BasePurchasePrice;
	});

	return Results;
}

TArray<FMGVehicleCatalogRow> UMGVehicleCatalogSubsystem::GetVehiclesByTag(const FString& Tag) const
{
	TArray<FMGVehicleCatalogRow> Results;

	for (const auto& Pair : VehicleCache)
	{
		if (Pair.Value.Tags.Contains(Tag))
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

TArray<FName> UMGVehicleCatalogSubsystem::GetAllVehicleIDs() const
{
	TArray<FName> IDs;
	VehicleCache.GetKeys(IDs);
	return IDs;
}

int32 UMGVehicleCatalogSubsystem::GetVehicleCount() const
{
	return VehicleCache.Num();
}

// ========== Validation ==========

bool UMGVehicleCatalogSubsystem::IsCatalogLoaded() const
{
	return bCacheBuilt && VehicleCache.Num() > 0;
}

void UMGVehicleCatalogSubsystem::ReloadCatalog()
{
	// Reload the DataTable if needed
	if (!VehicleCatalogTableRef.IsNull())
	{
		VehicleCatalogTable = VehicleCatalogTableRef.LoadSynchronous();
	}

	// Rebuild cache
	if (VehicleCatalogTable)
	{
		BuildCache();
		UE_LOG(LogTemp, Log, TEXT("MGVehicleCatalogSubsystem: Reloaded catalog with %d vehicles"), VehicleCache.Num());
	}
}

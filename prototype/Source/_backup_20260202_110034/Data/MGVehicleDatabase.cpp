// Copyright Midnight Grind. All Rights Reserved.

#include "Data/MGVehicleDatabase.h"

// ==========================================
// UMGVehicleDatabase Implementation
// ==========================================

UMGVehicleDefinition* UMGVehicleDatabase::GetVehicleByID(FName VehicleID) const
{
	for (const TSoftObjectPtr<UMGVehicleDefinition>& VehiclePtr : AllVehicles)
	{
		if (UMGVehicleDefinition* Vehicle = VehiclePtr.LoadSynchronous())
		{
			if (Vehicle->VehicleID == VehicleID)
			{
				return Vehicle;
			}
		}
	}
	return nullptr;
}

TArray<UMGVehicleDefinition*> UMGVehicleDatabase::GetVehiclesByMake(EMGVehicleMake Make) const
{
	TArray<UMGVehicleDefinition*> Result;

	for (const TSoftObjectPtr<UMGVehicleDefinition>& VehiclePtr : AllVehicles)
	{
		if (UMGVehicleDefinition* Vehicle = VehiclePtr.LoadSynchronous())
		{
			if (Vehicle->Make == Make)
			{
				Result.Add(Vehicle);
			}
		}
	}

	return Result;
}

TArray<UMGVehicleDefinition*> UMGVehicleDatabase::GetVehiclesByEra(EMGVehicleEra Era) const
{
	TArray<UMGVehicleDefinition*> Result;

	for (const TSoftObjectPtr<UMGVehicleDefinition>& VehiclePtr : AllVehicles)
	{
		if (UMGVehicleDefinition* Vehicle = VehiclePtr.LoadSynchronous())
		{
			if (Vehicle->Era == Era)
			{
				Result.Add(Vehicle);
			}
		}
	}

	return Result;
}

TArray<UMGVehicleDefinition*> UMGVehicleDatabase::GetStarterVehicles() const
{
	TArray<UMGVehicleDefinition*> Result;

	for (const TSoftObjectPtr<UMGVehicleDefinition>& VehiclePtr : AllVehicles)
	{
		if (UMGVehicleDefinition* Vehicle = VehiclePtr.LoadSynchronous())
		{
			if (Vehicle->bIsStarterVehicle)
			{
				Result.Add(Vehicle);
			}
		}
	}

	return Result;
}

TArray<UMGVehicleDefinition*> UMGVehicleDatabase::GetVehiclesInPriceRange(int64 MinPrice, int64 MaxPrice) const
{
	TArray<UMGVehicleDefinition*> Result;

	for (const TSoftObjectPtr<UMGVehicleDefinition>& VehiclePtr : AllVehicles)
	{
		if (UMGVehicleDefinition* Vehicle = VehiclePtr.LoadSynchronous())
		{
			if (Vehicle->PurchasePrice >= MinPrice && Vehicle->PurchasePrice <= MaxPrice)
			{
				Result.Add(Vehicle);
			}
		}
	}

	// Sort by price ascending
	Result.Sort([](const UMGVehicleDefinition& A, const UMGVehicleDefinition& B)
	{
		return A.PurchasePrice < B.PurchasePrice;
	});

	return Result;
}

TArray<UMGVehicleDefinition*> UMGVehicleDatabase::GetVehiclesInPIRange(int32 MinPI, int32 MaxPI) const
{
	TArray<UMGVehicleDefinition*> Result;

	for (const TSoftObjectPtr<UMGVehicleDefinition>& VehiclePtr : AllVehicles)
	{
		if (UMGVehicleDefinition* Vehicle = VehiclePtr.LoadSynchronous())
		{
			if (Vehicle->BasePI >= MinPI && Vehicle->BasePI <= MaxPI)
			{
				Result.Add(Vehicle);
			}
		}
	}

	// Sort by PI ascending
	Result.Sort([](const UMGVehicleDefinition& A, const UMGVehicleDefinition& B)
	{
		return A.BasePI < B.BasePI;
	});

	return Result;
}

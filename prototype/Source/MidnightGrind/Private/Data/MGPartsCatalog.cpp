// Copyright Midnight Grind. All Rights Reserved.

#include "Data/MGPartsCatalog.h"

// ==========================================
// UMGPartDefinition Implementation
// ==========================================

bool UMGPartDefinition::IsCompatibleWithVehicle(const UMGVehicleDefinition* Vehicle) const
{
	if (!Vehicle)
	{
		return false;
	}

	// Check specific vehicle ID compatibility
	if (CompatibleVehicleIDs.Num() > 0)
	{
		if (!CompatibleVehicleIDs.Contains(Vehicle->VehicleID))
		{
			return false;
		}
	}

	// Check make compatibility
	if (CompatibleMakes.Num() > 0)
	{
		if (!CompatibleMakes.Contains(Vehicle->Make))
		{
			return false;
		}
	}

	// Check engine compatibility
	if (CompatibleEngines.Num() > 0)
	{
		if (!CompatibleEngines.Contains(Vehicle->Engine.Configuration))
		{
			return false;
		}
	}

	return true;
}

// ==========================================
// UMGPartsCatalog Implementation
// ==========================================

UMGPartDefinition* UMGPartsCatalog::GetPartByID(FName PartID) const
{
	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->PartID == PartID)
			{
				return Part;
			}
		}
	}
	return nullptr;
}

TArray<UMGPartDefinition*> UMGPartsCatalog::GetPartsByCategory(EMGPartCategory Category) const
{
	TArray<UMGPartDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->Category == Category)
			{
				Result.Add(Part);
			}
		}
	}

	// Sort by tier then price
	Result.Sort([](const UMGPartDefinition& A, const UMGPartDefinition& B)
	{
		if (A.Tier != B.Tier)
		{
			return static_cast<uint8>(A.Tier) < static_cast<uint8>(B.Tier);
		}
		return A.PurchasePrice < B.PurchasePrice;
	});

	return Result;
}

TArray<UMGPartDefinition*> UMGPartsCatalog::GetPartsByTier(EMGPartTier Tier) const
{
	TArray<UMGPartDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->Tier == Tier)
			{
				Result.Add(Part);
			}
		}
	}

	return Result;
}

TArray<UMGPartDefinition*> UMGPartsCatalog::GetPartsByBrand(EMGPartBrand Brand) const
{
	TArray<UMGPartDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->Brand == Brand)
			{
				Result.Add(Part);
			}
		}
	}

	return Result;
}

TArray<UMGPartDefinition*> UMGPartsCatalog::GetPartsForVehicle(const UMGVehicleDefinition* Vehicle) const
{
	TArray<UMGPartDefinition*> Result;

	if (!Vehicle)
	{
		return Result;
	}

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->IsCompatibleWithVehicle(Vehicle))
			{
				Result.Add(Part);
			}
		}
	}

	return Result;
}

TArray<UMGPartDefinition*> UMGPartsCatalog::GetPartsInPriceRange(int64 MinPrice, int64 MaxPrice) const
{
	TArray<UMGPartDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->PurchasePrice >= MinPrice && Part->PurchasePrice <= MaxPrice)
			{
				Result.Add(Part);
			}
		}
	}

	// Sort by price
	Result.Sort([](const UMGPartDefinition& A, const UMGPartDefinition& B)
	{
		return A.PurchasePrice < B.PurchasePrice;
	});

	return Result;
}

TArray<UMGPartDefinition*> UMGPartsCatalog::GetUpgradesForCategory(const UMGVehicleDefinition* Vehicle, EMGPartCategory Category) const
{
	TArray<UMGPartDefinition*> Result;

	if (!Vehicle)
	{
		return Result;
	}

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (Part->Category == Category && Part->IsCompatibleWithVehicle(Vehicle))
			{
				Result.Add(Part);
			}
		}
	}

	// Sort by tier then price
	Result.Sort([](const UMGPartDefinition& A, const UMGPartDefinition& B)
	{
		if (A.Tier != B.Tier)
		{
			return static_cast<uint8>(A.Tier) < static_cast<uint8>(B.Tier);
		}
		return A.PurchasePrice < B.PurchasePrice;
	});

	return Result;
}

TArray<UMGWheelDefinition*> UMGPartsCatalog::GetAllWheels() const
{
	TArray<UMGWheelDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (UMGWheelDefinition* Wheel = Cast<UMGWheelDefinition>(Part))
			{
				Result.Add(Wheel);
			}
		}
	}

	return Result;
}

TArray<UMGTireDefinition*> UMGPartsCatalog::GetAllTires() const
{
	TArray<UMGTireDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (UMGTireDefinition* Tire = Cast<UMGTireDefinition>(Part))
			{
				Result.Add(Tire);
			}
		}
	}

	return Result;
}

TArray<UMGTurboDefinition*> UMGPartsCatalog::GetAllTurbos() const
{
	TArray<UMGTurboDefinition*> Result;

	for (const TSoftObjectPtr<UMGPartDefinition>& PartPtr : AllParts)
	{
		if (UMGPartDefinition* Part = PartPtr.LoadSynchronous())
		{
			if (UMGTurboDefinition* Turbo = Cast<UMGTurboDefinition>(Part))
			{
				Result.Add(Turbo);
			}
		}
	}

	return Result;
}

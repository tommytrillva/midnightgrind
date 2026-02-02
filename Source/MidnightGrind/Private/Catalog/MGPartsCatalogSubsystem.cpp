// Copyright Midnight Grind. All Rights Reserved.

/**
 * @file MGPartsCatalogSubsystem.cpp
 * @brief Implementation of the Parts Catalog Subsystem for aftermarket parts data access,
 *        pricing lookups, and mechanic system integration.
 */

#include "Catalog/MGPartsCatalogSubsystem.h"
#include "Engine/DataTable.h"

// Initialize static members
const FMGPartPricingInfo UMGPartsCatalogSubsystem::InvalidPricingInfo = FMGPartPricingInfo();
const FMGPartSpecializationInfo UMGPartsCatalogSubsystem::InvalidSpecializationInfo = FMGPartSpecializationInfo();

void UMGPartsCatalogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Attempt to load DataTable if soft reference is set
	if (!PartsCatalogTableRef.IsNull())
	{
		PartsCatalogTable = PartsCatalogTableRef.LoadSynchronous();
		if (PartsCatalogTable)
		{
			BuildCache();
			UE_LOG(LogTemp, Log, TEXT("MGPartsCatalogSubsystem: Loaded %d parts from catalog"), PartCache.Num());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MGPartsCatalogSubsystem: Failed to load PartsCatalogTable"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("MGPartsCatalogSubsystem: No PartsCatalogTable configured. Set PartsCatalogTableRef in Blueprint."));
	}
}

void UMGPartsCatalogSubsystem::Deinitialize()
{
	ClearCache();
	Super::Deinitialize();
}

void UMGPartsCatalogSubsystem::BuildCache()
{
	ClearCache();

	if (!PartsCatalogTable)
	{
		return;
	}

	// Iterate all rows and build caches
	const FString ContextString(TEXT("PartsCatalogBuildCache"));
	TArray<FName> RowNames = PartsCatalogTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		FMGPartCatalogRow* Row = PartsCatalogTable->FindRow<FMGPartCatalogRow>(RowName, ContextString);
		if (Row)
		{
			// Use PartID as the key if set, otherwise use row name
			FName CacheKey = Row->PartID.IsNone() ? RowName : Row->PartID;
			PartCache.Add(CacheKey, *Row);

			// Build category index
			TArray<FName>& CategoryParts = PartsByCategory.FindOrAdd(Row->Category);
			CategoryParts.Add(CacheKey);

			// Build vehicle compatibility index
			if (Row->CompatibleVehicles.Num() == 0)
			{
				// Universal part - add to special "ALL" key
				TArray<FName>& UniversalParts = PartsByVehicle.FindOrAdd(FName("UNIVERSAL"));
				UniversalParts.Add(CacheKey);
			}
			else
			{
				for (const FName& VehicleID : Row->CompatibleVehicles)
				{
					TArray<FName>& VehicleParts = PartsByVehicle.FindOrAdd(VehicleID);
					VehicleParts.Add(CacheKey);
				}
			}
		}
	}

	bCacheBuilt = true;
}

void UMGPartsCatalogSubsystem::ClearCache()
{
	PartCache.Empty();
	PartsByCategory.Empty();
	PartsByVehicle.Empty();
	bCacheBuilt = false;
}

// ========== Pricing Lookups ==========

FMGPartPricingInfo UMGPartsCatalogSubsystem::GetPartPricing(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		FMGPartPricingInfo Info;
		Info.BasePrice = Row->BasePrice;
		Info.LaborCost = Row->LaborCost;
		Info.InstallTime = Row->InstallTime;
		Info.bIsValid = true;
		return Info;
	}

	return InvalidPricingInfo;
}

int32 UMGPartsCatalogSubsystem::GetPartBasePrice(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->BasePrice;
	}
	return -1;
}

int32 UMGPartsCatalogSubsystem::GetPartTotalCost(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->BasePrice + Row->LaborCost;
	}
	return -1;
}

int32 UMGPartsCatalogSubsystem::GetAdjustedPartPrice(FName PartID, float VehiclePriceMultiplier) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return FMath::RoundToInt(Row->BasePrice * VehiclePriceMultiplier);
	}
	return -1;
}

// ========== Specialization Lookups ==========

FMGPartSpecializationInfo UMGPartsCatalogSubsystem::GetPartSpecialization(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		FMGPartSpecializationInfo Info;
		Info.Category = Row->Category;
		Info.SubCategory = Row->SubCategory;
		Info.RequiredSkillLevel = Row->RequiredSkillLevel;
		Info.InstallTime = Row->InstallTime;
		Info.bIsValid = true;
		return Info;
	}

	return InvalidSpecializationInfo;
}

EMGPartCategory UMGPartsCatalogSubsystem::GetPartCategory(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->Category;
	}
	return EMGPartCategory::Engine; // Default
}

FString UMGPartsCatalogSubsystem::GetPartSubCategory(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->SubCategory;
	}
	return FString();
}

bool UMGPartsCatalogSubsystem::DoesSpecializationMatchPart(EMGPartCategory MechanicSpecialization, FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->Category == MechanicSpecialization;
	}
	return false;
}

float UMGPartsCatalogSubsystem::GetPartInstallTime(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->InstallTime;
	}
	return 60.0f; // Default: 1 hour
}

int32 UMGPartsCatalogSubsystem::GetPartRequiredSkillLevel(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->RequiredSkillLevel;
	}
	return 1; // Default: minimum skill
}

// ========== Data Lookups ==========

bool UMGPartsCatalogSubsystem::GetPartData(FName PartID, FMGPartCatalogRow& OutData) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		OutData = *Row;
		return true;
	}
	return false;
}

bool UMGPartsCatalogSubsystem::PartExists(FName PartID) const
{
	return PartCache.Contains(PartID);
}

FText UMGPartsCatalogSubsystem::GetPartDisplayName(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->DisplayName;
	}
	return FText::FromString(TEXT("Unknown Part"));
}

EMGPartTier UMGPartsCatalogSubsystem::GetPartTier(FName PartID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		return Row->Tier;
	}
	return EMGPartTier::Stock; // Default
}

// ========== Compatibility Checks ==========

bool UMGPartsCatalogSubsystem::IsPartCompatibleWithVehicle(FName PartID, FName VehicleID) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		// Empty compatible list = universal part
		if (Row->CompatibleVehicles.Num() == 0)
		{
			return true;
		}

		return Row->CompatibleVehicles.Contains(VehicleID);
	}
	return false;
}

TArray<FMGPartCatalogRow> UMGPartsCatalogSubsystem::GetPartsForVehicle(FName VehicleID) const
{
	TArray<FMGPartCatalogRow> Results;

	// Get vehicle-specific parts
	const TArray<FName>* VehicleParts = PartsByVehicle.Find(VehicleID);
	if (VehicleParts)
	{
		for (const FName& PartID : *VehicleParts)
		{
			const FMGPartCatalogRow* Row = PartCache.Find(PartID);
			if (Row)
			{
				Results.Add(*Row);
			}
		}
	}

	// Also add universal parts
	const TArray<FName>* UniversalParts = PartsByVehicle.Find(FName("UNIVERSAL"));
	if (UniversalParts)
	{
		for (const FName& PartID : *UniversalParts)
		{
			const FMGPartCatalogRow* Row = PartCache.Find(PartID);
			if (Row)
			{
				Results.Add(*Row);
			}
		}
	}

	return Results;
}

bool UMGPartsCatalogSubsystem::ArePrerequisitesMet(FName PartID, const TArray<FName>& InstalledParts) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		// Check all required parts are installed
		for (const FName& RequiredPart : Row->RequiredParts)
		{
			if (!InstalledParts.Contains(RequiredPart))
			{
				return false;
			}
		}
		return true;
	}
	return true; // Part not found, assume no requirements
}

bool UMGPartsCatalogSubsystem::HasConflictingParts(FName PartID, const TArray<FName>& InstalledParts) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		for (const FName& ConflictPart : Row->ConflictingParts)
		{
			if (InstalledParts.Contains(ConflictPart))
			{
				return true;
			}
		}
	}
	return false;
}

// ========== Filtering ==========

TArray<FMGPartCatalogRow> UMGPartsCatalogSubsystem::GetPartsByCategory(EMGPartCategory Category) const
{
	TArray<FMGPartCatalogRow> Results;

	const TArray<FName>* CategoryParts = PartsByCategory.Find(Category);
	if (CategoryParts)
	{
		for (const FName& PartID : *CategoryParts)
		{
			const FMGPartCatalogRow* Row = PartCache.Find(PartID);
			if (Row)
			{
				Results.Add(*Row);
			}
		}
	}

	return Results;
}

TArray<FMGPartCatalogRow> UMGPartsCatalogSubsystem::GetPartsByTier(EMGPartTier Tier) const
{
	TArray<FMGPartCatalogRow> Results;

	for (const auto& Pair : PartCache)
	{
		if (Pair.Value.Tier == Tier)
		{
			Results.Add(Pair.Value);
		}
	}

	return Results;
}

TArray<FMGPartCatalogRow> UMGPartsCatalogSubsystem::GetPartsInPriceRange(int32 MinPrice, int32 MaxPrice) const
{
	TArray<FMGPartCatalogRow> Results;

	for (const auto& Pair : PartCache)
	{
		int32 Price = Pair.Value.BasePrice;
		if (Price >= MinPrice && Price <= MaxPrice)
		{
			Results.Add(Pair.Value);
		}
	}

	// Sort by price ascending
	Results.Sort([](const FMGPartCatalogRow& A, const FMGPartCatalogRow& B) {
		return A.BasePrice < B.BasePrice;
	});

	return Results;
}

TArray<FName> UMGPartsCatalogSubsystem::GetAllPartIDs() const
{
	TArray<FName> IDs;
	PartCache.GetKeys(IDs);
	return IDs;
}

int32 UMGPartsCatalogSubsystem::GetPartCount() const
{
	return PartCache.Num();
}

// ========== Performance Stats ==========

bool UMGPartsCatalogSubsystem::GetPartPerformanceStats(FName PartID, int32& OutPowerBonus, int32& OutTorqueBonus,
	int32& OutWeightChange, int32& OutPIChange) const
{
	const FMGPartCatalogRow* Row = PartCache.Find(PartID);
	if (Row)
	{
		OutPowerBonus = Row->PowerBonus;
		OutTorqueBonus = Row->TorqueBonus;
		OutWeightChange = Row->WeightChange;
		OutPIChange = Row->PIChange;
		return true;
	}

	OutPowerBonus = 0;
	OutTorqueBonus = 0;
	OutWeightChange = 0;
	OutPIChange = 0;
	return false;
}

// ========== Validation ==========

bool UMGPartsCatalogSubsystem::IsCatalogLoaded() const
{
	return bCacheBuilt && PartCache.Num() > 0;
}

void UMGPartsCatalogSubsystem::ReloadCatalog()
{
	// Reload the DataTable if needed
	if (!PartsCatalogTableRef.IsNull())
	{
		PartsCatalogTable = PartsCatalogTableRef.LoadSynchronous();
	}

	// Rebuild cache
	if (PartsCatalogTable)
	{
		BuildCache();
		UE_LOG(LogTemp, Log, TEXT("MGPartsCatalogSubsystem: Reloaded catalog with %d parts"), PartCache.Num());
	}
}

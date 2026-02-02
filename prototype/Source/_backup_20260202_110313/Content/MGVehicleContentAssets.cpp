// Copyright Midnight Grind. All Rights Reserved.

#include "Content/MGVehicleContentAssets.h"

FPrimaryAssetId UMGVehicleDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("Vehicle"), VehicleID);
}

FText UMGVehicleDataAsset::GetFormattedSpecs() const
{
	FString SpecString = FString::Printf(
		TEXT("%s\n%.1fL %s%s%s\n%d HP / %d Nm\n0-100: %.1fs | Top: %d KPH"),
		*Engine.EngineName.ToString(),
		Engine.Displacement,
		Engine.Cylinders == 4 ? TEXT("I4") :
		Engine.Cylinders == 6 ? TEXT("V6") :
		Engine.Cylinders == 8 ? TEXT("V8") :
		Engine.Cylinders == 10 ? TEXT("V10") :
		Engine.Cylinders == 12 ? TEXT("V12") : TEXT(""),
		Engine.bTurbocharged ? TEXT(" Turbo") : TEXT(""),
		Engine.bSupercharged ? TEXT(" SC") : TEXT(""),
		FMath::RoundToInt(Engine.Horsepower),
		FMath::RoundToInt(Engine.Torque),
		ZeroToHundredTime,
		FMath::RoundToInt(TopSpeedKPH)
	);

	return FText::FromString(SpecString);
}

FText UMGVehicleDataAsset::GetClassDisplayName() const
{
	switch (VehicleClass)
	{
		case EMGVehicleClass::D_Class: return FText::FromString(TEXT("D Class"));
		case EMGVehicleClass::C_Class: return FText::FromString(TEXT("C Class"));
		case EMGVehicleClass::B_Class: return FText::FromString(TEXT("B Class"));
		case EMGVehicleClass::A_Class: return FText::FromString(TEXT("A Class"));
		case EMGVehicleClass::S_Class: return FText::FromString(TEXT("S Class"));
		case EMGVehicleClass::S_Plus: return FText::FromString(TEXT("S+ Class"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

FPrimaryAssetId UMGVehicleCollectionAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType("VehicleCollection"), CollectionID);
}

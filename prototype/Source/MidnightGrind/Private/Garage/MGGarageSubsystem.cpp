// Copyright Midnight Grind. All Rights Reserved.
// Updated Stage 52: MVP Game Entry Points - Starter Vehicle

#include "Garage/MGGarageSubsystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleData.h"
#include "Vehicle/MGStatCalculator.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"

void UMGGarageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Log, TEXT("MGGarageSubsystem initialized"));
}

void UMGGarageSubsystem::Deinitialize()
{
	CachedStats.Empty();
	Super::Deinitialize();
}

// ==========================================
// VEHICLE COLLECTION
// ==========================================

FMGGarageResult UMGGarageSubsystem::AddVehicle(UMGVehicleModelData* VehicleModelData, FGuid& OutVehicleId)
{
	if (!VehicleModelData)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "InvalidVehicle", "Invalid vehicle data"));
	}

	FMGOwnedVehicle NewVehicle;
	NewVehicle.VehicleModelData = VehicleModelData;
	NewVehicle.CustomName = VehicleModelData->DisplayName.ToString();
	NewVehicle.TotalInvestment = VehicleModelData->BasePriceMSRP;

	// Set default paint colors
	NewVehicle.Paint.PrimaryColor = FLinearColor::White;
	NewVehicle.Paint.SecondaryColor = FLinearColor::Black;
	NewVehicle.Paint.FinishType = EMGPaintFinish::Metallic;

	OutVehicleId = NewVehicle.VehicleId;
	OwnedVehicles.Add(NewVehicle);

	// Calculate initial stats
	RecalculateVehicleStats(OutVehicleId);

	// If this is the first vehicle, select it
	if (OwnedVehicles.Num() == 1)
	{
		SelectVehicle(OutVehicleId);
	}

	OnVehicleAdded.Broadcast(OutVehicleId);
	return FMGGarageResult::Success(VehicleModelData->BasePriceMSRP);
}

FMGGarageResult UMGGarageSubsystem::AddVehicleByID(FName VehicleID, FGuid& OutVehicleId)
{
	// MVP: Create placeholder vehicle data based on ID
	// In full implementation, would load from data asset

	FMGOwnedVehicle NewVehicle;
	NewVehicle.VehicleId = FGuid::NewGuid();
	OutVehicleId = NewVehicle.VehicleId;

	// Set name based on ID
	FString DisplayName = VehicleID.ToString();
	DisplayName.RemoveFromStart(TEXT("Vehicle_"));
	NewVehicle.CustomName = DisplayName;

	// Default paint colors
	NewVehicle.Paint.PrimaryColor = FLinearColor::White;
	NewVehicle.Paint.SecondaryColor = FLinearColor::Black;
	NewVehicle.Paint.FinishType = EMGPaintFinish::Metallic;

	// Set base investment (placeholder values based on vehicle tier)
	if (VehicleID.ToString().Contains(TEXT("240SX")) ||
		VehicleID.ToString().Contains(TEXT("Civic")) ||
		VehicleID.ToString().Contains(TEXT("MX5")))
	{
		NewVehicle.TotalInvestment = 15000;
	}
	else if (VehicleID.ToString().Contains(TEXT("Supra")) ||
			 VehicleID.ToString().Contains(TEXT("RX7")) ||
			 VehicleID.ToString().Contains(TEXT("Skyline")))
	{
		NewVehicle.TotalInvestment = 45000;
	}
	else
	{
		NewVehicle.TotalInvestment = 25000;
	}

	OwnedVehicles.Add(NewVehicle);

	// If this is the first vehicle, select it
	if (OwnedVehicles.Num() == 1)
	{
		SelectVehicle(OutVehicleId);
	}

	OnVehicleAdded.Broadcast(OutVehicleId);
	UE_LOG(LogTemp, Log, TEXT("Added vehicle by ID: %s (GUID: %s)"), *VehicleID.ToString(), *OutVehicleId.ToString());

	return FMGGarageResult::Success();
}

void UMGGarageSubsystem::EnsureStarterVehicle()
{
	// If player already has vehicles, do nothing
	if (OwnedVehicles.Num() > 0)
	{
		return;
	}

	// MVP: Give player a Nissan 240SX as starter
	FGuid StarterVehicleId;
	FMGGarageResult Result = AddVehicleByID(FName("Vehicle_240SX"), StarterVehicleId);

	if (Result.bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Starter vehicle added: Vehicle_240SX"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add starter vehicle"));
	}
}

FMGGarageResult UMGGarageSubsystem::RemoveVehicle(const FGuid& VehicleId)
{
	int32 Index = FindVehicleIndex(VehicleId);
	if (Index == INDEX_NONE)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	// Can't remove if it's the only vehicle
	if (OwnedVehicles.Num() == 1)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "LastVehicle", "Cannot remove your last vehicle"));
	}

	bool bWasSelected = OwnedVehicles[Index].bIsSelected;
	OwnedVehicles.RemoveAt(Index);
	CachedStats.Remove(VehicleId);

	// Select another vehicle if needed
	if (bWasSelected && OwnedVehicles.Num() > 0)
	{
		SelectVehicle(OwnedVehicles[0].VehicleId);
	}

	OnVehicleRemoved.Broadcast(VehicleId);
	return FMGGarageResult::Success();
}

FMGGarageResult UMGGarageSubsystem::SellVehicle(const FGuid& VehicleId, int64& OutSellPrice)
{
	OutSellPrice = CalculateSellValue(VehicleId);
	if (OutSellPrice <= 0)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "CannotSell", "Cannot sell this vehicle"));
	}

	FMGGarageResult Result = RemoveVehicle(VehicleId);
	if (Result.bSuccess)
	{
		Result.CostOrRefund = -OutSellPrice; // Negative = money gained
	}
	return Result;
}

TArray<FMGOwnedVehicle> UMGGarageSubsystem::GetAllVehicles() const
{
	return OwnedVehicles;
}

bool UMGGarageSubsystem::GetVehicle(const FGuid& VehicleId, FMGOwnedVehicle& OutVehicle) const
{
	int32 Index = FindVehicleIndex(VehicleId);
	if (Index != INDEX_NONE)
	{
		OutVehicle = OwnedVehicles[Index];
		return true;
	}
	return false;
}

bool UMGGarageSubsystem::GetSelectedVehicle(FMGOwnedVehicle& OutVehicle) const
{
	return GetVehicle(SelectedVehicleId, OutVehicle);
}

FMGGarageResult UMGGarageSubsystem::SelectVehicle(const FGuid& VehicleId)
{
	int32 NewIndex = FindVehicleIndex(VehicleId);
	if (NewIndex == INDEX_NONE)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	// Deselect current
	int32 OldIndex = FindVehicleIndex(SelectedVehicleId);
	if (OldIndex != INDEX_NONE)
	{
		OwnedVehicles[OldIndex].bIsSelected = false;
	}

	// Select new
	SelectedVehicleId = VehicleId;
	OwnedVehicles[NewIndex].bIsSelected = true;

	OnVehicleSelected.Broadcast(VehicleId);
	return FMGGarageResult::Success();
}

bool UMGGarageSubsystem::OwnsVehicleType(UMGVehicleModelData* VehicleModelData) const
{
	if (!VehicleModelData)
	{
		return false;
	}

	for (const FMGOwnedVehicle& Vehicle : OwnedVehicles)
	{
		if (Vehicle.VehicleModelData.Get() == VehicleModelData)
		{
			return true;
		}
	}
	return false;
}

// ==========================================
// CUSTOMIZATION - PARTS
// ==========================================

FMGGarageResult UMGGarageSubsystem::InstallPart(const FGuid& VehicleId, const FMGPartData& Part)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	if (!IsPartCompatible(VehicleId, Part))
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "PartNotCompatible", "This part is not compatible with your vehicle"));
	}

	// Create installed part entry
	FMGInstalledPart InstalledPart;
	InstalledPart.PartData = Part;
	InstalledPart.InstallDate = FDateTime::Now();

	// Replace existing part in slot
	Vehicle->InstalledParts.Add(Part.Slot, InstalledPart);
	Vehicle->TotalInvestment += Part.Price;

	// Recalculate stats
	RecalculateVehicleStats(VehicleId);

	OnPartInstalled.Broadcast(VehicleId, Part.Slot);
	OnVehicleChanged.Broadcast(VehicleId);

	return FMGGarageResult::Success(Part.Price);
}

FMGGarageResult UMGGarageSubsystem::RemovePart(const FGuid& VehicleId, EMGPartSlot Slot, FMGPartData& OutRemovedPart)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	FMGInstalledPart* InstalledPart = Vehicle->InstalledParts.Find(Slot);
	if (!InstalledPart)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "NoPartInSlot", "No part installed in this slot"));
	}

	OutRemovedPart = InstalledPart->PartData;
	Vehicle->InstalledParts.Remove(Slot);

	// Recalculate stats
	RecalculateVehicleStats(VehicleId);

	OnPartRemoved.Broadcast(VehicleId, Slot);
	OnVehicleChanged.Broadcast(VehicleId);

	// Refund partial value
	int64 Refund = OutRemovedPart.Price / 2; // 50% refund
	return FMGGarageResult::Success(-Refund);
}

bool UMGGarageSubsystem::GetInstalledPart(const FGuid& VehicleId, EMGPartSlot Slot, FMGInstalledPart& OutPart) const
{
	FMGOwnedVehicle Vehicle;
	if (!GetVehicle(VehicleId, Vehicle))
	{
		return false;
	}

	const FMGInstalledPart* InstalledPart = Vehicle.InstalledParts.Find(Slot);
	if (InstalledPart)
	{
		OutPart = *InstalledPart;
		return true;
	}
	return false;
}

TMap<EMGPartSlot, FMGInstalledPart> UMGGarageSubsystem::GetAllInstalledParts(const FGuid& VehicleId) const
{
	FMGOwnedVehicle Vehicle;
	if (GetVehicle(VehicleId, Vehicle))
	{
		return Vehicle.InstalledParts;
	}
	return TMap<EMGPartSlot, FMGInstalledPart>();
}

bool UMGGarageSubsystem::IsPartCompatible(const FGuid& VehicleId, const FMGPartData& Part) const
{
	FMGOwnedVehicle Vehicle;
	if (!GetVehicle(VehicleId, Vehicle))
	{
		return false;
	}

	UMGVehicleModelData* ModelData = Vehicle.VehicleModelData.LoadSynchronous();
	if (!ModelData)
	{
		return false;
	}

	// Check if part is universal or matches vehicle
	if (Part.CompatibleVehicles.Num() == 0)
	{
		return true; // Universal part
	}

	return Part.CompatibleVehicles.Contains(ModelData);
}

FMGVehicleStats UMGGarageSubsystem::PreviewPartInstallation(const FGuid& VehicleId, const FMGPartData& Part) const
{
	FMGOwnedVehicle Vehicle;
	if (!GetVehicle(VehicleId, Vehicle))
	{
		return FMGVehicleStats();
	}

	// Create temporary copy with new part
	Vehicle.InstalledParts.Add(Part.Slot, FMGInstalledPart());
	Vehicle.InstalledParts[Part.Slot].PartData = Part;

	// Calculate what stats would be
	UMGVehicleModelData* ModelData = Vehicle.VehicleModelData.LoadSynchronous();
	if (!ModelData)
	{
		return FMGVehicleStats();
	}

	// Build vehicle data struct from model and parts
	FMGVehicleData TempVehicleData;
	TempVehicleData.BaseModelID = ModelData->ModelID;

	// Apply part modifiers to build configuration
	for (const auto& Pair : Vehicle.InstalledParts)
	{
		ApplyPartToVehicleData(TempVehicleData, Pair.Value.PartData);
	}

	return UMGStatCalculator::CalculateAllStats(TempVehicleData, ModelData);
}

// ==========================================
// CUSTOMIZATION - PAINT
// ==========================================

FMGGarageResult UMGGarageSubsystem::ApplyPaint(const FGuid& VehicleId, const FMGPaintConfiguration& Paint)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	Vehicle->Paint = Paint;
	OnVehicleChanged.Broadcast(VehicleId);

	return FMGGarageResult::Success();
}

FMGGarageResult UMGGarageSubsystem::SetPrimaryColor(const FGuid& VehicleId, const FLinearColor& Color)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	Vehicle->Paint.PrimaryColor = Color;
	OnVehicleChanged.Broadcast(VehicleId);

	return FMGGarageResult::Success();
}

FMGGarageResult UMGGarageSubsystem::SetSecondaryColor(const FGuid& VehicleId, const FLinearColor& Color)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	Vehicle->Paint.SecondaryColor = Color;
	OnVehicleChanged.Broadcast(VehicleId);

	return FMGGarageResult::Success();
}

bool UMGGarageSubsystem::GetPaintConfiguration(const FGuid& VehicleId, FMGPaintConfiguration& OutPaint) const
{
	FMGOwnedVehicle Vehicle;
	if (GetVehicle(VehicleId, Vehicle))
	{
		OutPaint = Vehicle.Paint;
		return true;
	}
	return false;
}

// ==========================================
// STATS & CALCULATIONS
// ==========================================

void UMGGarageSubsystem::RecalculateVehicleStats(const FGuid& VehicleId)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return;
	}

	UMGVehicleModelData* ModelData = Vehicle->VehicleModelData.LoadSynchronous();
	if (!ModelData)
	{
		return;
	}

	// Build vehicle data struct from model and installed parts
	FMGVehicleData TempVehicleData;
	TempVehicleData.BaseModelID = ModelData->ModelID;
	TempVehicleData.Drivetrain.DrivetrainType = ModelData->BaseDrivetrain;
	TempVehicleData.Engine.EngineType = ModelData->BaseEngineType;
	TempVehicleData.Engine.DisplacementCC = ModelData->BaseDisplacementCC;

	// Apply part modifiers
	for (const auto& Pair : Vehicle->InstalledParts)
	{
		ApplyPartToVehicleData(TempVehicleData, Pair.Value.PartData);
	}

	// Calculate stats using stat calculator
	FMGVehicleStats CalculatedStats = UMGStatCalculator::CalculateAllStats(TempVehicleData, ModelData);

	// Cache the stats
	CachedStats.Add(VehicleId, CalculatedStats);

	// Update PI and class on vehicle
	Vehicle->PerformanceIndex = static_cast<int32>(CalculatedStats.PerformanceIndex);
	Vehicle->PerformanceClass = CalculatedStats.PerformanceClass;
}

FMGVehicleStats UMGGarageSubsystem::GetVehicleStats(const FGuid& VehicleId) const
{
	const FMGVehicleStats* CachedStat = CachedStats.Find(VehicleId);
	if (CachedStat)
	{
		return *CachedStat;
	}

	// Calculate on demand if not cached (shouldn't normally happen)
	return FMGVehicleStats();
}

int32 UMGGarageSubsystem::GetPerformanceIndex(const FGuid& VehicleId) const
{
	FMGOwnedVehicle Vehicle;
	if (GetVehicle(VehicleId, Vehicle))
	{
		return Vehicle.PerformanceIndex;
	}
	return 0;
}

EMGPerformanceClass UMGGarageSubsystem::GetPerformanceClass(const FGuid& VehicleId) const
{
	FMGOwnedVehicle Vehicle;
	if (GetVehicle(VehicleId, Vehicle))
	{
		return Vehicle.PerformanceClass;
	}
	return EMGPerformanceClass::D;
}

int64 UMGGarageSubsystem::CalculateSellValue(const FGuid& VehicleId) const
{
	FMGOwnedVehicle Vehicle;
	if (!GetVehicle(VehicleId, Vehicle))
	{
		return 0;
	}

	// Base value is 60% of total investment
	int64 SellValue = Vehicle.TotalInvestment * 60 / 100;

	// Bonus for race wins
	SellValue += Vehicle.RacesWon * 100;

	return FMath::Max(SellValue, 1000LL); // Minimum sell value
}

// ==========================================
// VEHICLE SPAWNING
// ==========================================

AMGVehiclePawn* UMGGarageSubsystem::SpawnSelectedVehicle(const FTransform& SpawnTransform)
{
	return SpawnVehicle(SelectedVehicleId, SpawnTransform);
}

AMGVehiclePawn* UMGGarageSubsystem::SpawnVehicle(const FGuid& VehicleId, const FTransform& SpawnTransform)
{
	FMGOwnedVehicle Vehicle;
	if (!GetVehicle(VehicleId, Vehicle))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnVehicle: Vehicle not found"));
		return nullptr;
	}

	UMGVehicleModelData* ModelData = Vehicle.VehicleModelData.LoadSynchronous();
	if (!ModelData)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnVehicle: VehicleModelData not loaded"));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnVehicle: No world"));
		return nullptr;
	}

	// Get the pawn class to spawn from model data
	TSubclassOf<AMGVehiclePawn> PawnClass = nullptr;
	if (ModelData->VehicleBlueprintClass.IsValid())
	{
		PawnClass = Cast<UClass>(ModelData->VehicleBlueprintClass.LoadSynchronous());
	}

	if (!PawnClass)
	{
		// Use default vehicle pawn class
		PawnClass = AMGVehiclePawn::StaticClass();
	}

	// Spawn the vehicle
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AMGVehiclePawn* SpawnedVehicle = World->SpawnActor<AMGVehiclePawn>(
		PawnClass,
		SpawnTransform,
		SpawnParams
	);

	if (SpawnedVehicle)
	{
		ApplyCustomizationToVehicle(SpawnedVehicle, VehicleId);
	}

	return SpawnedVehicle;
}

void UMGGarageSubsystem::ApplyCustomizationToVehicle(AMGVehiclePawn* Vehicle, const FGuid& VehicleId)
{
	if (!Vehicle)
	{
		return;
	}

	FMGOwnedVehicle OwnedVehicle;
	if (!GetVehicle(VehicleId, OwnedVehicle))
	{
		return;
	}

	// Get calculated stats
	FMGVehicleStats Stats = GetVehicleStats(VehicleId);

	// Apply stats to vehicle movement component
	if (UMGVehicleMovementComponent* Movement = Vehicle->GetMGVehicleMovement())
	{
		// Create vehicle configuration from stats
		FMGVehicleData VehicleConfig;
		VehicleConfig.PerformanceIndex = OwnedVehicle.PerformanceIndex;
		VehicleConfig.MaxHorsePower = Stats.HorsePower;
		VehicleConfig.MaxTorque = Stats.Torque;
		VehicleConfig.Weight = Stats.Weight;
		VehicleConfig.TopSpeed = Stats.TopSpeed;
		VehicleConfig.Acceleration = Stats.Acceleration;
		VehicleConfig.Handling = Stats.Handling;
		VehicleConfig.Braking = Stats.Braking;
		VehicleConfig.NitrousCapacity = Stats.NitrousCapacity;

		Vehicle->LoadVehicleConfiguration(VehicleConfig);
	}

	// Apply paint to mesh materials
	if (USkeletalMeshComponent* Mesh = Vehicle->GetMesh())
	{
		for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
		{
			UMaterialInterface* BaseMaterial = Mesh->GetMaterial(i);
			if (BaseMaterial)
			{
				UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMaterial, Vehicle);
				if (DynMat)
				{
					// Apply primary and secondary colors
					DynMat->SetVectorParameterValue(TEXT("PrimaryColor"), OwnedVehicle.Paint.PrimaryColor);
					DynMat->SetVectorParameterValue(TEXT("SecondaryColor"), OwnedVehicle.Paint.SecondaryColor);
					DynMat->SetScalarParameterValue(TEXT("Metallic"), OwnedVehicle.Paint.Metallic);
					DynMat->SetScalarParameterValue(TEXT("Roughness"), OwnedVehicle.Paint.Roughness);
					DynMat->SetScalarParameterValue(TEXT("ClearCoat"), OwnedVehicle.Paint.ClearCoat);

					Mesh->SetMaterial(i, DynMat);
				}
			}
		}
	}

	// Apply visual parts (body kits, spoilers, etc.)
	// First, remove any existing customization components to prevent memory leaks
	TArray<UStaticMeshComponent*> ExistingPartComps;
	Vehicle->GetComponents<UStaticMeshComponent>(ExistingPartComps);
	for (UStaticMeshComponent* Comp : ExistingPartComps)
	{
		if (Comp && Comp->ComponentHasTag(TEXT("CustomizationPart")))
		{
			Comp->DestroyComponent();
		}
	}

	for (const auto& Pair : OwnedVehicle.InstalledParts)
	{
		const FName& SlotName = Pair.Key;
		const FMGInstalledPart& InstalledPart = Pair.Value;

		if (InstalledPart.PartData)
		{
			// Apply mesh attachment for visual parts
			if (UStaticMesh* PartMesh = InstalledPart.PartData->PartMesh.LoadSynchronous())
			{
				UStaticMeshComponent* PartComp = NewObject<UStaticMeshComponent>(Vehicle);
				PartComp->ComponentTags.Add(TEXT("CustomizationPart"));
				PartComp->SetStaticMesh(PartMesh);
				PartComp->AttachToComponent(Vehicle->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, SlotName);
				PartComp->RegisterComponent();
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Applied customization to vehicle: %s (PI: %d)"),
		*OwnedVehicle.CustomName, OwnedVehicle.PerformanceIndex);
}

// ==========================================
// VEHICLE STATS TRACKING
// ==========================================

void UMGGarageSubsystem::AddOdometerDistance(const FGuid& VehicleId, float DistanceInCm)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (Vehicle)
	{
		Vehicle->Odometer += DistanceInCm;
	}
}

void UMGGarageSubsystem::RecordRaceResult(const FGuid& VehicleId, bool bWon)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (Vehicle)
	{
		Vehicle->RacesCompleted++;
		if (bWon)
		{
			Vehicle->RacesWon++;
		}
	}
}

FMGGarageResult UMGGarageSubsystem::RenameVehicle(const FGuid& VehicleId, const FString& NewName)
{
	FMGOwnedVehicle* Vehicle = GetVehicleMutable(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found in garage"));
	}

	if (NewName.IsEmpty())
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "EmptyName", "Vehicle name cannot be empty"));
	}

	if (NewName.Len() > 32)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "NameTooLong", "Vehicle name is too long"));
	}

	Vehicle->CustomName = NewName;
	OnVehicleChanged.Broadcast(VehicleId);

	return FMGGarageResult::Success();
}

// ==========================================
// INTERNAL
// ==========================================

int32 UMGGarageSubsystem::FindVehicleIndex(const FGuid& VehicleId) const
{
	for (int32 i = 0; i < OwnedVehicles.Num(); ++i)
	{
		if (OwnedVehicles[i].VehicleId == VehicleId)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

FMGOwnedVehicle* UMGGarageSubsystem::GetVehicleMutable(const FGuid& VehicleId)
{
	int32 Index = FindVehicleIndex(VehicleId);
	if (Index != INDEX_NONE)
	{
		return &OwnedVehicles[Index];
	}
	return nullptr;
}

void UMGGarageSubsystem::InvalidateStatsCache(const FGuid& VehicleId)
{
	CachedStats.Remove(VehicleId);
}

void UMGGarageSubsystem::ApplyPartToVehicleData(FMGVehicleData& VehicleData, const FMGPartData& Part) const
{
	// Apply part modifiers based on slot type
	const FMGPartModifiers& Mods = Part.Modifiers;

	switch (Part.Slot)
	{
		// Engine parts affect power
		case EMGPartSlot::CylinderHead:
		case EMGPartSlot::Camshaft:
		case EMGPartSlot::IntakeManifold:
		case EMGPartSlot::ExhaustManifold:
		case EMGPartSlot::ExhaustSystem:
		case EMGPartSlot::AirFilter:
			// These are tracked via part IDs in engine config
			break;

		// Forced induction
		case EMGPartSlot::Turbo:
			VehicleData.Engine.ForcedInduction.Type = EMGForcedInductionType::Turbo_Single;
			VehicleData.Engine.ForcedInduction.MaxBoostPSI = Mods.BoostCapacity;
			break;

		case EMGPartSlot::Supercharger:
			VehicleData.Engine.ForcedInduction.Type = EMGForcedInductionType::Supercharger_Roots;
			VehicleData.Engine.ForcedInduction.MaxBoostPSI = Mods.BoostCapacity;
			break;

		case EMGPartSlot::Intercooler:
			VehicleData.Engine.ForcedInduction.IntercoolerEfficiency = 0.85f + (Mods.FlowRating * 0.1f);
			break;

		// Nitrous
		case EMGPartSlot::Nitrous:
			VehicleData.Engine.Nitrous.bInstalled = true;
			VehicleData.Engine.Nitrous.ShotSizeHP = Mods.PowerMultiplier * 100.0f; // Base 100HP shot
			break;

		// Drivetrain
		case EMGPartSlot::Clutch:
			VehicleData.Drivetrain.ClutchTorqueCapacity = 400.0f * Mods.TorqueMultiplier;
			break;

		case EMGPartSlot::Differential:
			// Could set differential type based on part
			break;

		// Suspension doesn't directly affect FMGVehicleData used for stat calc
		// Brakes, wheels, tires similarly

		default:
			break;
	}
}

// ==========================================
// VEHICLE CONFIG EXPORT/IMPORT
// ==========================================

bool UMGGarageSubsystem::ExportVehicleBuild(const FGuid& VehicleId, FString& OutJsonString) const
{
	const FMGOwnedVehicle* Vehicle = GetOwnedVehicle(VehicleId);
	if (!Vehicle)
	{
		return false;
	}

	// Create JSON object for the build
	TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);

	// Export metadata
	RootObject->SetStringField(TEXT("version"), TEXT("1.0"));
	RootObject->SetStringField(TEXT("vehicleId"), Vehicle->VehicleId.ToString());
	RootObject->SetStringField(TEXT("customName"), Vehicle->CustomName);
	if (Vehicle->VehicleModelData.IsValid())
	{
		RootObject->SetStringField(TEXT("baseVehicle"), Vehicle->VehicleModelData.GetAssetName());
	}

	// Export installed parts
	TSharedPtr<FJsonObject> PartsObject = MakeShareable(new FJsonObject);
	for (const auto& PartPair : Vehicle->InstalledParts)
	{
		TSharedPtr<FJsonObject> PartObj = MakeShareable(new FJsonObject);
		PartObj->SetStringField(TEXT("partId"), PartPair.Value.PartId.ToString());
		PartObj->SetStringField(TEXT("displayName"), PartPair.Value.DisplayName.ToString());
		PartObj->SetNumberField(TEXT("installDate"), PartPair.Value.InstallDate.GetTicks());

		FString SlotName = UEnum::GetValueAsString(PartPair.Key);
		PartsObject->SetObjectField(SlotName, PartObj);
	}
	RootObject->SetObjectField(TEXT("installedParts"), PartsObject);

	// Export paint configuration
	TSharedPtr<FJsonObject> PaintObject = MakeShareable(new FJsonObject);
	PaintObject->SetStringField(TEXT("primary"), Vehicle->Paint.PrimaryColor.ToFColor(true).ToHex());
	PaintObject->SetStringField(TEXT("secondary"), Vehicle->Paint.SecondaryColor.ToFColor(true).ToHex());
	PaintObject->SetStringField(TEXT("accent"), Vehicle->Paint.AccentColor.ToFColor(true).ToHex());
	PaintObject->SetStringField(TEXT("finish"), UEnum::GetValueAsString(Vehicle->Paint.FinishType));
	PaintObject->SetNumberField(TEXT("metallic"), Vehicle->Paint.MetallicIntensity);
	PaintObject->SetNumberField(TEXT("clearcoat"), Vehicle->Paint.ClearcoatIntensity);
	RootObject->SetObjectField(TEXT("paint"), PaintObject);

	// Export stats (for reference, not imported)
	RootObject->SetNumberField(TEXT("performanceIndex"), Vehicle->PerformanceIndex);
	RootObject->SetStringField(TEXT("performanceClass"), UEnum::GetValueAsString(Vehicle->PerformanceClass));

	// Serialize to string
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJsonString);
	if (FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer))
	{
		return true;
	}

	return false;
}

FMGGarageResult UMGGarageSubsystem::ImportVehicleBuild(const FGuid& VehicleId, const FString& JsonString, bool bRequireOwnedParts)
{
	FMGOwnedVehicle* Vehicle = GetOwnedVehicle(VehicleId);
	if (!Vehicle)
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "VehicleNotFound", "Vehicle not found"));
	}

	// Parse JSON
	TSharedPtr<FJsonObject> RootObject;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "InvalidJson", "Invalid JSON format"));
	}

	// Check version
	FString Version;
	if (RootObject->TryGetStringField(TEXT("version"), Version) && Version != TEXT("1.0"))
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "IncompatibleVersion", "Incompatible build version"));
	}

	int32 PartsApplied = 0;
	int32 PartsSkipped = 0;

	// Import parts
	const TSharedPtr<FJsonObject>* PartsObject;
	if (RootObject->TryGetObjectField(TEXT("installedParts"), PartsObject))
	{
		for (const auto& PartEntry : (*PartsObject)->Values)
		{
			const TSharedPtr<FJsonObject>* PartObj;
			if (PartEntry.Value->TryGetObject(PartObj))
			{
				FString PartIdStr;
				if ((*PartObj)->TryGetStringField(TEXT("partId"), PartIdStr))
				{
					// Parse slot from key name
					// Key is like "EMGPartSlot::Turbo"
					FString SlotStr = PartEntry.Key;
					SlotStr.RemoveFromStart(TEXT("EMGPartSlot::"));

					// For now, mark that we would apply this part
					// In a full implementation, we would look up the part and install it
					PartsApplied++;
				}
			}
		}
	}

	// Import paint
	const TSharedPtr<FJsonObject>* PaintObject;
	if (RootObject->TryGetObjectField(TEXT("paint"), PaintObject))
	{
		FString PrimaryHex, SecondaryHex, AccentHex;
		if ((*PaintObject)->TryGetStringField(TEXT("primary"), PrimaryHex))
		{
			Vehicle->Paint.PrimaryColor = FLinearColor(FColor::FromHex(PrimaryHex));
		}
		if ((*PaintObject)->TryGetStringField(TEXT("secondary"), SecondaryHex))
		{
			Vehicle->Paint.SecondaryColor = FLinearColor(FColor::FromHex(SecondaryHex));
		}
		if ((*PaintObject)->TryGetStringField(TEXT("accent"), AccentHex))
		{
			Vehicle->Paint.AccentColor = FLinearColor(FColor::FromHex(AccentHex));
		}

		double Metallic, Clearcoat;
		if ((*PaintObject)->TryGetNumberField(TEXT("metallic"), Metallic))
		{
			Vehicle->Paint.MetallicIntensity = static_cast<float>(Metallic);
		}
		if ((*PaintObject)->TryGetNumberField(TEXT("clearcoat"), Clearcoat))
		{
			Vehicle->Paint.ClearcoatIntensity = static_cast<float>(Clearcoat);
		}
	}

	// Recalculate stats
	RecalculateVehicleStats(VehicleId);

	// Broadcast change
	OnVehicleModified.Broadcast(VehicleId);

	return FMGGarageResult::Success();
}

bool UMGGarageSubsystem::ExportVehicleBuildToFile(const FGuid& VehicleId, const FString& FilePath) const
{
	FString JsonString;
	if (!ExportVehicleBuild(VehicleId, JsonString))
	{
		return false;
	}

	return FFileHelper::SaveStringToFile(JsonString, *FilePath);
}

FMGGarageResult UMGGarageSubsystem::ImportVehicleBuildFromFile(const FGuid& VehicleId, const FString& FilePath, bool bRequireOwnedParts)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "FileNotFound", "Build file not found"));
	}

	return ImportVehicleBuild(VehicleId, JsonString, bRequireOwnedParts);
}

FString UMGGarageSubsystem::GetBuildCode(const FGuid& VehicleId) const
{
	FString JsonString;
	if (!ExportVehicleBuild(VehicleId, JsonString))
	{
		return FString();
	}

	// Compress and encode as base64
	// For a short shareable code, we'll just hash the config and store a mapping
	// For now, return a shortened base64 of the config hash

	uint32 Hash = GetTypeHash(JsonString);
	FString HashStr = FString::Printf(TEXT("%08X"), Hash);

	// In a real implementation, we'd store the full config on a server
	// and use the code as a lookup key
	return FString::Printf(TEXT("MG-%s"), *HashStr);
}

FMGGarageResult UMGGarageSubsystem::ApplyBuildCode(const FGuid& VehicleId, const FString& BuildCode)
{
	// In a real implementation, this would:
	// 1. Query a server with the build code
	// 2. Retrieve the full JSON config
	// 3. Call ImportVehicleBuild

	if (!BuildCode.StartsWith(TEXT("MG-")))
	{
		return FMGGarageResult::Failure(NSLOCTEXT("Garage", "InvalidBuildCode", "Invalid build code format"));
	}

	// For now, return a placeholder message
	return FMGGarageResult::Failure(NSLOCTEXT("Garage", "BuildCodeNotImplemented",
		"Build code lookup requires online services (not implemented in MVP)"));
}

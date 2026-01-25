// Copyright Midnight Grind. All Rights Reserved.
// Updated Stage 52: MVP Game Entry Points - Starter Vehicle

#include "Garage/MGGarageSubsystem.h"
#include "Vehicle/MGVehiclePawn.h"
#include "Vehicle/MGVehicleData.h"
#include "Vehicle/MGStatCalculator.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

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

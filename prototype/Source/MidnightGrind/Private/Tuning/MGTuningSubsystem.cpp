// Copyright Midnight Grind. All Rights Reserved.

#include "Tuning/MGTuningSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UMGTuningSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LoadTuningData();
}

void UMGTuningSubsystem::Deinitialize()
{
	SaveTuningData();
	Super::Deinitialize();
}

bool UMGTuningSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

// ============================================================================
// Part Management
// ============================================================================

bool UMGTuningSubsystem::InstallPart(FName VehicleID, FName PartID)
{
	if (!CanInstallPart(VehicleID, PartID))
	{
		return false;
	}

	const FMGTuningPart* Part = PartDatabase.Find(PartID);
	if (!Part)
	{
		return false;
	}

	FMGVehicleTuning& Tuning = VehicleTunings.FindOrAdd(VehicleID);
	Tuning.VehicleID = VehicleID;
	Tuning.InstalledParts.Add(Part->Category, PartID);

	RecalculateStats(VehicleID);

	OnPartInstalled.Broadcast(VehicleID, *Part);
	OnTuningChanged.Broadcast(VehicleID, Tuning);

	SaveTuningData();
	return true;
}

bool UMGTuningSubsystem::RemovePart(FName VehicleID, EMGTuningCategory Category)
{
	FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
	if (!Tuning)
	{
		return false;
	}

	if (!Tuning->InstalledParts.Contains(Category))
	{
		return false;
	}

	Tuning->InstalledParts.Remove(Category);

	RecalculateStats(VehicleID);

	OnPartRemoved.Broadcast(VehicleID, Category);
	OnTuningChanged.Broadcast(VehicleID, *Tuning);

	SaveTuningData();
	return true;
}

bool UMGTuningSubsystem::PurchasePart(FName PartID)
{
	if (OwnsPart(PartID))
	{
		return false;
	}

	const FMGTuningPart* Part = PartDatabase.Find(PartID);
	if (!Part)
	{
		return false;
	}

	// Would check currency and deduct here
	OwnedParts.Add(PartID);

	SaveTuningData();
	return true;
}

bool UMGTuningSubsystem::OwnsPart(FName PartID) const
{
	return OwnedParts.Contains(PartID);
}

bool UMGTuningSubsystem::CanInstallPart(FName VehicleID, FName PartID) const
{
	if (!OwnsPart(PartID))
	{
		return false;
	}

	const FMGTuningPart* Part = PartDatabase.Find(PartID);
	if (!Part)
	{
		return false;
	}

	// Check vehicle compatibility
	if (Part->CompatibleVehicles.Num() > 0 && !Part->CompatibleVehicles.Contains(VehicleID))
	{
		return false;
	}

	// Check prerequisite part
	if (!Part->RequiredPreviousPart.IsNone())
	{
		const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
		if (!Tuning)
		{
			return false;
		}

		const FName* InstalledPart = Tuning->InstalledParts.Find(Part->Category);
		if (!InstalledPart || *InstalledPart != Part->RequiredPreviousPart)
		{
			return false;
		}
	}

	return true;
}

FMGTuningPart UMGTuningSubsystem::GetInstalledPart(FName VehicleID, EMGTuningCategory Category) const
{
	const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
	if (!Tuning)
	{
		return FMGTuningPart();
	}

	const FName* PartID = Tuning->InstalledParts.Find(Category);
	if (!PartID)
	{
		return FMGTuningPart();
	}

	if (const FMGTuningPart* Part = PartDatabase.Find(*PartID))
	{
		FMGTuningPart InstalledPart = *Part;
		InstalledPart.bInstalled = true;
		InstalledPart.bOwned = true;
		return InstalledPart;
	}

	return FMGTuningPart();
}

TArray<FMGTuningPart> UMGTuningSubsystem::GetAvailableParts(FName VehicleID, EMGTuningCategory Category) const
{
	TArray<FMGTuningPart> AvailableParts;

	for (const auto& PartPair : PartDatabase)
	{
		if (PartPair.Value.Category != Category)
		{
			continue;
		}

		// Check vehicle compatibility
		if (PartPair.Value.CompatibleVehicles.Num() > 0 && !PartPair.Value.CompatibleVehicles.Contains(VehicleID))
		{
			continue;
		}

		FMGTuningPart Part = PartPair.Value;
		Part.bOwned = OwnedParts.Contains(Part.PartID);

		// Check if installed
		const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
		if (Tuning)
		{
			const FName* InstalledPartID = Tuning->InstalledParts.Find(Category);
			Part.bInstalled = InstalledPartID && *InstalledPartID == Part.PartID;
		}

		AvailableParts.Add(Part);
	}

	// Sort by level
	AvailableParts.Sort([](const FMGTuningPart& A, const FMGTuningPart& B)
	{
		return static_cast<int32>(A.Level) < static_cast<int32>(B.Level);
	});

	return AvailableParts;
}

TArray<FMGTuningPart> UMGTuningSubsystem::GetOwnedParts() const
{
	TArray<FMGTuningPart> Owned;

	for (const FName& PartID : OwnedParts)
	{
		if (const FMGTuningPart* Part = PartDatabase.Find(PartID))
		{
			FMGTuningPart OwnedPart = *Part;
			OwnedPart.bOwned = true;
			Owned.Add(OwnedPart);
		}
	}

	return Owned;
}

// ============================================================================
// Slider Tuning
// ============================================================================

void UMGTuningSubsystem::SetSliderValue(FName VehicleID, FName SliderID, float Value)
{
	const FMGTuningSlider* Slider = SliderDatabase.Find(SliderID);
	if (!Slider)
	{
		return;
	}

	float ClampedValue = FMath::Clamp(Value, Slider->MinValue, Slider->MaxValue);

	FMGVehicleTuning& Tuning = VehicleTunings.FindOrAdd(VehicleID);
	Tuning.VehicleID = VehicleID;
	Tuning.SliderValues.Add(SliderID, ClampedValue);

	// Apply to specific tuning values based on slider ID
	if (SliderID == TEXT("BrakeBias"))
	{
		Tuning.BrakeBias = ClampedValue;
	}
	else if (SliderID == TEXT("RideHeight"))
	{
		Tuning.RideHeight = ClampedValue;
	}
	else if (SliderID == TEXT("FrontDownforce"))
	{
		Tuning.FrontDownforce = ClampedValue;
	}
	else if (SliderID == TEXT("RearDownforce"))
	{
		Tuning.RearDownforce = ClampedValue;
	}
	else if (SliderID == TEXT("CamberFront"))
	{
		Tuning.CamberFront = ClampedValue;
	}
	else if (SliderID == TEXT("CamberRear"))
	{
		Tuning.CamberRear = ClampedValue;
	}
	else if (SliderID == TEXT("AntiRollFront"))
	{
		Tuning.AntiRollFront = ClampedValue;
	}
	else if (SliderID == TEXT("AntiRollRear"))
	{
		Tuning.AntiRollRear = ClampedValue;
	}
	else if (SliderID == TEXT("SpringFront"))
	{
		Tuning.SpringStiffnessFront = ClampedValue;
	}
	else if (SliderID == TEXT("SpringRear"))
	{
		Tuning.SpringStiffnessRear = ClampedValue;
	}
	else if (SliderID == TEXT("DifferentialFront"))
	{
		Tuning.DifferentialFront = ClampedValue;
	}
	else if (SliderID == TEXT("DifferentialRear"))
	{
		Tuning.DifferentialRear = ClampedValue;
	}

	RecalculateStats(VehicleID);

	OnTuningChanged.Broadcast(VehicleID, Tuning);
	SaveTuningData();
}

float UMGTuningSubsystem::GetSliderValue(FName VehicleID, FName SliderID) const
{
	const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
	if (Tuning)
	{
		if (const float* Value = Tuning->SliderValues.Find(SliderID))
		{
			return *Value;
		}
	}

	// Return default value
	if (const FMGTuningSlider* Slider = SliderDatabase.Find(SliderID))
	{
		return Slider->DefaultValue;
	}

	return 50.0f;
}

TArray<FMGTuningSlider> UMGTuningSubsystem::GetAvailableSliders(FName VehicleID) const
{
	TArray<FMGTuningSlider> Sliders;

	for (const auto& SliderPair : SliderDatabase)
	{
		FMGTuningSlider Slider = SliderPair.Value;
		Slider.CurrentValue = GetSliderValue(VehicleID, Slider.SliderID);
		Sliders.Add(Slider);
	}

	return Sliders;
}

void UMGTuningSubsystem::ResetSliderToDefault(FName VehicleID, FName SliderID)
{
	if (const FMGTuningSlider* Slider = SliderDatabase.Find(SliderID))
	{
		SetSliderValue(VehicleID, SliderID, Slider->DefaultValue);
	}
}

void UMGTuningSubsystem::ResetAllSlidersToDefault(FName VehicleID)
{
	for (const auto& SliderPair : SliderDatabase)
	{
		SetSliderValue(VehicleID, SliderPair.Key, SliderPair.Value.DefaultValue);
	}
}

// ============================================================================
// Advanced Tuning
// ============================================================================

void UMGTuningSubsystem::SetGearRatio(FName VehicleID, int32 GearIndex, float Ratio)
{
	FMGVehicleTuning& Tuning = VehicleTunings.FindOrAdd(VehicleID);
	Tuning.VehicleID = VehicleID;

	// Ensure gear array is large enough
	while (Tuning.GearRatios.Num() <= GearIndex)
	{
		Tuning.GearRatios.Add(1.0f);
	}

	Tuning.GearRatios[GearIndex] = Ratio;

	RecalculateStats(VehicleID);
	OnTuningChanged.Broadcast(VehicleID, Tuning);
	SaveTuningData();
}

void UMGTuningSubsystem::SetFinalDrive(FName VehicleID, float Ratio)
{
	FMGVehicleTuning& Tuning = VehicleTunings.FindOrAdd(VehicleID);
	Tuning.VehicleID = VehicleID;
	Tuning.FinalGearRatio = Ratio;

	RecalculateStats(VehicleID);
	OnTuningChanged.Broadcast(VehicleID, Tuning);
	SaveTuningData();
}

void UMGTuningSubsystem::SetDrivetrainSwap(FName VehicleID, EMGDrivetrainType NewDrivetrain)
{
	FMGVehicleTuning& Tuning = VehicleTunings.FindOrAdd(VehicleID);
	Tuning.VehicleID = VehicleID;
	Tuning.DrivetrainSwap = NewDrivetrain;
	Tuning.bHasDrivetrainSwap = true;

	RecalculateStats(VehicleID);
	OnTuningChanged.Broadcast(VehicleID, Tuning);
	SaveTuningData();
}

FMGVehicleTuning UMGTuningSubsystem::GetVehicleTuning(FName VehicleID) const
{
	if (const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID))
	{
		return *Tuning;
	}

	return FMGVehicleTuning();
}

void UMGTuningSubsystem::SetVehicleTuning(FName VehicleID, const FMGVehicleTuning& Tuning)
{
	VehicleTunings.Add(VehicleID, Tuning);

	RecalculateStats(VehicleID);
	OnTuningChanged.Broadcast(VehicleID, Tuning);
	SaveTuningData();
}

// ============================================================================
// Stats
// ============================================================================

FMGVehicleStats UMGTuningSubsystem::GetBaseStats(FName VehicleID) const
{
	if (const FMGVehicleStats* Stats = BaseVehicleStats.Find(VehicleID))
	{
		return *Stats;
	}

	return FMGVehicleStats();
}

FMGVehicleStats UMGTuningSubsystem::GetTunedStats(FName VehicleID) const
{
	if (const FMGVehicleStats* Stats = TunedVehicleStats.Find(VehicleID))
	{
		return *Stats;
	}

	return GetBaseStats(VehicleID);
}

int32 UMGTuningSubsystem::GetPerformanceIndex(FName VehicleID) const
{
	FMGVehicleStats Stats = GetTunedStats(VehicleID);
	return Stats.PerformanceIndex;
}

FMGVehicleStats UMGTuningSubsystem::PreviewPartInstall(FName VehicleID, FName PartID) const
{
	FMGVehicleStats BaseStats = GetBaseStats(VehicleID);
	FMGVehicleStats PreviewStats = GetTunedStats(VehicleID);

	const FMGTuningPart* Part = PartDatabase.Find(PartID);
	if (!Part)
	{
		return PreviewStats;
	}

	// Apply part bonuses
	PreviewStats.TopSpeed = FMath::Max(0.0f, PreviewStats.TopSpeed + Part->TopSpeedBonus);
	PreviewStats.Acceleration = FMath::Max(0.0f, PreviewStats.Acceleration + Part->AccelerationBonus);
	PreviewStats.Handling = FMath::Max(0.0f, PreviewStats.Handling + Part->HandlingBonus);
	PreviewStats.Braking = FMath::Max(0.0f, PreviewStats.Braking + Part->BrakingBonus);
	PreviewStats.Nitro = FMath::Max(0.0f, PreviewStats.Nitro + Part->NitroBonus);
	PreviewStats.Weight = FMath::Max(500.0f, PreviewStats.Weight + Part->WeightChange);
	PreviewStats.PerformanceIndex += Part->PerformanceIndexChange;

	return PreviewStats;
}

// ============================================================================
// Presets
// ============================================================================

FName UMGTuningSubsystem::SavePreset(FName VehicleID, const FText& PresetName)
{
	const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
	if (!Tuning)
	{
		return NAME_None;
	}

	FMGTuningPreset NewPreset;
	NewPreset.PresetID = *FString::Printf(TEXT("Preset_%s_%d"), *VehicleID.ToString(), FMath::RandRange(1000, 9999));
	NewPreset.PresetName = PresetName;
	NewPreset.TuningData = *Tuning;
	NewPreset.bIsDefault = false;
	NewPreset.bIsShared = false;

	SavedPresets.Add(NewPreset.PresetID, NewPreset);

	OnPresetSaved.Broadcast(VehicleID, NewPreset.PresetID);
	SaveTuningData();

	return NewPreset.PresetID;
}

bool UMGTuningSubsystem::LoadPreset(FName VehicleID, FName PresetID)
{
	const FMGTuningPreset* Preset = SavedPresets.Find(PresetID);
	if (!Preset)
	{
		// Check community presets
		for (const FMGTuningPreset& CommunityPreset : CommunityPresets)
		{
			if (CommunityPreset.PresetID == PresetID)
			{
				Preset = &CommunityPreset;
				break;
			}
		}
	}

	if (!Preset)
	{
		return false;
	}

	FMGVehicleTuning NewTuning = Preset->TuningData;
	NewTuning.VehicleID = VehicleID;

	VehicleTunings.Add(VehicleID, NewTuning);

	RecalculateStats(VehicleID);

	OnPresetLoaded.Broadcast(VehicleID, PresetID);
	OnTuningChanged.Broadcast(VehicleID, NewTuning);

	SaveTuningData();
	return true;
}

bool UMGTuningSubsystem::DeletePreset(FName PresetID)
{
	if (SavedPresets.Contains(PresetID))
	{
		SavedPresets.Remove(PresetID);
		SaveTuningData();
		return true;
	}

	return false;
}

TArray<FMGTuningPreset> UMGTuningSubsystem::GetSavedPresets(FName VehicleID) const
{
	TArray<FMGTuningPreset> Presets;

	for (const auto& PresetPair : SavedPresets)
	{
		if (PresetPair.Value.TuningData.VehicleID == VehicleID)
		{
			Presets.Add(PresetPair.Value);
		}
	}

	return Presets;
}

TArray<FMGTuningPreset> UMGTuningSubsystem::GetCommunityPresets(FName VehicleID) const
{
	TArray<FMGTuningPreset> Presets;

	for (const FMGTuningPreset& Preset : CommunityPresets)
	{
		if (Preset.TuningData.VehicleID == VehicleID)
		{
			Presets.Add(Preset);
		}
	}

	// Sort by rating
	Presets.Sort([](const FMGTuningPreset& A, const FMGTuningPreset& B)
	{
		return A.Rating > B.Rating;
	});

	return Presets;
}

void UMGTuningSubsystem::SharePreset(FName PresetID)
{
	if (FMGTuningPreset* Preset = SavedPresets.Find(PresetID))
	{
		Preset->bIsShared = true;
		// Would send to server here
		SaveTuningData();
	}
}

// ============================================================================
// Registration
// ============================================================================

void UMGTuningSubsystem::RegisterVehicle(FName VehicleID, const FMGVehicleStats& BaseStats)
{
	BaseVehicleStats.Add(VehicleID, BaseStats);
	TunedVehicleStats.Add(VehicleID, BaseStats);
}

void UMGTuningSubsystem::RegisterPart(const FMGTuningPart& Part)
{
	PartDatabase.Add(Part.PartID, Part);
}

void UMGTuningSubsystem::RegisterSlider(const FMGTuningSlider& Slider)
{
	SliderDatabase.Add(Slider.SliderID, Slider);
}

// ============================================================================
// Protected Helpers
// ============================================================================

FMGVehicleStats UMGTuningSubsystem::CalculateTunedStats(FName VehicleID) const
{
	FMGVehicleStats Stats = GetBaseStats(VehicleID);

	const FMGVehicleTuning* Tuning = VehicleTunings.Find(VehicleID);
	if (!Tuning)
	{
		return Stats;
	}

	// Apply installed parts
	for (const auto& PartPair : Tuning->InstalledParts)
	{
		if (const FMGTuningPart* Part = PartDatabase.Find(PartPair.Value))
		{
			Stats.TopSpeed += Part->TopSpeedBonus;
			Stats.Acceleration += Part->AccelerationBonus;
			Stats.Handling += Part->HandlingBonus;
			Stats.Braking += Part->BrakingBonus;
			Stats.Nitro += Part->NitroBonus;
			Stats.Weight += Part->WeightChange;
			Stats.PerformanceIndex += Part->PerformanceIndexChange;
		}
	}

	// Apply tuning adjustments
	// Suspension affects handling
	float SuspensionBalance = (Tuning->SpringStiffnessFront - 50.0f) * 0.01f;
	Stats.Handling += SuspensionBalance * 5.0f;

	// Downforce affects top speed and handling
	float TotalDownforce = (Tuning->FrontDownforce + Tuning->RearDownforce) / 2.0f - 50.0f;
	Stats.Handling += TotalDownforce * 0.1f;
	Stats.TopSpeed -= TotalDownforce * 0.05f;

	// Ride height affects handling and top speed
	float RideHeightDelta = Tuning->RideHeight - 50.0f;
	Stats.TopSpeed += RideHeightDelta * 0.03f;
	Stats.Handling -= RideHeightDelta * 0.02f;

	// Brake bias affects braking
	float BiasEffect = FMath::Abs(Tuning->BrakeBias - 50.0f) * 0.1f;
	Stats.Braking -= BiasEffect;

	// Clamp values
	Stats.TopSpeed = FMath::Max(50.0f, Stats.TopSpeed);
	Stats.Acceleration = FMath::Clamp(Stats.Acceleration, 0.0f, 100.0f);
	Stats.Handling = FMath::Clamp(Stats.Handling, 0.0f, 100.0f);
	Stats.Braking = FMath::Clamp(Stats.Braking, 0.0f, 100.0f);
	Stats.Nitro = FMath::Clamp(Stats.Nitro, 0.0f, 100.0f);
	Stats.Weight = FMath::Max(500.0f, Stats.Weight);
	Stats.PerformanceIndex = FMath::Max(100, Stats.PerformanceIndex);

	return Stats;
}

void UMGTuningSubsystem::RecalculateStats(FName VehicleID)
{
	FMGVehicleStats NewStats = CalculateTunedStats(VehicleID);
	TunedVehicleStats.Add(VehicleID, NewStats);

	OnStatsChanged.Broadcast(VehicleID, NewStats);
}

void UMGTuningSubsystem::SaveTuningData()
{
	// Persist tuning data
	// Implementation would use USaveGame or cloud save
}

void UMGTuningSubsystem::LoadTuningData()
{
	// Load persisted tuning data
	// Implementation would use USaveGame or cloud save
}

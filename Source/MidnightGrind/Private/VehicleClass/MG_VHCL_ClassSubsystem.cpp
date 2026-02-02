// Copyright Epic Games, Inc. All Rights Reserved.

#include "VehicleClass/MG_VHCL_ClassSubsystem.h"
#include "Save/MG_SAVE_ManagerSubsystem.h"

void UMGVehicleClassSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	InitializeDefaultClassTiers();

	// Initialize PI weight presets
	{
		FMGPIWeights Balanced;
		Balanced.SpeedWeight = 1.0f;
		Balanced.AccelerationWeight = 1.0f;
		Balanced.HandlingWeight = 1.0f;
		Balanced.BrakingWeight = 0.5f;
		Balanced.NitroWeight = 0.5f;
		Balanced.DurabilityWeight = 0.25f;
		Balanced.PowerToWeightFactor = 1.5f;
		PIWeightPresets.Add(EMGPIWeightPreset::Balanced, Balanced);
	}
	{
		FMGPIWeights SpeedFocus;
		SpeedFocus.SpeedWeight = 2.0f;
		SpeedFocus.AccelerationWeight = 0.75f;
		SpeedFocus.HandlingWeight = 0.5f;
		SpeedFocus.BrakingWeight = 0.25f;
		SpeedFocus.NitroWeight = 1.0f;
		SpeedFocus.DurabilityWeight = 0.1f;
		SpeedFocus.PowerToWeightFactor = 2.0f;
		PIWeightPresets.Add(EMGPIWeightPreset::SpeedFocus, SpeedFocus);
	}
	{
		FMGPIWeights AccelFocus;
		AccelFocus.SpeedWeight = 0.75f;
		AccelFocus.AccelerationWeight = 2.0f;
		AccelFocus.HandlingWeight = 0.75f;
		AccelFocus.BrakingWeight = 0.5f;
		AccelFocus.NitroWeight = 1.5f;
		AccelFocus.DurabilityWeight = 0.25f;
		AccelFocus.PowerToWeightFactor = 2.0f;
		PIWeightPresets.Add(EMGPIWeightPreset::AccelFocus, AccelFocus);
	}
	{
		FMGPIWeights HandlingFocus;
		HandlingFocus.SpeedWeight = 0.5f;
		HandlingFocus.AccelerationWeight = 0.75f;
		HandlingFocus.HandlingWeight = 2.0f;
		HandlingFocus.BrakingWeight = 1.5f;
		HandlingFocus.NitroWeight = 0.25f;
		HandlingFocus.DurabilityWeight = 0.5f;
		HandlingFocus.PowerToWeightFactor = 0.5f;
		PIWeightPresets.Add(EMGPIWeightPreset::HandlingFocus, HandlingFocus);
	}
	{
		FMGPIWeights DriftTuned;
		DriftTuned.SpeedWeight = 0.75f;
		DriftTuned.AccelerationWeight = 1.5f;
		DriftTuned.HandlingWeight = 1.5f;
		DriftTuned.BrakingWeight = 0.5f;
		DriftTuned.NitroWeight = 0.5f;
		DriftTuned.DurabilityWeight = 0.5f;
		DriftTuned.PowerToWeightFactor = 1.0f;
		PIWeightPresets.Add(EMGPIWeightPreset::DriftTuned, DriftTuned);
	}
	{
		FMGPIWeights DragTuned;
		DragTuned.SpeedWeight = 1.5f;
		DragTuned.AccelerationWeight = 2.5f;
		DragTuned.HandlingWeight = 0.1f;
		DragTuned.BrakingWeight = 0.1f;
		DragTuned.NitroWeight = 2.0f;
		DragTuned.DurabilityWeight = 0.1f;
		DragTuned.PowerToWeightFactor = 3.0f;
		PIWeightPresets.Add(EMGPIWeightPreset::DragTuned, DragTuned);
	}

	LoadVehicleClassData();
}

void UMGVehicleClassSubsystem::Deinitialize()
{
	SaveVehicleClassData();
	Super::Deinitialize();
}

void UMGVehicleClassSubsystem::InitializeDefaultClassTiers()
{
	{
		FMGClassTierDefinition D;
		D.Tier = EMGVehicleClassTier::D;
		D.DisplayName = FText::FromString(TEXT("D Class"));
		D.MinPI = 0;
		D.MaxPI = 199;
		D.ClassColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
		D.BaseRewardMultiplier = 0.5f;
		D.DifficultyMultiplier = 0.5f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::D, D);
	}
	{
		FMGClassTierDefinition C;
		C.Tier = EMGVehicleClassTier::C;
		C.DisplayName = FText::FromString(TEXT("C Class"));
		C.MinPI = 200;
		C.MaxPI = 349;
		C.ClassColor = FLinearColor(0.3f, 0.6f, 0.3f, 1.0f);
		C.BaseRewardMultiplier = 0.75f;
		C.DifficultyMultiplier = 0.75f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::C, C);
	}
	{
		FMGClassTierDefinition B;
		B.Tier = EMGVehicleClassTier::B;
		B.DisplayName = FText::FromString(TEXT("B Class"));
		B.MinPI = 350;
		B.MaxPI = 499;
		B.ClassColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);
		B.BaseRewardMultiplier = 1.0f;
		B.DifficultyMultiplier = 1.0f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::B, B);
	}
	{
		FMGClassTierDefinition A;
		A.Tier = EMGVehicleClassTier::A;
		A.DisplayName = FText::FromString(TEXT("A Class"));
		A.MinPI = 500;
		A.MaxPI = 649;
		A.ClassColor = FLinearColor(0.8f, 0.4f, 0.1f, 1.0f);
		A.BaseRewardMultiplier = 1.25f;
		A.DifficultyMultiplier = 1.25f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::A, A);
	}
	{
		FMGClassTierDefinition S;
		S.Tier = EMGVehicleClassTier::S;
		S.DisplayName = FText::FromString(TEXT("S Class"));
		S.MinPI = 650;
		S.MaxPI = 799;
		S.ClassColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
		S.BaseRewardMultiplier = 1.5f;
		S.DifficultyMultiplier = 1.5f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::S, S);
	}
	{
		FMGClassTierDefinition SPlus;
		SPlus.Tier = EMGVehicleClassTier::SPlus;
		SPlus.DisplayName = FText::FromString(TEXT("S+ Class"));
		SPlus.MinPI = 800;
		SPlus.MaxPI = 899;
		SPlus.ClassColor = FLinearColor(0.6f, 0.1f, 0.6f, 1.0f);
		SPlus.BaseRewardMultiplier = 1.75f;
		SPlus.DifficultyMultiplier = 1.75f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::SPlus, SPlus);
	}
	{
		FMGClassTierDefinition Hyper;
		Hyper.Tier = EMGVehicleClassTier::Hyper;
		Hyper.DisplayName = FText::FromString(TEXT("Hyper Class"));
		Hyper.MinPI = 900;
		Hyper.MaxPI = 949;
		Hyper.ClassColor = FLinearColor(0.9f, 0.7f, 0.1f, 1.0f);
		Hyper.BaseRewardMultiplier = 2.0f;
		Hyper.DifficultyMultiplier = 2.0f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::Hyper, Hyper);
	}
	{
		FMGClassTierDefinition Legend;
		Legend.Tier = EMGVehicleClassTier::Legend;
		Legend.DisplayName = FText::FromString(TEXT("Legend Class"));
		Legend.MinPI = 950;
		Legend.MaxPI = 999;
		Legend.ClassColor = FLinearColor(1.0f, 0.9f, 0.5f, 1.0f);
		Legend.BaseRewardMultiplier = 3.0f;
		Legend.DifficultyMultiplier = 3.0f;
		ClassTierDefinitions.Add(EMGVehicleClassTier::Legend, Legend);
	}
}

bool UMGVehicleClassSubsystem::RegisterVehicle(const FMGVehicleClassification& Classification)
{
	if (Classification.VehicleId.IsEmpty())
	{
		return false;
	}

	FMGVehicleClassification NewClassification = Classification;

	// Calculate class tier from PI if not set
	if (NewClassification.ClassTier == EMGVehicleClassTier::Custom || NewClassification.CurrentPI > 0)
	{
		NewClassification.ClassTier = CalculateClassTierFromPI(NewClassification.CurrentPI);
	}

	// Calculate power to weight ratio
	if (NewClassification.PerformanceProfile.WeightKG > 0)
	{
		NewClassification.PerformanceProfile.PowerToWeightRatio =
			CalculatePowerToWeightRatio(NewClassification.PerformanceProfile.PowerHP, NewClassification.PerformanceProfile.WeightKG);
	}

	RegisteredVehicles.Add(NewClassification.VehicleId, NewClassification);
	OnVehicleRegistered.Broadcast(NewClassification.VehicleId, NewClassification.ClassTier);

	return true;
}

bool UMGVehicleClassSubsystem::UnregisterVehicle(const FString& VehicleId)
{
	return RegisteredVehicles.Remove(VehicleId) > 0;
}

FMGVehicleClassification UMGVehicleClassSubsystem::GetVehicleClassification(const FString& VehicleId) const
{
	if (const FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId))
	{
		return *Classification;
	}
	return FMGVehicleClassification();
}

bool UMGVehicleClassSubsystem::IsVehicleRegistered(const FString& VehicleId) const
{
	return RegisteredVehicles.Contains(VehicleId);
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetAllVehicles() const
{
	TArray<FMGVehicleClassification> Result;
	RegisteredVehicles.GenerateValueArray(Result);
	return Result;
}

bool UMGVehicleClassSubsystem::RegisterClassTier(const FMGClassTierDefinition& TierDef)
{
	ClassTierDefinitions.Add(TierDef.Tier, TierDef);
	return true;
}

FMGClassTierDefinition UMGVehicleClassSubsystem::GetClassTierDefinition(EMGVehicleClassTier Tier) const
{
	if (const FMGClassTierDefinition* Def = ClassTierDefinitions.Find(Tier))
	{
		return *Def;
	}
	return FMGClassTierDefinition();
}

EMGVehicleClassTier UMGVehicleClassSubsystem::GetVehicleClassTier(const FString& VehicleId) const
{
	if (const FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId))
	{
		return Classification->ClassTier;
	}
	return EMGVehicleClassTier::D;
}

EMGVehicleClassTier UMGVehicleClassSubsystem::CalculateClassTierFromPI(int32 PI) const
{
	for (const auto& TierPair : ClassTierDefinitions)
	{
		if (PI >= TierPair.Value.MinPI && PI <= TierPair.Value.MaxPI)
		{
			return TierPair.Key;
		}
	}

	// Default based on PI ranges
	if (PI >= 950) return EMGVehicleClassTier::Legend;
	if (PI >= 900) return EMGVehicleClassTier::Hyper;
	if (PI >= 800) return EMGVehicleClassTier::SPlus;
	if (PI >= 650) return EMGVehicleClassTier::S;
	if (PI >= 500) return EMGVehicleClassTier::A;
	if (PI >= 350) return EMGVehicleClassTier::B;
	if (PI >= 200) return EMGVehicleClassTier::C;
	return EMGVehicleClassTier::D;
}

FLinearColor UMGVehicleClassSubsystem::GetClassColor(EMGVehicleClassTier Tier) const
{
	if (const FMGClassTierDefinition* Def = ClassTierDefinitions.Find(Tier))
	{
		return Def->ClassColor;
	}
	return FLinearColor::White;
}

FText UMGVehicleClassSubsystem::GetClassDisplayName(EMGVehicleClassTier Tier) const
{
	if (const FMGClassTierDefinition* Def = ClassTierDefinitions.Find(Tier))
	{
		return Def->DisplayName;
	}

	switch (Tier)
	{
		case EMGVehicleClassTier::D: return FText::FromString(TEXT("D"));
		case EMGVehicleClassTier::C: return FText::FromString(TEXT("C"));
		case EMGVehicleClassTier::B: return FText::FromString(TEXT("B"));
		case EMGVehicleClassTier::A: return FText::FromString(TEXT("A"));
		case EMGVehicleClassTier::S: return FText::FromString(TEXT("S"));
		case EMGVehicleClassTier::SPlus: return FText::FromString(TEXT("S+"));
		case EMGVehicleClassTier::Hyper: return FText::FromString(TEXT("Hyper"));
		case EMGVehicleClassTier::Legend: return FText::FromString(TEXT("Legend"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

int32 UMGVehicleClassSubsystem::GetVehiclePI(const FString& VehicleId) const
{
	if (const FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId))
	{
		return Classification->CurrentPI;
	}
	return 0;
}

int32 UMGVehicleClassSubsystem::CalculatePI(const FMGVehiclePerformanceProfile& Profile, const FMGPIWeights& Weights) const
{
	float PI = 0.0f;

	// Calculate weighted stat contributions (each stat out of 10)
	PI += Profile.Speed.ModifiedValue * Weights.SpeedWeight * 20.0f;
	PI += Profile.Acceleration.ModifiedValue * Weights.AccelerationWeight * 20.0f;
	PI += Profile.Handling.ModifiedValue * Weights.HandlingWeight * 15.0f;
	PI += Profile.Braking.ModifiedValue * Weights.BrakingWeight * 10.0f;
	PI += Profile.Nitro.ModifiedValue * Weights.NitroWeight * 10.0f;
	PI += Profile.Durability.ModifiedValue * Weights.DurabilityWeight * 5.0f;

	// Power to weight bonus
	PI += Profile.PowerToWeightRatio * Weights.PowerToWeightFactor * 50.0f;

	// Real performance bonuses
	float SpeedBonus = FMath::Clamp((Profile.TopSpeedKMH - 150.0f) / 200.0f, 0.0f, 1.0f) * 100.0f;
	float AccelBonus = FMath::Clamp((8.0f - Profile.ZeroToSixtyTime) / 6.0f, 0.0f, 1.0f) * 100.0f;
	float PowerBonus = FMath::Clamp((Profile.PowerHP - 100.0f) / 1000.0f, 0.0f, 1.0f) * 50.0f;

	PI += SpeedBonus + AccelBonus + PowerBonus;

	return FMath::Clamp(FMath::RoundToInt(PI), 0, 999);
}

int32 UMGVehicleClassSubsystem::CalculatePIWithPreset(const FMGVehiclePerformanceProfile& Profile, EMGPIWeightPreset Preset) const
{
	FMGPIWeights Weights = GetPIWeightsForPreset(Preset);
	return CalculatePI(Profile, Weights);
}

bool UMGVehicleClassSubsystem::UpdateVehiclePI(const FString& VehicleId, int32 NewPI)
{
	FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId);
	if (!Classification)
	{
		return false;
	}

	int32 OldPI = Classification->CurrentPI;
	EMGVehicleClassTier OldClass = Classification->ClassTier;

	Classification->CurrentPI = FMath::Clamp(NewPI, 0, 999);
	Classification->ClassTier = CalculateClassTierFromPI(Classification->CurrentPI);

	if (OldPI != Classification->CurrentPI)
	{
		OnVehiclePIChanged.Broadcast(VehicleId, OldPI, Classification->CurrentPI);
	}

	if (OldClass != Classification->ClassTier)
	{
		OnVehicleClassChanged.Broadcast(VehicleId, OldClass, Classification->ClassTier);
	}

	return true;
}

FMGPIWeights UMGVehicleClassSubsystem::GetPIWeightsForPreset(EMGPIWeightPreset Preset) const
{
	if (const FMGPIWeights* Weights = PIWeightPresets.Find(Preset))
	{
		return *Weights;
	}

	// Return balanced as default
	if (const FMGPIWeights* Balanced = PIWeightPresets.Find(EMGPIWeightPreset::Balanced))
	{
		return *Balanced;
	}

	return FMGPIWeights();
}

FMGUpgradePIImpact UMGVehicleClassSubsystem::CalculateUpgradeImpact(const FString& VehicleId, const FString& UpgradeId) const
{
	FMGUpgradePIImpact Impact;
	Impact.UpgradeId = UpgradeId;

	// This would typically query upgrade system for impact values
	// Placeholder implementation
	Impact.PIChange = 10;
	Impact.bMayChangeClass = false;

	if (const FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId))
	{
		int32 NewPI = Classification->CurrentPI + Impact.PIChange;
		EMGVehicleClassTier NewClass = CalculateClassTierFromPI(NewPI);
		Impact.bMayChangeClass = (NewClass != Classification->ClassTier);
	}

	return Impact;
}

int32 UMGVehicleClassSubsystem::GetPIHeadroomForClass(const FString& VehicleId) const
{
	if (const FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId))
	{
		FMGClassTierDefinition TierDef = GetClassTierDefinition(Classification->ClassTier);
		return TierDef.MaxPI - Classification->CurrentPI;
	}
	return 0;
}

FMGVehiclePerformanceProfile UMGVehicleClassSubsystem::GetPerformanceProfile(const FString& VehicleId) const
{
	if (const FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId))
	{
		return Classification->PerformanceProfile;
	}
	return FMGVehiclePerformanceProfile();
}

bool UMGVehicleClassSubsystem::UpdatePerformanceProfile(const FString& VehicleId, const FMGVehiclePerformanceProfile& Profile)
{
	FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId);
	if (!Classification)
	{
		return false;
	}

	Classification->PerformanceProfile = Profile;

	// Recalculate PI
	int32 NewPI = CalculatePIWithPreset(Profile, EMGPIWeightPreset::Balanced);
	UpdateVehiclePI(VehicleId, NewPI);

	return true;
}

float UMGVehicleClassSubsystem::GetPerformanceStat(const FString& VehicleId, EMGPerformanceStat Stat) const
{
	FMGVehiclePerformanceProfile Profile = GetPerformanceProfile(VehicleId);

	switch (Stat)
	{
		case EMGPerformanceStat::Speed: return Profile.Speed.ModifiedValue;
		case EMGPerformanceStat::Acceleration: return Profile.Acceleration.ModifiedValue;
		case EMGPerformanceStat::Handling: return Profile.Handling.ModifiedValue;
		case EMGPerformanceStat::Braking: return Profile.Braking.ModifiedValue;
		case EMGPerformanceStat::Nitro: return Profile.Nitro.ModifiedValue;
		case EMGPerformanceStat::Durability: return Profile.Durability.ModifiedValue;
		default: return 0.0f;
	}
}

FMGVehicleComparison UMGVehicleClassSubsystem::CompareVehicles(const FString& VehicleIdA, const FString& VehicleIdB) const
{
	FMGVehicleComparison Result;
	Result.VehicleA = VehicleIdA;
	Result.VehicleB = VehicleIdB;

	FMGVehicleClassification ClassA = GetVehicleClassification(VehicleIdA);
	FMGVehicleClassification ClassB = GetVehicleClassification(VehicleIdB);

	Result.PIDifference = ClassA.CurrentPI - ClassB.CurrentPI;

	// Calculate stat differences
	Result.StatDifferences.Add(EMGPerformanceStat::Speed,
		ClassA.PerformanceProfile.Speed.ModifiedValue - ClassB.PerformanceProfile.Speed.ModifiedValue);
	Result.StatDifferences.Add(EMGPerformanceStat::Acceleration,
		ClassA.PerformanceProfile.Acceleration.ModifiedValue - ClassB.PerformanceProfile.Acceleration.ModifiedValue);
	Result.StatDifferences.Add(EMGPerformanceStat::Handling,
		ClassA.PerformanceProfile.Handling.ModifiedValue - ClassB.PerformanceProfile.Handling.ModifiedValue);
	Result.StatDifferences.Add(EMGPerformanceStat::Braking,
		ClassA.PerformanceProfile.Braking.ModifiedValue - ClassB.PerformanceProfile.Braking.ModifiedValue);
	Result.StatDifferences.Add(EMGPerformanceStat::Nitro,
		ClassA.PerformanceProfile.Nitro.ModifiedValue - ClassB.PerformanceProfile.Nitro.ModifiedValue);
	Result.StatDifferences.Add(EMGPerformanceStat::Durability,
		ClassA.PerformanceProfile.Durability.ModifiedValue - ClassB.PerformanceProfile.Durability.ModifiedValue);

	// Determine recommendation
	if (Result.PIDifference > 20)
	{
		Result.RecommendedChoice = VehicleIdA;
		Result.ComparisonSummary = FString::Printf(TEXT("%s has significantly higher PI (+%d)"), *VehicleIdA, Result.PIDifference);
	}
	else if (Result.PIDifference < -20)
	{
		Result.RecommendedChoice = VehicleIdB;
		Result.ComparisonSummary = FString::Printf(TEXT("%s has significantly higher PI (+%d)"), *VehicleIdB, -Result.PIDifference);
	}
	else
	{
		Result.ComparisonSummary = TEXT("Vehicles are closely matched - choose based on preferred driving style");
	}

	return Result;
}

float UMGVehicleClassSubsystem::CalculatePowerToWeightRatio(float PowerHP, float WeightKG) const
{
	if (WeightKG <= 0.0f)
	{
		return 0.0f;
	}
	return PowerHP / WeightKG;
}

bool UMGVehicleClassSubsystem::RegisterRestriction(const FMGClassRestriction& Restriction)
{
	if (Restriction.RestrictionId.IsEmpty())
	{
		return false;
	}

	RegisteredRestrictions.Add(Restriction.RestrictionId, Restriction);
	return true;
}

FMGClassRestriction UMGVehicleClassSubsystem::GetRestriction(const FString& RestrictionId) const
{
	if (const FMGClassRestriction* Restriction = RegisteredRestrictions.Find(RestrictionId))
	{
		return *Restriction;
	}
	return FMGClassRestriction();
}

bool UMGVehicleClassSubsystem::DoesVehicleMeetRestriction(const FString& VehicleId, const FString& RestrictionId) const
{
	return GetViolatedRestrictions(VehicleId, RestrictionId).Num() == 0;
}

TArray<FString> UMGVehicleClassSubsystem::GetViolatedRestrictions(const FString& VehicleId, const FString& RestrictionId) const
{
	TArray<FString> Violations;

	const FMGClassRestriction* Restriction = RegisteredRestrictions.Find(RestrictionId);
	const FMGVehicleClassification* Vehicle = RegisteredVehicles.Find(VehicleId);

	if (!Restriction || !Vehicle)
	{
		Violations.Add(TEXT("Invalid restriction or vehicle"));
		return Violations;
	}

	// Check class tier
	if (Restriction->AllowedTiers.Num() > 0 && !Restriction->AllowedTiers.Contains(Vehicle->ClassTier))
	{
		Violations.Add(TEXT("Vehicle class tier not allowed"));
	}

	// Check PI range
	if (Vehicle->CurrentPI < Restriction->MinPI)
	{
		Violations.Add(FString::Printf(TEXT("PI too low (min: %d)"), Restriction->MinPI));
	}
	if (Vehicle->CurrentPI > Restriction->MaxPI)
	{
		Violations.Add(FString::Printf(TEXT("PI too high (max: %d)"), Restriction->MaxPI));
	}

	// Check body type
	if (Restriction->AllowedBodyTypes.Num() > 0 && !Restriction->AllowedBodyTypes.Contains(Vehicle->BodyType))
	{
		Violations.Add(TEXT("Body type not allowed"));
	}

	// Check drivetrain
	if (Restriction->AllowedDrivetrains.Num() > 0 && !Restriction->AllowedDrivetrains.Contains(Vehicle->Drivetrain))
	{
		Violations.Add(TEXT("Drivetrain type not allowed"));
	}

	// Check era
	if (Restriction->AllowedEras.Num() > 0 && !Restriction->AllowedEras.Contains(Vehicle->Era))
	{
		Violations.Add(TEXT("Vehicle era not allowed"));
	}

	// Check country
	if (Restriction->AllowedCountries.Num() > 0 && !Restriction->AllowedCountries.Contains(Vehicle->CountryOfOrigin))
	{
		Violations.Add(TEXT("Country of origin not allowed"));
	}

	// Check manufacturer
	if (Restriction->AllowedManufacturers.Num() > 0 && !Restriction->AllowedManufacturers.Contains(Vehicle->Manufacturer.ToString()))
	{
		Violations.Add(TEXT("Manufacturer not allowed"));
	}

	// Check required tags
	for (const FString& RequiredTag : Restriction->RequiredTags)
	{
		if (!Vehicle->SpecialTags.Contains(RequiredTag))
		{
			Violations.Add(FString::Printf(TEXT("Missing required tag: %s"), *RequiredTag));
		}
	}

	// Check excluded vehicles
	if (Restriction->ExcludedVehicleIds.Contains(VehicleId))
	{
		Violations.Add(TEXT("Vehicle is specifically excluded"));
	}

	// Check stock requirement
	if (Restriction->bRequireStock && Vehicle->CurrentPI != Vehicle->BasePI)
	{
		Violations.Add(TEXT("Vehicle must be stock (no upgrades)"));
	}

	return Violations;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetEligibleVehicles(const FString& RestrictionId) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (DoesVehicleMeetRestriction(VehiclePair.Key, RestrictionId))
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByClass(EMGVehicleClassTier Tier) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.ClassTier == Tier)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByBodyType(EMGVehicleBodyType BodyType) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.BodyType == BodyType)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByDrivetrain(EMGDrivetrainType Drivetrain) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.Drivetrain == Drivetrain)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByEra(EMGVehicleEra Era) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.Era == Era)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByPIRange(int32 MinPI, int32 MaxPI) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.CurrentPI >= MinPI && VehiclePair.Value.CurrentPI <= MaxPI)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByManufacturer(const FString& Manufacturer) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.Manufacturer.ToString() == Manufacturer)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FMGVehicleClassification> UMGVehicleClassSubsystem::GetVehiclesByCountry(const FString& Country) const
{
	TArray<FMGVehicleClassification> Result;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.CountryOfOrigin == Country)
		{
			Result.Add(VehiclePair.Value);
		}
	}

	return Result;
}

TArray<FString> UMGVehicleClassSubsystem::GetAllManufacturers() const
{
	TSet<FString> ManufacturerSet;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		ManufacturerSet.Add(VehiclePair.Value.Manufacturer.ToString());
	}

	return ManufacturerSet.Array();
}

TArray<FString> UMGVehicleClassSubsystem::GetAllCountries() const
{
	TSet<FString> CountrySet;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (!VehiclePair.Value.CountryOfOrigin.IsEmpty())
		{
			CountrySet.Add(VehiclePair.Value.CountryOfOrigin);
		}
	}

	return CountrySet.Array();
}

int32 UMGVehicleClassSubsystem::GetTotalVehicleCount() const
{
	return RegisteredVehicles.Num();
}

int32 UMGVehicleClassSubsystem::GetVehicleCountInClass(EMGVehicleClassTier Tier) const
{
	int32 Count = 0;
	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.ClassTier == Tier)
		{
			Count++;
		}
	}
	return Count;
}

float UMGVehicleClassSubsystem::GetAveragePIInClass(EMGVehicleClassTier Tier) const
{
	int32 TotalPI = 0;
	int32 Count = 0;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.ClassTier == Tier)
		{
			TotalPI += VehiclePair.Value.CurrentPI;
			Count++;
		}
	}

	return Count > 0 ? static_cast<float>(TotalPI) / static_cast<float>(Count) : 0.0f;
}

FMGVehicleClassification UMGVehicleClassSubsystem::GetHighestPIVehicle() const
{
	FMGVehicleClassification Highest;
	int32 HighestPI = -1;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.CurrentPI > HighestPI)
		{
			HighestPI = VehiclePair.Value.CurrentPI;
			Highest = VehiclePair.Value;
		}
	}

	return Highest;
}

FMGVehicleClassification UMGVehicleClassSubsystem::GetLowestPIVehicle() const
{
	FMGVehicleClassification Lowest;
	int32 LowestPI = INT_MAX;

	for (const auto& VehiclePair : RegisteredVehicles)
	{
		if (VehiclePair.Value.CurrentPI < LowestPI)
		{
			LowestPI = VehiclePair.Value.CurrentPI;
			Lowest = VehiclePair.Value;
		}
	}

	return Lowest;
}

FText UMGVehicleClassSubsystem::GetBodyTypeDisplayName(EMGVehicleBodyType BodyType) const
{
	switch (BodyType)
	{
		case EMGVehicleBodyType::Compact: return FText::FromString(TEXT("Compact"));
		case EMGVehicleBodyType::Coupe: return FText::FromString(TEXT("Coupe"));
		case EMGVehicleBodyType::Sedan: return FText::FromString(TEXT("Sedan"));
		case EMGVehicleBodyType::Hatchback: return FText::FromString(TEXT("Hatchback"));
		case EMGVehicleBodyType::Sports: return FText::FromString(TEXT("Sports Car"));
		case EMGVehicleBodyType::Muscle: return FText::FromString(TEXT("Muscle Car"));
		case EMGVehicleBodyType::Supercar: return FText::FromString(TEXT("Supercar"));
		case EMGVehicleBodyType::Hypercar: return FText::FromString(TEXT("Hypercar"));
		case EMGVehicleBodyType::SUV: return FText::FromString(TEXT("SUV"));
		case EMGVehicleBodyType::Truck: return FText::FromString(TEXT("Truck"));
		case EMGVehicleBodyType::Wagon: return FText::FromString(TEXT("Wagon"));
		case EMGVehicleBodyType::Roadster: return FText::FromString(TEXT("Roadster"));
		case EMGVehicleBodyType::Kei: return FText::FromString(TEXT("Kei Car"));
		case EMGVehicleBodyType::Van: return FText::FromString(TEXT("Van"));
		case EMGVehicleBodyType::Classic: return FText::FromString(TEXT("Classic"));
		case EMGVehicleBodyType::Exotic: return FText::FromString(TEXT("Exotic"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UMGVehicleClassSubsystem::GetDrivetrainDisplayName(EMGDrivetrainType Drivetrain) const
{
	switch (Drivetrain)
	{
		case EMGDrivetrainType::FWD: return FText::FromString(TEXT("Front-Wheel Drive"));
		case EMGDrivetrainType::RWD: return FText::FromString(TEXT("Rear-Wheel Drive"));
		case EMGDrivetrainType::AWD: return FText::FromString(TEXT("All-Wheel Drive"));
		case EMGDrivetrainType::MR: return FText::FromString(TEXT("Mid-Engine RWD"));
		case EMGDrivetrainType::RR: return FText::FromString(TEXT("Rear-Engine RWD"));
		case EMGDrivetrainType::F4WD: return FText::FromString(TEXT("Full-Time 4WD"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

FText UMGVehicleClassSubsystem::GetEraDisplayName(EMGVehicleEra Era) const
{
	switch (Era)
	{
		case EMGVehicleEra::Classic: return FText::FromString(TEXT("Classic (Pre-1980)"));
		case EMGVehicleEra::Retro: return FText::FromString(TEXT("Retro (1980-1999)"));
		case EMGVehicleEra::Modern: return FText::FromString(TEXT("Modern (2000-2015)"));
		case EMGVehicleEra::Current: return FText::FromString(TEXT("Current (2015+)"));
		case EMGVehicleEra::Future: return FText::FromString(TEXT("Future Concept"));
		default: return FText::FromString(TEXT("Unknown"));
	}
}

void UMGVehicleClassSubsystem::SaveVehicleClassData()
{
	// Save is handled centrally by MG_SAVE_ManagerSubsystem
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			SaveManager->QuickSave();
		}
	}
}

void UMGVehicleClassSubsystem::LoadVehicleClassData()
{
	// Load vehicle class data from central save manager
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UMGSaveManagerSubsystem* SaveManager = GI->GetSubsystem<UMGSaveManagerSubsystem>())
		{
			if (const UMGSaveGame* SaveData = SaveManager->GetCurrentSaveData())
			{
				// Restore unlocked classes
				for (const FName& ClassName : SaveData->VehicleClassData.UnlockedClasses)
				{
					UnlockedClasses.AddUnique(ClassName);
				}
				UE_LOG(LogTemp, Log, TEXT("VehicleClassSubsystem: Loaded %d unlocked classes"), SaveData->VehicleClassData.UnlockedClasses.Num());
			}
		}
	}
}

void UMGVehicleClassSubsystem::RecalculateVehicleClass(const FString& VehicleId)
{
	FMGVehicleClassification* Classification = RegisteredVehicles.Find(VehicleId);
	if (Classification)
	{
		int32 NewPI = CalculatePIWithPreset(Classification->PerformanceProfile, EMGPIWeightPreset::Balanced);
		UpdateVehiclePI(VehicleId, NewPI);
	}
}

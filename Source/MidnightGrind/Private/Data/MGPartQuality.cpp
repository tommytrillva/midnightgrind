// Copyright Midnight Grind. All Rights Reserved.

#include "Data/MGPartQuality.h"

// ==========================================
// UMGPartQualityStatics Implementation
// ==========================================

FMGQualityEffects UMGPartQualityStatics::GetQualityEffects(EMGPartQuality Quality)
{
	FMGQualityEffects Effects;

	switch (Quality)
	{
	case EMGPartQuality::OEM:
		// Baseline - stock replacement quality
		Effects.PerformanceMultiplier = 1.0f;
		Effects.TopSpeedBonus = 0.0f;
		Effects.AccelerationEfficiency = 1.0f;
		Effects.WearRateMultiplier = 1.0f;
		Effects.BaseDurability = 100.0f;
		Effects.HeatResistance = 1.0f;
		Effects.StressTolerance = 0.75f;
		Effects.WeightDifferenceKG = 0.0f;
		Effects.WeightMultiplier = 1.0f;
		Effects.CostMultiplier = 1.0f;
		Effects.InstallationCostMultiplier = 1.0f;
		Effects.ResaleValueRetention = 0.5f;
		Effects.BaseFailureChance = 0.001f;
		Effects.StressFailureMultiplier = 2.0f;
		Effects.WearFailureIncrease = 0.008f;
		Effects.NVHFactor = 1.0f;
		Effects.RefinementFactor = 1.0f;
		break;

	case EMGPartQuality::Aftermarket:
		// Better than stock, good value proposition
		Effects.PerformanceMultiplier = 1.03f;
		Effects.TopSpeedBonus = 1.0f;
		Effects.AccelerationEfficiency = 1.02f;
		Effects.WearRateMultiplier = 0.95f;
		Effects.BaseDurability = 110.0f;
		Effects.HeatResistance = 1.05f;
		Effects.StressTolerance = 0.78f;
		Effects.WeightDifferenceKG = -0.5f;
		Effects.WeightMultiplier = 0.98f;
		Effects.CostMultiplier = 1.3f;
		Effects.InstallationCostMultiplier = 1.1f;
		Effects.ResaleValueRetention = 0.55f;
		Effects.BaseFailureChance = 0.0008f;
		Effects.StressFailureMultiplier = 1.8f;
		Effects.WearFailureIncrease = 0.007f;
		Effects.NVHFactor = 1.05f;
		Effects.RefinementFactor = 0.98f;
		break;

	case EMGPartQuality::RaceSpec:
		// Track-focused, significant gains but trade-offs
		Effects.PerformanceMultiplier = 1.08f;
		Effects.TopSpeedBonus = 3.0f;
		Effects.AccelerationEfficiency = 1.05f;
		Effects.WearRateMultiplier = 1.2f; // Wears faster due to aggressive design
		Effects.BaseDurability = 90.0f; // Lower street durability
		Effects.HeatResistance = 1.3f; // Better heat management
		Effects.StressTolerance = 0.85f;
		Effects.WeightDifferenceKG = -1.0f;
		Effects.WeightMultiplier = 0.95f;
		Effects.CostMultiplier = 1.8f;
		Effects.InstallationCostMultiplier = 1.3f;
		Effects.ResaleValueRetention = 0.45f; // Track parts don't resell well
		Effects.BaseFailureChance = 0.002f; // Higher base failure
		Effects.StressFailureMultiplier = 1.2f; // But handles stress better
		Effects.WearFailureIncrease = 0.01f;
		Effects.NVHFactor = 1.5f; // Much harsher
		Effects.RefinementFactor = 0.8f; // Less refined feel
		break;

	case EMGPartQuality::Billet:
		// Premium machined parts, excellent durability
		Effects.PerformanceMultiplier = 1.10f;
		Effects.TopSpeedBonus = 2.5f;
		Effects.AccelerationEfficiency = 1.06f;
		Effects.WearRateMultiplier = 0.7f;
		Effects.BaseDurability = 150.0f;
		Effects.HeatResistance = 1.4f;
		Effects.StressTolerance = 0.9f;
		Effects.WeightDifferenceKG = -1.5f;
		Effects.WeightMultiplier = 0.93f;
		Effects.CostMultiplier = 2.5f;
		Effects.InstallationCostMultiplier = 1.5f;
		Effects.ResaleValueRetention = 0.65f;
		Effects.BaseFailureChance = 0.0004f;
		Effects.StressFailureMultiplier = 1.3f;
		Effects.WearFailureIncrease = 0.004f;
		Effects.NVHFactor = 1.1f;
		Effects.RefinementFactor = 1.1f;
		break;

	case EMGPartQuality::Forged:
		// Top tier, best everything
		Effects.PerformanceMultiplier = 1.15f;
		Effects.TopSpeedBonus = 4.0f;
		Effects.AccelerationEfficiency = 1.08f;
		Effects.WearRateMultiplier = 0.6f;
		Effects.BaseDurability = 180.0f;
		Effects.HeatResistance = 1.5f;
		Effects.StressTolerance = 0.95f;
		Effects.WeightDifferenceKG = -2.5f;
		Effects.WeightMultiplier = 0.88f;
		Effects.CostMultiplier = 3.5f;
		Effects.InstallationCostMultiplier = 1.8f;
		Effects.ResaleValueRetention = 0.75f;
		Effects.BaseFailureChance = 0.0002f;
		Effects.StressFailureMultiplier = 1.1f;
		Effects.WearFailureIncrease = 0.002f;
		Effects.NVHFactor = 1.0f; // Refined even at high performance
		Effects.RefinementFactor = 1.2f;
		break;

	default:
		// Default to OEM if unknown
		break;
	}

	return Effects;
}

FMGBrandReputationData UMGPartQualityStatics::GetReputationData(EMGBrandReputation Reputation)
{
	FMGBrandReputationData Data;
	Data.Reputation = Reputation;

	switch (Reputation)
	{
	case EMGBrandReputation::Unknown:
		Data.QualityConsistency = 0.6f;
		Data.BrandPremium = 0.8f;
		Data.WarrantyCoverage = 0.0f;
		Data.ResaleValueBonus = 0.0f;
		Data.FailureChanceModifier = 1.8f;
		Data.DisplayName = NSLOCTEXT("PartQuality", "ReputationUnknown", "Unknown Brand");
		Data.Description = NSLOCTEXT("PartQuality", "ReputationUnknownDesc",
			"No-name or untested brand. Quality is unpredictable.");
		break;

	case EMGBrandReputation::Budget:
		Data.QualityConsistency = 0.75f;
		Data.BrandPremium = 0.9f;
		Data.WarrantyCoverage = 0.2f;
		Data.ResaleValueBonus = 0.05f;
		Data.FailureChanceModifier = 1.4f;
		Data.DisplayName = NSLOCTEXT("PartQuality", "ReputationBudget", "Budget Brand");
		Data.Description = NSLOCTEXT("PartQuality", "ReputationBudgetDesc",
			"Economy brand. Functional but basic quality control.");
		break;

	case EMGBrandReputation::Standard:
		Data.QualityConsistency = 0.85f;
		Data.BrandPremium = 1.0f;
		Data.WarrantyCoverage = 0.4f;
		Data.ResaleValueBonus = 0.1f;
		Data.FailureChanceModifier = 1.0f;
		Data.DisplayName = NSLOCTEXT("PartQuality", "ReputationStandard", "Standard Brand");
		Data.Description = NSLOCTEXT("PartQuality", "ReputationStandardDesc",
			"Established brand with reliable quality. Good value.");
		break;

	case EMGBrandReputation::Premium:
		Data.QualityConsistency = 0.92f;
		Data.BrandPremium = 1.25f;
		Data.WarrantyCoverage = 0.6f;
		Data.ResaleValueBonus = 0.15f;
		Data.FailureChanceModifier = 0.75f;
		Data.DisplayName = NSLOCTEXT("PartQuality", "ReputationPremium", "Premium Brand");
		Data.Description = NSLOCTEXT("PartQuality", "ReputationPremiumDesc",
			"Well-known performance brand. Trusted quality, premium pricing.");
		break;

	case EMGBrandReputation::Elite:
		Data.QualityConsistency = 0.97f;
		Data.BrandPremium = 1.5f;
		Data.WarrantyCoverage = 0.8f;
		Data.ResaleValueBonus = 0.2f;
		Data.FailureChanceModifier = 0.5f;
		Data.DisplayName = NSLOCTEXT("PartQuality", "ReputationElite", "Elite Brand");
		Data.Description = NSLOCTEXT("PartQuality", "ReputationEliteDesc",
			"Elite manufacturer with racing pedigree. Exceptional quality.");
		break;

	case EMGBrandReputation::FactoryWorks:
		Data.QualityConsistency = 0.99f;
		Data.BrandPremium = 1.75f;
		Data.WarrantyCoverage = 0.9f;
		Data.ResaleValueBonus = 0.25f;
		Data.FailureChanceModifier = 0.4f;
		Data.DisplayName = NSLOCTEXT("PartQuality", "ReputationFactoryWorks", "Factory Works");
		Data.Description = NSLOCTEXT("PartQuality", "ReputationFactoryWorksDesc",
			"Factory performance division. OEM backing with motorsport expertise.");
		break;

	default:
		// Default to Standard
		Data.QualityConsistency = 0.85f;
		Data.BrandPremium = 1.0f;
		Data.WarrantyCoverage = 0.4f;
		Data.ResaleValueBonus = 0.1f;
		Data.FailureChanceModifier = 1.0f;
		break;
	}

	return Data;
}

float UMGPartQualityStatics::CalculateFailureChance(
	EMGPartQuality Quality,
	EMGBrandReputation Reputation,
	float CurrentWearLevel,
	float StressLevel)
{
	const FMGQualityEffects QualityEffects = GetQualityEffects(Quality);
	const FMGBrandReputationData RepData = GetReputationData(Reputation);

	// Clamp inputs
	CurrentWearLevel = FMath::Clamp(CurrentWearLevel, 0.0f, 1.0f);
	StressLevel = FMath::Clamp(StressLevel, 0.0f, 1.0f);

	// Base failure chance from quality
	float FailureChance = QualityEffects.BaseFailureChance;

	// Add wear-based increase (per 10% wear)
	const int32 WearTenths = FMath::FloorToInt(CurrentWearLevel * 10.0f);
	FailureChance += QualityEffects.WearFailureIncrease * WearTenths;

	// Apply stress multiplier if stress exceeds tolerance
	if (StressLevel > QualityEffects.StressTolerance)
	{
		const float ExcessStress = StressLevel - QualityEffects.StressTolerance;
		const float StressMultiplier = 1.0f + (ExcessStress * (QualityEffects.StressFailureMultiplier - 1.0f) * 4.0f);
		FailureChance *= StressMultiplier;
	}

	// Apply brand reputation modifier
	FailureChance *= RepData.FailureChanceModifier;

	// Apply quality consistency variance (random factor based on brand QC)
	// Lower consistency = more variance = potentially higher failure
	const float ConsistencyVariance = 1.0f + ((1.0f - RepData.QualityConsistency) * FMath::FRand());
	FailureChance *= ConsistencyVariance;

	// Clamp to reasonable range
	return FMath::Clamp(FailureChance, 0.0f, 0.5f);
}

FMGPartFailureResult UMGPartQualityStatics::CheckPartFailure(
	EMGPartQuality Quality,
	EMGBrandReputation Reputation,
	float CurrentWearLevel,
	float StressLevel,
	FName PartID)
{
	FMGPartFailureResult Result;
	Result.FailedPartID = PartID;

	const float FailureChance = CalculateFailureChance(Quality, Reputation, CurrentWearLevel, StressLevel);
	const float Roll = FMath::FRand();

	if (Roll < FailureChance)
	{
		Result.bDidFail = true;

		// Determine severity based on how badly we failed the roll
		// and the stress level at time of failure
		const float FailureMargin = FailureChance - Roll;
		Result.FailureSeverity = FMath::Clamp(FailureMargin * 10.0f + (StressLevel * 0.5f), 0.0f, 1.0f);

		// Performance penalty scales with severity
		Result.PerformancePenalty = 0.1f + (Result.FailureSeverity * 0.6f);

		// Can continue if severity is below catastrophic threshold
		Result.bCanContinue = Result.FailureSeverity < 0.8f;

		// Calculate repair cost based on quality and severity
		const FMGQualityEffects QualityEffects = GetQualityEffects(Quality);
		const int64 BaseCost = static_cast<int64>(500 * QualityEffects.CostMultiplier);
		Result.RepairCost = static_cast<int64>(BaseCost * (1.0f + Result.FailureSeverity * 2.0f));

		// Apply warranty reduction
		const FMGBrandReputationData RepData = GetReputationData(Reputation);
		Result.RepairCost = static_cast<int64>(Result.RepairCost * (1.0f - RepData.WarrantyCoverage));

		// Generate failure message
		if (Result.FailureSeverity < 0.3f)
		{
			Result.FailureMessage = NSLOCTEXT("PartQuality", "FailureMinor",
				"Part malfunction detected. Minor performance impact.");
		}
		else if (Result.FailureSeverity < 0.6f)
		{
			Result.FailureMessage = NSLOCTEXT("PartQuality", "FailureModerate",
				"Part failure! Significant performance loss.");
		}
		else if (Result.FailureSeverity < 0.8f)
		{
			Result.FailureMessage = NSLOCTEXT("PartQuality", "FailureSevere",
				"Critical part failure! Severe performance degradation.");
		}
		else
		{
			Result.FailureMessage = NSLOCTEXT("PartQuality", "FailureCatastrophic",
				"Catastrophic part failure! Vehicle disabled.");
		}
	}

	return Result;
}

int64 UMGPartQualityStatics::CalculatePartPrice(
	int64 BasePrice,
	EMGPartQuality Quality,
	EMGBrandReputation Reputation)
{
	const FMGQualityEffects QualityEffects = GetQualityEffects(Quality);
	const FMGBrandReputationData RepData = GetReputationData(Reputation);

	// Apply quality cost multiplier
	float FinalPrice = static_cast<float>(BasePrice) * QualityEffects.CostMultiplier;

	// Apply brand premium
	FinalPrice *= RepData.BrandPremium;

	return static_cast<int64>(FMath::RoundToInt(FinalPrice));
}

int64 UMGPartQualityStatics::CalculateResaleValue(
	int64 PurchasePrice,
	EMGPartQuality Quality,
	EMGBrandReputation Reputation,
	float CurrentWearLevel)
{
	const FMGQualityEffects QualityEffects = GetQualityEffects(Quality);
	const FMGBrandReputationData RepData = GetReputationData(Reputation);

	// Base retention from quality
	float Retention = QualityEffects.ResaleValueRetention;

	// Add brand bonus
	Retention += RepData.ResaleValueBonus;

	// Apply wear penalty (linear reduction)
	CurrentWearLevel = FMath::Clamp(CurrentWearLevel, 0.0f, 1.0f);
	Retention *= (1.0f - (CurrentWearLevel * 0.5f)); // 50% max reduction from wear

	// Calculate final value
	const float ResaleValue = static_cast<float>(PurchasePrice) * Retention;

	return FMath::Max(static_cast<int64>(FMath::RoundToInt(ResaleValue)), 1LL);
}

float UMGPartQualityStatics::ApplyQualityModifier(float BaseValue, EMGPartQuality Quality)
{
	const FMGQualityEffects Effects = GetQualityEffects(Quality);
	return BaseValue * Effects.PerformanceMultiplier;
}

float UMGPartQualityStatics::CalculateQualityWeight(float BaseWeight, EMGPartQuality Quality)
{
	const FMGQualityEffects Effects = GetQualityEffects(Quality);

	// Apply weight multiplier and add/subtract weight difference
	float FinalWeight = (BaseWeight * Effects.WeightMultiplier) + Effects.WeightDifferenceKG;

	// Ensure weight is positive
	return FMath::Max(FinalWeight, 0.1f);
}

FText UMGPartQualityStatics::GetQualityDisplayName(EMGPartQuality Quality)
{
	switch (Quality)
	{
	case EMGPartQuality::OEM:
		return NSLOCTEXT("PartQuality", "QualityOEM", "OEM");
	case EMGPartQuality::Aftermarket:
		return NSLOCTEXT("PartQuality", "QualityAftermarket", "Aftermarket");
	case EMGPartQuality::RaceSpec:
		return NSLOCTEXT("PartQuality", "QualityRaceSpec", "Race-Spec");
	case EMGPartQuality::Billet:
		return NSLOCTEXT("PartQuality", "QualityBillet", "Billet");
	case EMGPartQuality::Forged:
		return NSLOCTEXT("PartQuality", "QualityForged", "Forged");
	default:
		return NSLOCTEXT("PartQuality", "QualityUnknown", "Unknown");
	}
}

FLinearColor UMGPartQualityStatics::GetQualityColor(EMGPartQuality Quality)
{
	switch (Quality)
	{
	case EMGPartQuality::OEM:
		// Gray - stock/baseline
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	case EMGPartQuality::Aftermarket:
		// Green - good value
		return FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
	case EMGPartQuality::RaceSpec:
		// Blue - performance focused
		return FLinearColor(0.2f, 0.4f, 1.0f, 1.0f);
	case EMGPartQuality::Billet:
		// Purple - premium
		return FLinearColor(0.6f, 0.2f, 0.8f, 1.0f);
	case EMGPartQuality::Forged:
		// Gold - top tier
		return FLinearColor(1.0f, 0.85f, 0.0f, 1.0f);
	default:
		return FLinearColor::White;
	}
}

FText UMGPartQualityStatics::GetReputationDisplayName(EMGBrandReputation Reputation)
{
	const FMGBrandReputationData Data = GetReputationData(Reputation);
	return Data.DisplayName;
}

FText UMGPartQualityStatics::GetQualityDescription(EMGPartQuality Quality)
{
	switch (Quality)
	{
	case EMGPartQuality::OEM:
		return NSLOCTEXT("PartQuality", "DescOEM",
			"Original Equipment Manufacturer quality. Stock replacement parts "
			"matching factory specifications. Baseline performance and reliability.");

	case EMGPartQuality::Aftermarket:
		return NSLOCTEXT("PartQuality", "DescAftermarket",
			"Quality aftermarket parts from reputable manufacturers. Modest "
			"performance gains over stock with good value proposition.");

	case EMGPartQuality::RaceSpec:
		return NSLOCTEXT("PartQuality", "DescRaceSpec",
			"Competition-grade parts designed for track use. Significant "
			"performance improvements but with increased NVH and potentially "
			"reduced street durability.");

	case EMGPartQuality::Billet:
		return NSLOCTEXT("PartQuality", "DescBillet",
			"Precision-machined from solid blocks of premium materials. "
			"Excellent strength-to-weight ratio and superior durability "
			"under extreme conditions.");

	case EMGPartQuality::Forged:
		return NSLOCTEXT("PartQuality", "DescForged",
			"Top-tier parts created through advanced forging processes. "
			"Highest strength, lightest weight, and best performance. "
			"Ultimate quality for maximum results.");

	default:
		return NSLOCTEXT("PartQuality", "DescUnknown", "Unknown quality tier.");
	}
}

int32 UMGPartQualityStatics::CompareQualityTiers(EMGPartQuality A, EMGPartQuality B)
{
	const int32 ValueA = GetQualityTierValue(A);
	const int32 ValueB = GetQualityTierValue(B);

	if (ValueA < ValueB)
	{
		return -1;
	}
	else if (ValueA > ValueB)
	{
		return 1;
	}
	return 0;
}

bool UMGPartQualityStatics::IsBetterQuality(EMGPartQuality A, EMGPartQuality B)
{
	return CompareQualityTiers(A, B) > 0;
}

int32 UMGPartQualityStatics::GetQualityTierValue(EMGPartQuality Quality)
{
	return static_cast<int32>(Quality);
}

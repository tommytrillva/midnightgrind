// Copyright Midnight Grind. All Rights Reserved.


#pragma once
#include "CoreMinimal.h"
#include "MGPartQuality.generated.h"

/**
 * @enum EMGPartQuality
 * @brief Defines the manufacturing quality tier of a part.
 *
 * Quality tiers represent the manufacturing process, materials used, and
 * overall build quality of a part. Higher quality parts offer better
 * performance, durability, and reliability at increased cost.
 *
 * Quality is distinct from part tier (EMGPartTier) which represents the
 * performance level (Stock to Legendary). A Race-tier part could be OEM
 * quality (factory replacement) or Forged quality (premium aftermarket).
 */
UENUM(BlueprintType)
enum class EMGPartQuality : uint8
{
	/**
	 * OEM (Original Equipment Manufacturer)
	 * Stock replacement parts matching factory specifications.
	 * Baseline performance and durability.
	 * Best value for maintaining stock reliability.
	 */
	OEM UMETA(DisplayName = "OEM"),

	/**
	 * Aftermarket
	 * Quality aftermarket parts from reputable manufacturers.
	 * Modest performance gains over stock.
	 * Good balance of price and performance.
	 */
	Aftermarket UMETA(DisplayName = "Aftermarket"),

	/**
	 * Race-Spec
	 * Competition-grade parts designed for track use.
	 * Significant performance improvements.
	 * Reduced comfort and increased NVH (noise, vibration, harshness).
	 * May have reduced street durability due to aggressive design.
	 */
	RaceSpec UMETA(DisplayName = "Race-Spec"),

	/**
	 * Billet
	 * Machined from solid blocks of premium materials.
	 * Excellent strength-to-weight ratio.
	 * Superior durability and heat resistance.
	 * Premium pricing reflects precision manufacturing.
	 */
	Billet UMETA(DisplayName = "Billet"),

	/**
	 * Forged
	 * Top-tier parts created through forging process.
	 * Highest strength, lightest weight, best performance.
	 * Exceptional durability under extreme stress.
	 * Maximum cost but ultimate quality.
	 */
	Forged UMETA(DisplayName = "Forged")
};

/**
 * @enum EMGBrandReputation
 * @brief Represents the reputation level of a parts manufacturer.
 *
 * Brand reputation affects perceived quality and resale value.
 * Higher reputation brands command premium prices but offer
 * better quality assurance and customer confidence.
 */
UENUM(BlueprintType)
enum class EMGBrandReputation : uint8
{
	/** Unknown or no-name brands. Budget option with quality uncertainty. */
	Unknown UMETA(DisplayName = "Unknown"),

	/** Budget brands. Functional but basic quality control. */
	Budget UMETA(DisplayName = "Budget"),

	/** Established brands. Reliable quality, good value. */
	Standard UMETA(DisplayName = "Standard"),

	/** Well-known performance brands. Trusted quality, premium pricing. */
	Premium UMETA(DisplayName = "Premium"),

	/** Elite manufacturers. Top-tier quality, highest prices. (e.g., HKS, Ohlins) */
	Elite UMETA(DisplayName = "Elite"),

	/** Factory performance divisions. OEM backing with racing heritage. (e.g., NISMO, TRD, STI) */
	FactoryWorks UMETA(DisplayName = "Factory Works")
};

/**
 * @struct FMGQualityEffects
 * @brief Defines the gameplay effects of a part's quality tier.
 *
 * Quality effects are multipliers and modifiers applied on top of
 * a part's base stats. These stack with the part's inherent effects
 * from FMGPartEffect to determine final performance.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGQualityEffects
{
	GENERATED_BODY()

	// ==========================================
	// PERFORMANCE MODIFIERS
	// ==========================================

	/**
	 * @brief Multiplier applied to all performance stats.
	 *
	 * Affects horsepower, torque, and other power-related bonuses.
	 * OEM = 1.0 (baseline), Forged = 1.15 (15% boost)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance",
		meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PerformanceMultiplier = 1.0f;

	/**
	 * @brief Additional top speed bonus in MPH.
	 *
	 * Higher quality parts may provide small top speed improvements
	 * due to reduced friction, better tolerances, etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float TopSpeedBonus = 0.0f;

	/**
	 * @brief Acceleration efficiency multiplier.
	 *
	 * Affects how efficiently power is transferred.
	 * Higher quality = less drivetrain loss.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance",
		meta = (ClampMin = "0.8", ClampMax = "1.2"))
	float AccelerationEfficiency = 1.0f;

	// ==========================================
	// DURABILITY MODIFIERS
	// ==========================================

	/**
	 * @brief Multiplier for part wear rate.
	 *
	 * Lower values = slower wear = longer part life.
	 * OEM = 1.0, Forged = 0.6 (40% slower wear)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
		meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float WearRateMultiplier = 1.0f;

	/**
	 * @brief Base durability points for the part.
	 *
	 * Higher values mean more total use before replacement needed.
	 * Measured in abstract "durability units".
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
		meta = (ClampMin = "50", ClampMax = "500"))
	float BaseDurability = 100.0f;

	/**
	 * @brief Heat resistance factor.
	 *
	 * Affects performance degradation under high temperature conditions.
	 * Higher = better heat management.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
		meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float HeatResistance = 1.0f;

	/**
	 * @brief Stress tolerance threshold.
	 *
	 * Maximum stress level (0-1) before risk of failure increases.
	 * Higher quality parts tolerate more stress safely.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Durability",
		meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float StressTolerance = 0.8f;

	// ==========================================
	// WEIGHT MODIFIERS
	// ==========================================

	/**
	 * @brief Weight difference in kilograms relative to OEM.
	 *
	 * Negative = lighter, Positive = heavier.
	 * Forged parts are typically lightest, Race-Spec may add weight
	 * for reinforcement in some cases.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float WeightDifferenceKG = 0.0f;

	/**
	 * @brief Percentage weight reduction from base part weight.
	 *
	 * Applied after WeightDifferenceKG for parts that scale with base weight.
	 * 1.0 = no change, 0.9 = 10% lighter
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight",
		meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float WeightMultiplier = 1.0f;

	// ==========================================
	// COST MODIFIERS
	// ==========================================

	/**
	 * @brief Multiplier applied to base part price.
	 *
	 * OEM = 1.0, Forged = 3.0+ (3x more expensive)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost",
		meta = (ClampMin = "0.5", ClampMax = "10.0"))
	float CostMultiplier = 1.0f;

	/**
	 * @brief Installation cost multiplier.
	 *
	 * Higher quality parts may require specialized installation.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost",
		meta = (ClampMin = "1.0", ClampMax = "3.0"))
	float InstallationCostMultiplier = 1.0f;

	/**
	 * @brief Resale value retention percentage.
	 *
	 * Percentage of purchase price retained when selling.
	 * Higher quality parts hold value better.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost",
		meta = (ClampMin = "0.2", ClampMax = "0.9"))
	float ResaleValueRetention = 0.5f;

	// ==========================================
	// RELIABILITY MODIFIERS
	// ==========================================

	/**
	 * @brief Base failure chance per race under normal conditions (0-1).
	 *
	 * Probability of part failure during a standard race.
	 * OEM should have very low chance, Unknown brands higher.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reliability",
		meta = (ClampMin = "0.0", ClampMax = "0.1"))
	float BaseFailureChance = 0.001f;

	/**
	 * @brief Failure chance multiplier under high stress conditions.
	 *
	 * Applied when vehicle is under extreme load (redline, nitrous, etc.)
	 * Lower quality parts fail more often under stress.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reliability",
		meta = (ClampMin = "1.0", ClampMax = "10.0"))
	float StressFailureMultiplier = 1.0f;

	/**
	 * @brief Failure chance increase per 10% wear.
	 *
	 * How much failure chance increases as part wears out.
	 * Forged parts degrade more gracefully.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reliability",
		meta = (ClampMin = "0.0", ClampMax = "0.05"))
	float WearFailureIncrease = 0.005f;

	// ==========================================
	// COMFORT/NVH MODIFIERS
	// ==========================================

	/**
	 * @brief Noise, Vibration, Harshness factor.
	 *
	 * 1.0 = stock comfort level, higher = more NVH.
	 * Race-Spec parts typically increase NVH significantly.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort",
		meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float NVHFactor = 1.0f;

	/**
	 * @brief Affects drivability and predictability.
	 *
	 * 1.0 = neutral, higher = more refined response.
	 * Lower values = more aggressive/twitchy behavior.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Comfort",
		meta = (ClampMin = "0.5", ClampMax = "1.5"))
	float RefinementFactor = 1.0f;

	// ==========================================
	// CONSTRUCTOR
	// ==========================================

	FMGQualityEffects()
		: PerformanceMultiplier(1.0f)
		, TopSpeedBonus(0.0f)
		, AccelerationEfficiency(1.0f)
		, WearRateMultiplier(1.0f)
		, BaseDurability(100.0f)
		, HeatResistance(1.0f)
		, StressTolerance(0.8f)
		, WeightDifferenceKG(0.0f)
		, WeightMultiplier(1.0f)
		, CostMultiplier(1.0f)
		, InstallationCostMultiplier(1.0f)
		, ResaleValueRetention(0.5f)
		, BaseFailureChance(0.001f)
		, StressFailureMultiplier(1.0f)
		, WearFailureIncrease(0.005f)
		, NVHFactor(1.0f)
		, RefinementFactor(1.0f)
	{
	}
};

/**
 * @struct FMGBrandReputationData
 * @brief Contains data associated with a brand's reputation level.
 *
 * Used to modify quality effects based on the manufacturer's reputation.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGBrandReputationData
{
	GENERATED_BODY()

	/** The reputation level this data applies to. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
	EMGBrandReputation Reputation = EMGBrandReputation::Standard;

	/**
	 * @brief Quality consistency factor (0-1).
	 *
	 * Affects variance in part quality within the same batch.
	 * 1.0 = perfectly consistent, lower = more variance.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation",
		meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float QualityConsistency = 0.9f;

	/**
	 * @brief Price premium multiplier.
	 *
	 * Additional cost multiplier based on brand prestige.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation",
		meta = (ClampMin = "0.8", ClampMax = "2.0"))
	float BrandPremium = 1.0f;

	/**
	 * @brief Warranty coverage level (0-1).
	 *
	 * Affects replacement/repair cost if part fails.
	 * Higher = better warranty = lower replacement cost.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WarrantyCoverage = 0.5f;

	/**
	 * @brief Resale value bonus percentage.
	 *
	 * Additional resale value retention from brand recognition.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation",
		meta = (ClampMin = "0.0", ClampMax = "0.3"))
	float ResaleValueBonus = 0.1f;

	/**
	 * @brief Failure chance modifier from brand QC.
	 *
	 * Multiplier on failure chance based on quality control.
	 * Lower = better QC = fewer failures.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation",
		meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float FailureChanceModifier = 1.0f;

	/** Display name for UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText DisplayName;

	/** Description for UI tooltips. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText Description;

	FMGBrandReputationData()
		: Reputation(EMGBrandReputation::Standard)
		, QualityConsistency(0.9f)
		, BrandPremium(1.0f)
		, WarrantyCoverage(0.5f)
		, ResaleValueBonus(0.1f)
		, FailureChanceModifier(1.0f)
	{
	}
};

/**
 * @struct FMGPartFailureResult
 * @brief Contains information about a part failure event.
 *
 * Used when a part fails during gameplay to communicate
 * the failure type and consequences.
 */
USTRUCT(BlueprintType)
struct MIDNIGHTGRIND_API FMGPartFailureResult
{
	GENERATED_BODY()

	/** Whether a failure occurred. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	bool bDidFail = false;

	/** The part ID that failed. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	FName FailedPartID;

	/** Severity of the failure (0 = minor, 1 = catastrophic). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FailureSeverity = 0.0f;

	/** Performance penalty while part is failed (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float PerformancePenalty = 0.0f;

	/** Whether the vehicle can continue operating. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	bool bCanContinue = true;

	/** Estimated repair cost. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	int64 RepairCost = 0;

	/** Localized failure message for UI. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Failure")
	FText FailureMessage;

	FMGPartFailureResult()
		: bDidFail(false)
		, FailureSeverity(0.0f)
		, PerformancePenalty(0.0f)
		, bCanContinue(true)
		, RepairCost(0)
	{
	}
};

/**
 * @class UMGPartQualityStatics
 * @brief Static utility functions for the part quality system.
 *
 * Provides blueprint-callable functions for querying quality effects,
 * calculating failure chances, and converting between quality tiers.
 */
UCLASS()
class MIDNIGHTGRIND_API UMGPartQualityStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ==========================================
	// QUALITY EFFECTS GETTERS
	// ==========================================

	/**
	 * @brief Gets the default quality effects for a given quality tier.
	 *
	 * @param Quality The quality tier to get effects for.
	 * @return FMGQualityEffects struct with default values for the tier.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Quality Effects"))
	static FMGQualityEffects GetQualityEffects(EMGPartQuality Quality);

	/**
	 * @brief Gets the default reputation data for a given reputation level.
	 *
	 * @param Reputation The reputation level to get data for.
	 * @return FMGBrandReputationData struct with default values.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Reputation Data"))
	static FMGBrandReputationData GetReputationData(EMGBrandReputation Reputation);

	// ==========================================
	// FAILURE CALCULATIONS
	// ==========================================

	/**
	 * @brief Calculates the current failure chance for a part.
	 *
	 * Takes into account quality, brand reputation, current wear level,
	 * and stress conditions to determine failure probability.
	 *
	 * @param Quality The part's quality tier.
	 * @param Reputation The brand's reputation level.
	 * @param CurrentWearLevel Current wear (0 = new, 1 = worn out).
	 * @param StressLevel Current stress level (0 = idle, 1 = maximum).
	 * @return Probability of failure (0-1).
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Calculate Failure Chance"))
	static float CalculateFailureChance(
		EMGPartQuality Quality,
		EMGBrandReputation Reputation,
		float CurrentWearLevel,
		float StressLevel);

	/**
	 * @brief Checks if a part fails under current conditions.
	 *
	 * Uses calculated failure chance and random roll to determine
	 * if a failure event occurs.
	 *
	 * @param Quality The part's quality tier.
	 * @param Reputation The brand's reputation level.
	 * @param CurrentWearLevel Current wear (0 = new, 1 = worn out).
	 * @param StressLevel Current stress level (0 = idle, 1 = maximum).
	 * @param PartID The ID of the part being checked.
	 * @return FMGPartFailureResult containing failure details.
	 */
	UFUNCTION(BlueprintCallable, Category = "Part Quality",
		meta = (DisplayName = "Check Part Failure"))
	static FMGPartFailureResult CheckPartFailure(
		EMGPartQuality Quality,
		EMGBrandReputation Reputation,
		float CurrentWearLevel,
		float StressLevel,
		FName PartID);

	// ==========================================
	// COST CALCULATIONS
	// ==========================================

	/**
	 * @brief Calculates the final price of a part with quality and brand modifiers.
	 *
	 * @param BasePrice The base price of the part.
	 * @param Quality The quality tier.
	 * @param Reputation The brand reputation.
	 * @return Final price after all modifiers.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Calculate Part Price"))
	static int64 CalculatePartPrice(
		int64 BasePrice,
		EMGPartQuality Quality,
		EMGBrandReputation Reputation);

	/**
	 * @brief Calculates the resale value of a part.
	 *
	 * @param PurchasePrice Original purchase price.
	 * @param Quality The quality tier.
	 * @param Reputation The brand reputation.
	 * @param CurrentWearLevel Current wear level (0-1).
	 * @return Current resale value.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Calculate Resale Value"))
	static int64 CalculateResaleValue(
		int64 PurchasePrice,
		EMGPartQuality Quality,
		EMGBrandReputation Reputation,
		float CurrentWearLevel);

	// ==========================================
	// PERFORMANCE CALCULATIONS
	// ==========================================

	/**
	 * @brief Applies quality modifiers to a performance value.
	 *
	 * @param BaseValue The base performance value.
	 * @param Quality The quality tier.
	 * @return Modified performance value.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Apply Quality Modifier"))
	static float ApplyQualityModifier(float BaseValue, EMGPartQuality Quality);

	/**
	 * @brief Calculates the weight impact of a quality tier.
	 *
	 * @param BaseWeight Base part weight in kg.
	 * @param Quality The quality tier.
	 * @return Final weight after quality modifications.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Calculate Quality Weight"))
	static float CalculateQualityWeight(float BaseWeight, EMGPartQuality Quality);

	// ==========================================
	// DISPLAY HELPERS
	// ==========================================

	/**
	 * @brief Gets the display name for a quality tier.
	 *
	 * @param Quality The quality tier.
	 * @return Localized display name.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Quality Display Name"))
	static FText GetQualityDisplayName(EMGPartQuality Quality);

	/**
	 * @brief Gets the display color for a quality tier (for UI).
	 *
	 * @param Quality The quality tier.
	 * @return Color associated with the quality tier.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Quality Color"))
	static FLinearColor GetQualityColor(EMGPartQuality Quality);

	/**
	 * @brief Gets the display name for a reputation level.
	 *
	 * @param Reputation The reputation level.
	 * @return Localized display name.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Reputation Display Name"))
	static FText GetReputationDisplayName(EMGBrandReputation Reputation);

	/**
	 * @brief Gets a description of the quality tier for tooltips.
	 *
	 * @param Quality The quality tier.
	 * @return Localized description text.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Quality Description"))
	static FText GetQualityDescription(EMGPartQuality Quality);

	// ==========================================
	// COMPARISON HELPERS
	// ==========================================

	/**
	 * @brief Compares two quality tiers.
	 *
	 * @param A First quality tier.
	 * @param B Second quality tier.
	 * @return -1 if A < B, 0 if A == B, 1 if A > B.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Compare Quality Tiers"))
	static int32 CompareQualityTiers(EMGPartQuality A, EMGPartQuality B);

	/**
	 * @brief Checks if quality A is better than quality B.
	 *
	 * @param A First quality tier.
	 * @param B Second quality tier.
	 * @return True if A is a higher quality tier than B.
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Is Better Quality"))
	static bool IsBetterQuality(EMGPartQuality A, EMGPartQuality B);

	/**
	 * @brief Gets the numeric value of a quality tier (0-4).
	 *
	 * @param Quality The quality tier.
	 * @return Integer value (OEM=0, Forged=4).
	 */
	UFUNCTION(BlueprintPure, Category = "Part Quality",
		meta = (DisplayName = "Get Quality Tier Value"))
	static int32 GetQualityTierValue(EMGPartQuality Quality);
};
